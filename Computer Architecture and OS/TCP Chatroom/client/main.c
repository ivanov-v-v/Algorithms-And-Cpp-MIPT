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
#define MAX_SESSIONS 1000
#ifndef BUFFSIZE
#define BUFFSIZE 2000
#endif // BUFFSIZE
#define PORT 1337

size_t CONNECTION_STATE;

void* write_handler(void*);
void* read_handler(void*);

char* inbuff;
char* outbuff;
char* client_message;
char* server_reply;

int log_in(int);

int main(int argc, char** argv) {
//    const char *line = "Test string";
//    char* text = (char*) calloc(1024, 1);
//    strcpy(text, line);
//    char* buffer = (char*) calloc(BUFFSIZE, 1);
//    encode(buffer, BUFFSIZE, 'r', text);
//    puts(buffer);
//    struct message_t* msg = (struct message_t*) malloc(sizeof(struct message_t));
//    msg->text = (char*)malloc(BUFFSIZE);
//    decode(msg, buffer);
//    printf("%s", msg->text);

    int socket_desc;
    struct sockaddr_in server;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        fprintf(stderr, "Socket creation failed");
        return EXIT_FAILURE;
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.1.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        return EXIT_FAILURE;
    }
    puts("Connection established");
    CONNECTION_STATE = 1;

    int* new_socket_ptr = (int*)(malloc(sizeof(int)));
    *new_socket_ptr = socket_desc;

    inbuff = (char*) calloc(BUFFSIZE, 1);
    outbuff = (char*) calloc(BUFFSIZE, 1);

    client_message = (char*) calloc(BUFFSIZE, 1);
    server_reply = (char*) calloc(BUFFSIZE, 1);

    // здесь -- логиниться
    if (log_in(socket_desc) < 0) {
        fprintf(stderr, "Authentication failed\n");
        return EXIT_FAILURE;
    }

    printf("Authentication successful\n");

    pthread_t in_thread, out_thread;
    if (pthread_create(&in_thread, NULL, write_handler, (void*) new_socket_ptr) < 0) {
        fprintf(stderr, "Could not create a new thread\n");
        return EXIT_FAILURE;
    }
    if (pthread_create(&out_thread, NULL, read_handler, (void*) new_socket_ptr) < 0) {
        fprintf(stderr, "Could not create a new thread\n");
        return EXIT_FAILURE;
    }
    pthread_join(in_thread, NULL);
    pthread_join(out_thread, NULL);

    close(socket_desc);
    free(new_socket_ptr);
    return EXIT_SUCCESS;
}

void* write_handler(void* socket_desc) {
    // блок объявления переменных
    int curr_socket = *(int*)socket_desc;
    int read_size, curr_uid;

    while (1) {
//        int ch, len = 0;dd
//        while ((ch = getchar()) != '\n') {
//            client_message[len] = ch;
//            ++len;
//        }
        // игнорировать пустое
        lgetline(&client_message);
        size_t len = strlen(client_message);
        if (!len) {
            continue;
        }
        client_message[len] = '\n';
        ++len;

        // добавить разделить в конце
        client_message[len] = '\0';
        char message_type;
        if (!strcmp(client_message, "/exit\n")) {
            snprintf(client_message, BUFFSIZE, "logout");
            message_type = 'o';
        } else if (!strcmp(client_message, "/list\n")) {
            // нормально ли decode проглотит пустое сообщение?
            client_message[0] = '\0';
//            snprintf(client_message, BUFFSIZE, "display users online");
            message_type = 'l';
        } else if (!strcmp(client_message, "/kick\n")) {
            char* uid = (char*) calloc(BUFFSIZE, 1);
            char* reason = (char*) calloc(BUFFSIZE, 1);
            scanf("%s\n", uid);
//            for(len = 0; (ch = getchar()) != '\n'; ++len) {
//                reason[len] = ch;
//            }
//            reason[len] = '\0';
            lgetline(&reason);
            char* start_pos = client_message;
            start_pos += snprintf(start_pos, BUFFSIZE, "%s\n", uid);
            snprintf(start_pos, BUFFSIZE, "%s\n", reason);
            message_type = 'k';
        } else if (!strcmp(client_message, "/history\n")) {
            size_t to_show_cnt;
            scanf("%zu", &to_show_cnt);
            snprintf(client_message, BUFFSIZE, "%zu\n", to_show_cnt);
            message_type = 'h';
        } else {
            message_type = 'r';
        }
        // отослать на сервер
        if (send_message(curr_socket, client_message, message_type) <= 0) {
            puts("Disconnected from the server");
            break;
        }
        bzero(client_message, strlen(client_message));
        if (message_type == 'o') {
            break;
        }
    }
    puts("Writing thread has finished");
    pthread_exit(NULL);
}

void* read_handler(void* socket_desc) {
    int curr_socket = *(int*)socket_desc;
    int read_size, curr_uid;

    struct message_t* msg_handler = (struct message_t*) malloc(sizeof(struct message_t));
    msg_handler->text = (char*)malloc(BUFFSIZE);

    while(1) {
        if (receive_message(curr_socket, msg_handler) <= 0) {
            break;
        }
        // вывести ответ
        printf("%s", msg_handler->text);
        fflush(stdout);
        bzero(server_reply, strlen(server_reply));
        bzero(msg_handler->text, strlen(msg_handler->text));
    }
    if (read_size < 0) {
        puts("No response received");
    }

    free(msg_handler->text);
    free(msg_handler);
    free(inbuff);
    free(server_reply);
    puts("Reading thread has finished");
    pthread_exit(NULL);
}

int log_in(int curr_socket) {
    char* login_buff = (char*) calloc(BUFFSIZE, 1);
    char* password_buff = (char*) calloc(BUFFSIZE, 1);

    int authentication_status;

    while (1) {
        printf("login: ");
        lgetline(&login_buff);
        printf("password: ");
        lgetline(&password_buff);

        snprintf(client_message, BUFFSIZE, "%s\n%s", login_buff, password_buff);
        if (send_message(curr_socket, client_message, 'i') < 0) {
            fprintf(stderr, "Data not sent\n");
            authentication_status = -1;
            break;
        }

        if (recv(curr_socket, server_reply, BUFFSIZE, 0) <= 0) {
            fprintf(stderr, "Empty or corrupted response from server\n");
            authentication_status = -1;
            break;
        }

        struct message_t* msg = (struct message_t*) malloc(sizeof(struct message_t));
        msg->text = (char*)malloc(BUFFSIZE);
        decode(msg, server_reply);
        lstrip(&msg->text);
        printf("%s\n", msg->text);

        // вывести ответ
        if (msg->type != 's' || strlen(msg->text) > 1 || msg->text[0] == '1') {
            fprintf(stderr, "Incorrect response type or length\n");
            authentication_status = -1;
            break;
        }
        char response_code = msg->text[0];
        if (response_code == '0') {
            printf("Logged in successfully\n");
            authentication_status = 0;
            break;
        }
        if (response_code == '3') {
            fprintf(stderr, "Authentication failure: incorrect credentials\n");
    }
        if (response_code == '5') {
            fprintf(stderr, "Authentication failure: account is banned\n");
            authentication_status = -1;
        }
        fflush(stdout);
        bzero(server_reply, strlen(server_reply));
    }

    free(login_buff);
    free(password_buff);
    return authentication_status;
}