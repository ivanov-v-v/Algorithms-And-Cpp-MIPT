/*
- 1. Реализовать динамические массивы (позже)
- 2. Реализовать структуру для юзеров и их сеансов. Реализовать процесс аутентификации. (готово)
- 3. Разобраться с хранением чисел как строк в формате big-endian (для начала, почему так повелось)
- 4. Реализовать структуру для сообщений от сервера к клиенту, написать соотв. функции 
-    (структура написана, бегло протестирована; осталось перенести основную логику на них)
- 5. Реализовать хранение истории сообщений
- 6. Подумать, как авторизовывать рута (готово)
- 7. Прописать в нужных местах мьютексы
- 8. Написать клиента
- 9. Добавить для инклюдов перечни функций, для которых они нужны
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#include <pthread.h>

#define MAX_CLIENTS 100
#define MAX_SESSIONS 1000
#define BUFFSIZE 2000

struct user_t {
    char* login[32];
    char* password[32];
    size_t user_id;
};

struct user_session_t {
    size_t user_id;
    int socket;
};

struct message_t {
    char type;
    int length;
    char *text;
};

void strip_spec_char(char*);
void* connection_handler(void*);
// сделать из многосточного текста строку-сообщение,
// которую можно было бы передать одним сокетом
void encode(char* buffer, size_t bsize, char type, char *text);
// собрать из сообщения структуру данных message
void decode(struct message_t* message, char* encoded_msg);

// сделать массивы динамическими
struct user_t* clients[MAX_CLIENTS];
struct user_session_t* sessions[MAX_SESSIONS];
int client_cnt = 0, session_cnt = 0;

// поддерживать очередь
char history[50][70000];

int main(int argc, char **argv) {
    // char *buffer = (char*) calloc(BUFFSIZE, 1);
    // char *text = (char*) calloc(BUFFSIZE, 1);
    // snprintf(text, BUFFSIZE, "OH\nMY\nGOD\n");
    // encode(buffer, BUFFSIZE, 'r', text);
    // printf("%s\n", buffer);
    // struct message_t* msg = (struct message_t*) malloc(sizeof(struct message_t));
    // msg->text = (char*) calloc(BUFFSIZE, 1);
    // decode(msg, buffer);
    // printf("%s\n", msg->text);
    // free(msg->text);
    // free(msg);
    // free(buffer);
    // free(text);
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        clients[i] = NULL;
    }
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        sessions[i] = NULL;
    }

    int socket_desc, new_socket, c, *new_socket_ptr;
    struct sockaddr_in server, client;
    char *message, *client_ip;
    int client_port;

    message = (char*) calloc(BUFFSIZE, 1);

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        perror("Couldn't create socket");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    int opt = 1;
    if (setsockopt (socket_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) < 0) {
       perror("setsockopt");
    }

    if (bind(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
        printf("%d: ", socket_desc);
        perror("Socket binding failed");
        return 1;
    }

    if (listen(socket_desc, 3)) {
       perror("Socket listening failed");
       return 1;
    }

    // создать рута и запросить его пароль
    printf("login: root\npassword: ");
    scanf("%s", message);
    clients[0] = (struct user_t*) malloc(sizeof(struct user_t));
    strcpy(clients[0]->login, "root");
    strcpy(clients[0]->password, message);
    clients[0]->user_id = 0;

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while (new_socket = accept(socket_desc, (struct sockaddr*)& client, (socklen_t*)& c)) {
        client_ip = inet_ntoa(client.sin_addr);
        client_port = ntohs(client.sin_port);

        printf("Connection accepted: %s %d\n", client_ip, client_port);
        message = "Welcome to our chatroom!\n";
        write(new_socket, message, strlen(message));

        pthread_t sniffer_thread;
        new_socket_ptr = (int*)(malloc(1));
        *new_socket_ptr = new_socket;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void*) new_socket_ptr) < 0) {
            perror("Could not create a new thread");
            return 1;
        }

        puts("Handler assigned");
    }

    if (new_socket < 0) {
        perror("New socket creation failed");
    }

    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        free(clients[i]);
    }
    close(socket_desc);
    return 0;
}

void strip_spec_char(char* text) {
    int i = 0;
    for (; text[i] != '\0'; ++i) {
        if (text[i] == '\r') {
            text[i] = '\0';
        }
    }
}

void encode(char* encoded_msg, size_t enc_size, char type, char *text) {
    char *buffer;
    int text_len, word_len;

    text_len = strlen(text), word_len = 1;
    strip_spec_char(text);

    snprintf(encoded_msg, enc_size, "[%c][%d]", type, text_len);

    buffer = (char*) calloc(BUFFSIZE, 1);
    for (size_t i = 0; i <= text_len; ++i, ++word_len) {
        if (text[i] == '\n' || text[i] == '\0') {
            printf("%d %d %s\n", i, word_len, text + i - word_len + 1);
            snprintf(buffer, BUFFSIZE, "[%d %s]", word_len, text + (i - word_len + 1));
            strcat(encoded_msg, buffer);
            word_len = 0;
        }
    }
    free(buffer);
}

void decode(struct message_t* message, char* encoded_msg) {
    if (!message) {
        puts("Message_t argument is not initialized");
        exit(1);
    }
    char *lastn = encoded_msg;
    message->type = *(lastn + 1);
    lastn += 4;
    while (isdigit(*lastn)) {
        message->length = message->length * 10 + (*lastn - '0');
        ++lastn;
    }
    ++lastn;
    int i = 0, curr_len = 0, tot_len = -1;
    while (1) {
        if (*lastn == '\0' && tot_len == -1) {
            // дошли до конца строки
            message->text[i] = '\0';
            break;
        }
        if (*lastn == '[' && tot_len == -1) {
            tot_len = 0;
            ++lastn;
            while (isdigit(*lastn)) {
                tot_len = tot_len * 10 + (*lastn - '0');
                ++lastn;
            }
            curr_len = 0;
            while (curr_len != tot_len - 1) {
                ++lastn;
                ++curr_len;
                message->text[i] = *lastn;
                ++i;
            }
            message->text[i] = '\n';
            ++i;
            tot_len = -1;
            lastn += 2;
        }
    }
}

void send_message(char* message, int curr_socket) {
    int len = strlen(message);
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i] && sessions[i]->socket != curr_socket) {
            write(sessions[i]->socket, message, len);
        }
    }
}

void send_message_all(char* message) {
    int len = strlen(message);
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i]) {
            write(sessions[i]->socket, message, len);
        }
    }
}

void* connection_handler(void* socket_desc) {
    // блок объявления переменных
    int curr_socket = *(int*)socket_desc;
    int read_size, curr_uid;
    char *outbuff, *client_message;
    outbuff = (char*) calloc(BUFFSIZE, 1);
    client_message = (char*) calloc(BUFFSIZE, 1);

    // авторизация:
    // считывание логина
    while (1) {
        snprintf(outbuff, BUFFSIZE, "login: ");
        write(curr_socket, outbuff, BUFFSIZE);
        bzero(outbuff, strlen(outbuff));
        read_size = recv(curr_socket, client_message, BUFFSIZE, 0);
        client_message[read_size] = '\0';
        strip_spec_char(client_message);
        read_size = strlen(client_message);

        if (read_size < 2 || read_size > 31) {
            snprintf(outbuff, BUFFSIZE, "incorrect login length: must lie between 2 and 31\n");
            write(curr_socket, outbuff, BUFFSIZE);
            bzero(outbuff, strlen(outbuff));
        } else {
            int invalid_login_flag = 0;
            for (size_t i = 0; i < read_size; ++i) {
                if (client_message[i] < ' ') {
                    snprintf(outbuff, BUFFSIZE, "login contains invalid symbols\n");
                    write(curr_socket, outbuff, BUFFSIZE);
                    bzero(outbuff, strlen(outbuff));
                    invalid_login_flag = 1;
                    break;
                }
            }
            if (!invalid_login_flag) {
                printf("%s\n", client_message);
                break;
            }
        }
    }

    // заблокировать таблицу клиентов мьютексом
    int already_registered_flag = 0;
    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != NULL && !strcmp(clients[i]->login, client_message)) {
            already_registered_flag = 1;
            curr_uid = i;
            break;
        }
    }

    if (!already_registered_flag) {
        snprintf(outbuff, BUFFSIZE, "Welcome! It's your first time here. Type in your password to finish registration.\n");
        write(curr_socket, outbuff, BUFFSIZE);
        bzero(outbuff, strlen(outbuff));
        // тут может происходить переполнение буфера
        for (size_t i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] == NULL) {
                curr_uid = i;
                clients[i] = (struct user_t*) malloc(sizeof(struct user_t));
                strcpy(clients[i]->login, client_message);
                clients[i]->user_id = curr_uid;
                break;
            }
        }
    }

    // считывание пароля
    while (1) {
        // выделить проверку на валидность строки в отдельную функцию
        snprintf(outbuff, BUFFSIZE, "password: ");
        write(curr_socket, outbuff, BUFFSIZE);
        bzero(outbuff, strlen(outbuff));

        read_size = recv(curr_socket, client_message, BUFFSIZE, 0);
        client_message[read_size] = '\0';
        strip_spec_char(client_message);
        read_size = strlen(client_message);

        if (read_size < 2 || read_size > 31) {
            snprintf(outbuff, BUFFSIZE, "incorrect password length: must lie between 2 and 31\n");
            write(curr_socket, outbuff, BUFFSIZE);
            bzero(outbuff, strlen(outbuff));
        } else {
            int invalid_password_flag = 0;
            for (size_t i = 0; i < read_size; ++i) {
                if (client_message[i] < ' ') {
                    snprintf(outbuff, BUFFSIZE, "password contains invalid symbols\n");
                    write(curr_socket, outbuff, BUFFSIZE);
                    bzero(outbuff, strlen(outbuff));
                    invalid_password_flag = 1;
                    break;
                }
            }
            if (invalid_password_flag) {
                continue;
            }
            if (already_registered_flag) {
                if (!strcmp(clients[curr_uid]->password, client_message)) {
                    // здесь может происходить переполнение буфера
                    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
                        if (sessions[i] == NULL) {
                            sessions[i] = (struct user_session_t*) malloc(sizeof(struct user_session_t));
                            sessions[i]->user_id = curr_uid;
                            sessions[i]->socket = curr_socket;
                            break;
                        }
                    }
                    break;
                } else {
                    snprintf(outbuff, BUFFSIZE, "passwords do not match\n");
                    write(curr_socket, outbuff, BUFFSIZE);
                    bzero(outbuff, strlen(outbuff));
                }
            } else {
                strcpy(clients[curr_uid]->password, client_message);
                // здесь может происходить переполнение буфера
                for (size_t i = 0; i < MAX_SESSIONS; ++i) {
                    if (sessions[i] == NULL) {
                        sessions[i] = (struct user_session_t*) malloc(sizeof(struct user_session_t));
                        sessions[i]->user_id = curr_uid;
                        sessions[i]->socket = curr_socket;
                        break;
                    }
                }
                break;
            }
        }
    }

    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] != NULL) {
            printf("%s: %s\n", clients[i]->login, clients[i]->password);
        }
    }

    // вынести в отдельную функцию авторизацию
    // добавить мьютексы (узнать, что это вообще такое),
    // чтобы список с пользователями нельзя было изменить, пока идёт запись

    snprintf(outbuff, BUFFSIZE, "%s has joined us!\n", clients[curr_uid]->login);
    send_message(outbuff, curr_socket);

    while ((read_size = recv(curr_socket, client_message, BUFFSIZE, 0)) > 0) {
        client_message[read_size] = '\0';
        strip_spec_char(client_message);

        // получить время сообщения (серверное, потому вычисляется здесь)
        time_t rawtime;
        time (&rawtime);
        struct tm *timeinfo = localtime (&rawtime);
        char *timestamp = (char*) calloc(64, 1);
        strftime(timestamp, 64, "%c", timeinfo);

        // разослать сообщение пользователя всем участникам (здесь должен быть парсер с разбором случаев)
        snprintf(outbuff, BUFFSIZE, "%s %s: %s\n", timestamp, clients[curr_uid]->login, client_message);
        send_message(outbuff, curr_socket);

        encode(outbuff, BUFFSIZE, 'r', client_message);
        printf("%s\n", outbuff);
        struct message_t* msg = (struct message_t*) malloc(sizeof(struct message_t));
        msg->text = (char*)malloc(BUFFSIZE);
        decode(msg, outbuff);
        printf("%s", msg->text);

        free(msg->text);
        free(msg);

        bzero(outbuff, strlen(outbuff));
        bzero(client_message, read_size);
    }

    snprintf(outbuff, BUFFSIZE, "%s logged out\n", clients[curr_uid]->login);
    send_message(outbuff, curr_uid);

    // блок очистки данных и закрытия потока
    free(client_message);
    free(outbuff);

    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i] != NULL && sessions[i]->socket == curr_socket) {
            free(sessions[i]);
            sessions[i] = NULL;
            break;
        }
    }

    if (!read_size) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    pthread_exit(NULL);
    free(socket_desc);
    return 0;
}