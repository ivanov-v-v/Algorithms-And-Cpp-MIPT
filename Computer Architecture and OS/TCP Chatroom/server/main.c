#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>

#include <pthread.h>

#include "lstring.h"
#include "msgformat.h"

#define MAX_CLIENTS 100
#define MAX_MESSAGE_LEN 65535
#define MAX_SESSIONS 1000
#define HISTORY_LEN 50

#ifndef BUFFSIZE
    #define BUFFSIZE 2000
#endif // BUFFSIZE
#define PORT 1337

struct user_t {
    char* login;
    char* password;
    size_t user_id;
};

struct user_session_t {
    size_t user_id;
    int socket;
};

void* connection_handler(void*);

// сделать массивы динамическими
struct user_t* clients[MAX_CLIENTS];
struct user_session_t* sessions[MAX_SESSIONS];

// история: последние 50 сообщений
char* history[HISTORY_LEN];

int main(int argc, char **argv) {
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

    message = NULL;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        fprintf(stderr, "Couldn't create socket");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt (socket_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt)) < 0) {
       fprintf(stderr, "setsockopt");
    }

    if (bind(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
        printf("%d: ", socket_desc);
        fprintf(stderr, "Socket binding failed");
        return EXIT_FAILURE;
    }

    if (listen(socket_desc, 3)) {
       fprintf(stderr, "Socket listening failed");
       return EXIT_FAILURE;
    }

    while (1) {
        // создать рута и запросить его пароль
        printf("login: root\npassword: ");
        lgetline(&message);
        lstrip(&message);
        size_t passwd_len = lstrlen(&message);
        size_t incorrect_passwd_flag = 0;
        if (!(passwd_len > 0 && passwd_len < 32)) {
            fprintf(stderr, "incorrect login length: must lie between 2 and 31\n");
            incorrect_passwd_flag = 1;
        }
        for (size_t i = 0; i < passwd_len; ++i) {
            if (message[i] < ' ') {
                fprintf(stderr, "login contains invalid symbols\n");
                incorrect_passwd_flag = 1;
                break;
            }
        }
        if (!incorrect_passwd_flag) {
            puts("root password is set");
            break;
        }
    }

    clients[0] = (struct user_t*) malloc(sizeof(struct user_t));
    clients[0]->login = clients[0]->password = NULL;

    lstrcpy(&clients[0]->login, "root");
    lstrcpy(&clients[0]->password, message);
    clients[0]->user_id = 0;

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while (new_socket = accept(socket_desc, (struct sockaddr*)& client, (socklen_t*)& c)) {
        client_ip = inet_ntoa(client.sin_addr);
        client_port = ntohs(client.sin_port);

        printf("Connection accepted: %s %d\n", client_ip, client_port);

        pthread_t sniffer_thread;
        new_socket_ptr = (int*)(malloc(sizeof(int)));
        *new_socket_ptr = new_socket;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void*) new_socket_ptr) < 0) {
            fprintf(stderr, "Could not create a new thread");
            return EXIT_FAILURE;
        }

        puts("Handler assigned");
    }

    for (size_t i = 0; i < MAX_CLIENTS; ++i) {
        free(clients[i]);
    }
    for (size_t i = 0; i < HISTORY_LEN; ++i) {
        free(history[i]);
    }
    close(socket_desc);
    if (new_socket < 0) {
        fprintf(stderr, "New socket creation failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void broadcast_message (int curr_socket, char *message) {
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i] && sessions[i]->socket != curr_socket) {
            send_message(sessions[i]->socket, message);
        }
    }
}

void broadcast_message_to_all (char *message) {
    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i]) {
            send_message(sessions[i]->socket, message);
        }
    }
}

void* connection_handler(void* socket_desc) {
    // блок объявления переменных
    int curr_socket = *(int*)socket_desc;
    int read_size, curr_uid, curr_session;
    char *outbuff = NULL,
            *client_message = NULL,
            *server_reply = NULL,
            *enc_buff = NULL;

    // авторизация:
    // считывание логина

    char *login = NULL,
            *password = NULL;

    while (1) {
        struct message_t* auth_handler = (struct message_t*) malloc(sizeof(struct message_t));
        auth_handler->text = NULL;

        read_size = 0;
        while (!read_size) {
            read_size = receive_message(curr_socket, auth_handler);
        }

        char* linebreak = strchr(auth_handler->text, '\n');

        lstrncpy(&login, auth_handler->text, linebreak - auth_handler->text);
        lstrncpy(&password, linebreak + 1, lstrlen(&auth_handler->text));

        lstrip(&login);
        lstrip(&password);

        free(auth_handler->text);
        free(auth_handler);

        size_t login_len = lstrlen(&login);
        int invalid_login_flag = 0;

        if (login_len < 2 || login_len > 31) {
            fprintf(stderr, "Incorrect login length: must lie between 2 and 31\n");

            lsnprintf(&server_reply, "[s][0x%016x][0x%016x][4]", htonl(1), htonl(1));
            send_message(curr_socket, server_reply);

            invalid_login_flag = 1;
        } else {
            for (size_t i = 0; i < login_len; ++i) {
                if (login[i] < ' ') {
                    fprintf(stderr, "Login contains invalid symbols\n");

                    lsnprintf(&server_reply, "[s][0x%016x][0x%016x][4]", htonl(1), htonl(1));
                    send_message(curr_socket, server_reply);

                    invalid_login_flag = 1;
                    break;
                }
            }
        }

        if (invalid_login_flag) {
            continue;
        }

        // заблокировать таблицу клиентов мьютексом
        int already_registered_flag = 0;
        for (size_t i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i] != NULL && !strcmp(clients[i]->login, login)) {
                already_registered_flag = 1;
                curr_uid = i;
                break;
            }
        }

        // считывание пароля
        // выделить проверку на валидность строки в отдельную функцию

        size_t passwd_len = lstrlen(&password);
        int invalid_password_flag = 0;

        if (passwd_len < 2 || passwd_len > 31) {
            fprintf(stderr, "Incorrect password length: must lie between 2 and 31\n");
            lsnprintf(&server_reply, "[s][0x%016x][0x%016x][4]", htonl(1), htonl(1));
            send_message(curr_socket, server_reply);
            invalid_password_flag = 1;
        } else {
            for (size_t i = 0; i < passwd_len; ++i) {
                if (password[i] < ' ') {
                    fprintf(stderr, "Password contains invalid symbols\n");
                    lsnprintf(&server_reply, "[s][0x%016x][0x%016x][4]", htonl(1), htonl(1));
                    send_message(curr_socket, server_reply);
                    invalid_password_flag = 1;
                    break;
                }
            }
            if (invalid_password_flag) {
                continue;
            }
            if (already_registered_flag) {
                if (!strcmp(clients[curr_uid]->password, password)) {
                    // здесь может происходить переполнение буфера
                    for (size_t i = 0; i < MAX_SESSIONS; ++i) {
                        if (sessions[i] == NULL) {
                            sessions[i] = (struct user_session_t *) malloc(sizeof(struct user_session_t));
                            sessions[i]->user_id = curr_uid;
                            sessions[i]->socket = curr_socket;
                            curr_session = i;
                            break;
                        }
                    }
                    break;
                } else {
                    lsnprintf(&server_reply, "[s][0x%016x][0x%016x][3]", htonl(1), htonl(1));
                    send_message(curr_socket, server_reply);
                    invalid_password_flag = 1;
                }
            } else {
                // тут может происходить переполнение буфера
                for (size_t i = 0; i < MAX_CLIENTS; ++i) {
                    if (clients[i] == NULL) {
                        curr_uid = i;
                        clients[i] = (struct user_t*) malloc(sizeof(struct user_t));
                        clients[i]->login = clients[i]->password = NULL;
                        lstrcpy(&clients[i]->login, login);
                        clients[i]->user_id = curr_uid;
                        break;
                    }
                }
                lstrcpy(&clients[curr_uid]->password, password);
                // здесь может происходить переполнение буфера
                for (size_t i = 0; i < MAX_SESSIONS; ++i) {
                    if (sessions[i] == NULL) {
                        sessions[i] = (struct user_session_t *) malloc(sizeof(struct user_session_t));
                        sessions[i]->user_id = curr_uid;
                        sessions[i]->socket = curr_socket;
                        curr_session = i;
                        break;
                    }
                }
                break;
            }
        }
    }

    lsnprintf(&server_reply, "[s][0x%016x][0x%016x][0]", htonl(1), htonl(1));
    send_message(curr_socket, server_reply);
    bzero(server_reply, lstrlen(&server_reply));

    // вынести в отдельную функцию авторизацию
    // добавить мьютексы (узнать, что это вообще такое),
    // чтобы список с пользователями нельзя было изменить, пока идёт запись

    while (1) {
        struct message_t* msg_handler = (struct message_t*) malloc(sizeof(struct message_t));
        msg_handler->text = NULL;

        if ((read_size = receive_message(curr_socket, msg_handler)) <= 0) {
            break;
        }
        // получить время сообщения (серверное, потому вычисляется здесь)
        time_t rawtime;
        time (&rawtime);
        struct tm *timeinfo = localtime (&rawtime);
        char *timestamp = calloc(64, 1);
        strftime(timestamp, 64, "%c", timeinfo);

        msg_handler->timestamp = rawtime;
        int invalid_message_flag = (msg_handler->length > MAX_MESSAGE_LEN);
        for (size_t i = 0; i < msg_handler->length && !invalid_message_flag; ++i) {
            if (msg_handler->text[i] < ' ' && msg_handler->text[i] != '\n') {
                invalid_message_flag = 1;
            }
        }
        if (invalid_message_flag) {
            lsnprintf(&server_reply, "[s][0x%016x][0x%016x][6]", htonl(1), htonl(1));
            send_message(curr_socket, server_reply);
            bzero(server_reply, lstrlen(&server_reply));
            continue;
        }
        if (msg_handler->type == 'o') {
            break;
        } else if (msg_handler->type == 'l') {
            for (size_t i = 0; i < MAX_SESSIONS; ++i) {
                if (sessions[i]) {
                    for (size_t j = 0; j < MAX_CLIENTS; ++j) {
                        if (clients[j] && sessions[i]->user_id == clients[j]->user_id) {
                            char* user_info = NULL;
                            if (curr_uid == 0) {
                                lsnprintf(&user_info, "|- login: %s, password: %s, uid: %zu, session: %zu\n",
                                          clients[j]->login, clients[j]->password, sessions[i]->user_id, i);
                            } else {
                                lsnprintf(&user_info, "|- login: %s, uid: %zu, session: %zu\n",
                                          clients[j]->login, sessions[i]->user_id, i);
                            }
                            lstrcat(&outbuff, user_info);
                            free(user_info);
                            break;
                        }
                    }
                }
            }
            lsnprintf(&enc_buff, "[l][0x%016x][0x%016x][%s]", htonl(lstrlen(&outbuff)), htonl(lstrlen(&outbuff)), outbuff);
            send_message(curr_socket, enc_buff);
            bzero(outbuff, lstrlen(&outbuff));
        } else if (msg_handler->type == 'k') {
            // check if current user is root
            for (size_t i = 0; i < MAX_SESSIONS; ++i) {
                if (sessions[i] && sessions[i]->socket == curr_socket) {
                    if (sessions[i]->user_id == 0) {
                        char* uid_to_kick = NULL;
                        char* reason = NULL;

                        char* linebreak = strchr(msg_handler->text, '\n');
                        lstrncpy(&uid_to_kick, msg_handler->text, linebreak - msg_handler->text);
                        lstrncpy(&reason, linebreak + 1, lstrlen(&msg_handler->text) - 1);

                        lstrip(&uid_to_kick);
                        lstrip(&reason);

                        if (!strcmp(uid_to_kick, "0")) {
                            lsnprintf(&server_reply, "[s][0x%016x][0x%016x][5]", htonl(1), htonl(1));
                            send_message(curr_socket, server_reply);
                            break;
                        }

                        size_t kicked_out = 0;
                        for (size_t j = 0; j < MAX_SESSIONS; ++j) {
                            if (i == j) { // нельзя кикнуть себя
                                continue;
                            }
                            if (sessions[j] && sessions[j]->user_id == atoi(uid_to_kick)) {
                                shutdown(sessions[j]->socket, SHUT_RDWR);
                                free(sessions[j]);
                                sessions[j] = NULL;
                                kicked_out = 1;
                            }
                        }
                        if (kicked_out) {
                            lsnprintf(&client_message, "[k][0x%016x][0x%016x][%s][0x%016x][%s]",
                                      htonl(lstrlen(&uid_to_kick) + lstrlen(&reason)),
                                      htonl(lstrlen(&uid_to_kick)), uid_to_kick,
                                      htonl(lstrlen(&reason)), reason
                             );
                            broadcast_message_to_all(server_reply);
                        } else {
                            lsnprintf(&server_reply, "[s][0x%016x][0x%016x][2]", htonl(1), htonl(1));
                            send_message(curr_socket, server_reply);
                        }
                    } else {
                        lsnprintf(&server_reply, "[s][0x%016x][0x%016x][5]", htonl(1), htonl(1));
                        send_message(curr_socket, server_reply);
                    }
                    break;
                }
                bzero(server_reply, lstrlen(&server_reply));
            }
        } else if (msg_handler->type == 'h') {
            lstrip(&msg_handler->text);
            int to_show_cnt = atoi(msg_handler->text);
            if (to_show_cnt > HISTORY_LEN) {
                to_show_cnt = HISTORY_LEN;
            }
            for (int i = to_show_cnt - 1; i >= 0; --i) {
                if (lstrlen(&history[i])) {
                    lsnprintf(&enc_buff, "[h][0x%016x][0x%016x][%s]",
                              htonl(lstrlen(&history[i])),
                              htonl(lstrlen(&history[i])), history[i]
                     );
                    send_message(curr_socket, enc_buff);
                    sleep(0.5);
                }
            }
        } else {
            for (int i = HISTORY_LEN - 2; i >= 0; --i) {
                lsnprintf(&history[i + 1], history[i]);
            }
            lsnprintf(&history[0], "H: %s\n%s\n%s\n", timestamp, clients[curr_uid]->login, msg_handler->text);
            // разослать сообщение пользователя всем участникам (здесь должен быть парсер с разбором случаев)
            lsnprintf(&enc_buff, "[r][0x%016x][0x%016x][%s][0x%016x][%s][0x%016x][%s]",
                      htonl(lstrlen(&timestamp) + lstrlen(&clients[curr_uid]->login) + lstrlen(&msg_handler->text)),
                      htonl(lstrlen(&timestamp)), timestamp,
                      htonl(lstrlen(&clients[curr_uid]->login)), clients[curr_uid]->login,
                      htonl(lstrlen(&msg_handler->text)), msg_handler->text
            );
            broadcast_message_to_all(enc_buff);
            bzero(enc_buff, lstrlen(&enc_buff));
        }
        bzero(msg_handler->text, lstrlen(&msg_handler->text));
        free(msg_handler->text);
        free(msg_handler);
    }

    time_t rawtime;
    time (&rawtime);
    struct tm *timeinfo = localtime (&rawtime);
    char *timestamp = calloc(64, 1);
    strftime(timestamp, 64, "%c", timeinfo);

    lsnprintf(&outbuff, "%s logged out from %d\n", clients[curr_uid]->login, curr_session);
    lsnprintf(&enc_buff, "[m][0x%016x][0x%016x][%s][0x%016x][%s]",
              htonl(lstrlen(&outbuff) + lstrlen(&timestamp)),
              htonl(lstrlen(&timestamp)), timestamp,
              htonl(lstrlen(&outbuff)),  outbuff
     );
    broadcast_message(curr_uid, enc_buff);

    free(client_message);
    free(outbuff);
    free(enc_buff);

    if (sessions[curr_session]) {
        shutdown(sessions[curr_session]->socket, SHUT_RDWR);
        free(sessions[curr_session]);
        sessions[curr_session] = NULL;
    }

    if (!read_size) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        fprintf(stderr, "recv failed");
    }

    puts("Client logged out");
    free(socket_desc);
    return EXIT_SUCCESS;
}
