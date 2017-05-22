#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "lstring.h"
#include "msgformat.h"

#ifndef BUFFSIZE
#define BUFFSIZE 2000
#endif // BUFFSIZE
#define PORT 1337

void* write_handler(void*);
void* read_handler(void*);

char* inbuff = NULL;
char* client_message = NULL;
char* server_reply = NULL;

int log_in(int);

int main(int argc, char** argv) {
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

    int* new_socket_ptr = (int*)(malloc(sizeof(int)));
    *new_socket_ptr = socket_desc;

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

    while (1) {
        // игнорировать пустое
        lgetline(&inbuff);
        size_t len = lstrlen(&inbuff);
        if (!len) {
            continue;
        }
        inbuff[len] = '\n';
        ++len;

        // добавить разделить в конце
        inbuff[len] = '\0';
        lstrip(&inbuff);
//        printf("INBUFF: %s\n", inbuff);
        char message_type;
        if (!strcmp(inbuff, "/logout")) {
            lsnprintf(&client_message, "[o][0x%016x][0x%016x][]", htonl(0), htonl(0));
            message_type = 'o';
        } else if (!strcmp(inbuff, "/list")) {
            lsnprintf(&client_message, "[l][0x%016x][0x%016x][]", htonl(0), htonl(0));
            message_type = 'l';
        } else if (!strcmp(inbuff, "/kick")) {
            char* uid =  calloc(BUFFSIZE, 1);
            char* reason =  calloc(BUFFSIZE, 1);
            printf("uid: ");
            lgetline(&uid);
            printf("reason: ");
            lgetline(&reason);
            lsnprintf(&client_message, "[k][0x%016x][0x%016x][%s][0x%016x][%s]",
                      htonl(lstrlen(&uid) + lstrlen(&reason)),
                      htonl(lstrlen(&uid)), uid,
                      htonl(lstrlen(&reason)), reason
             );
            message_type = 'k';
        } else if (!strcmp(inbuff, "/history")) {
            char* to_show_cnt = calloc(30, 1);
            printf("len: ");
            scanf("%s", to_show_cnt);
            lsnprintf(&client_message, "[h][0x%016x][0x%016x][%s]",
                      htonl(lstrlen(&to_show_cnt)),
                      htonl(lstrlen(&to_show_cnt)), to_show_cnt
             );
            free(to_show_cnt);
            message_type = 'h';
        } else {
            lsnprintf(&client_message, "[r][0x%016x][0x%016x][%s]",
                      htonl(lstrlen(&inbuff)),
                      htonl(lstrlen(&inbuff)), inbuff
             );
            message_type = 'r';
        }
        // отослать на сервер
        if (send_message(curr_socket, client_message) <= 0) {
            puts("Disconnected from the server");
            break;
        }
        bzero(client_message, lstrlen(&client_message));
        bzero(inbuff, lstrlen(&inbuff));
        if (message_type == 'o') {
            break;
        }
    }
    puts("Writing thread has finished");
    pthread_exit(NULL);
}

void* read_handler(void* socket_desc) {
    int curr_socket = *(int*)socket_desc;
    int read_size;

    struct message_t* msg_handler = (struct message_t*) malloc(sizeof(struct message_t));
    msg_handler->text = malloc(BUFFSIZE);

    while(1) {
        if ((read_size = receive_message(curr_socket, msg_handler)) <= 0) {
            break;
        }
        // вывести ответ
        printf("%s\n", msg_handler->text);
        fflush(stdout);
        bzero(server_reply, lstrlen(&server_reply));
        bzero(msg_handler->text, lstrlen(&msg_handler->text));
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
    char* login_buff =  calloc(BUFFSIZE, 1);
    char* password_buff =  calloc(BUFFSIZE, 1);
    int authentication_status;

    struct message_t* auth_handler = (struct message_t*) malloc(sizeof(struct message_t));
    auth_handler->text = malloc(BUFFSIZE);

    while (1) {
        printf("login: ");
        lgetline(&login_buff);
        printf("password: ");
        lgetline(&password_buff);

        lsnprintf(&client_message, "[i][0x%016x][0x%016x][%s][0x%016x][%s]",
                  htonl(lstrlen(&login_buff) + lstrlen(&password_buff)),
                  htonl(lstrlen(&login_buff)), login_buff,
                  htonl(lstrlen(&password_buff)), password_buff
         );

        if (send_message(curr_socket, client_message) < 0) {
            fprintf(stderr, "Data not sent\n");
            authentication_status = -1;
            break;
        }

        if (receive_message(curr_socket, auth_handler) <= 0) {
            fprintf(stderr, "Empty or corrupted response from server\n");
            authentication_status = -1;
            break;
        }

        lstrip(&auth_handler->text);
        printf("%s\n", auth_handler->text);

        // вывести ответ
        if (auth_handler->type != 's' || lstrlen(&auth_handler->text) > 1 || auth_handler->text[0] == '1') {
            fprintf(stderr, "Incorrect response type or length\n");
            authentication_status = -1;
            break;
        }
        char response_code = auth_handler->text[0];
        if (response_code == '0') {
            printf("Logged in successfully\n");
            authentication_status = 0;
            break;
        }
        if (response_code == '3') {
            fprintf(stderr, "Authentication failure: incorrect credentials\n");
        }
        if (response_code == '4') {
            fprintf(stderr, "Registration failure: incorrect data\n");
        }
        if (response_code == '5') {
            fprintf(stderr, "Authentication failure: account is banned\n");
            authentication_status = -1;
            break;
        }
        fflush(stdout);
        bzero(server_reply, lstrlen(&server_reply));
    }

    free(login_buff);
    free(password_buff);
    free(auth_handler->text);
    free(auth_handler);
    return authentication_status;
}