// implement dynamic version of strcat

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef BUFFSIZE
    #define BUFFSIZE 2000
#endif // BUFFSIZE

#ifndef MSGFORMAT_H_
#define MSGFORMAT_H_

struct message_t {
    char type;
    size_t length;
    char *text;
    time_t timestamp;
};

int decode(struct message_t* msg_handler, char* encoded_msg) {
    if (!msg_handler) {
        fprintf(stderr, "Message_t argument is not initialized");
        // т.к. такое может произойти только из-за ошибки в коде
        exit(EXIT_FAILURE);
    }
    FILE *in;
    in = fmemopen(encoded_msg, lstrlen(&encoded_msg), "r");
    fscanf(in, "[%c]", &msg_handler->type);
    fscanf(in, "[%x]", &msg_handler->length);
    msg_handler->length = ntohl(msg_handler->length);
    int multiline = 0;
    for (;;) {
        int line_len;
        if (fscanf(in, "[%x]", &line_len) <= 0) {
            break;
        }
        if (multiline) {
            lstrcat(&msg_handler->text, "\n");
        }
        line_len = ntohl(line_len);
        char *line = NULL;
        line = calloc(line_len + 1, 1);
        // [
        fgetc(in);
        // line_start
        for (size_t i = 0; i < line_len; ++i) {
            line[i] = fgetc(in);
        }
        line[line_len] = '\0';
        // line_end
        fgetc(in);
        // ]
        if (!line_len) {
            lsnprintf(&line, "\n");
        }
        lstrcat(&msg_handler->text, line);
        free(line);
        multiline = 1;
    }
}

int send_message(int rcvr_socket, char* message) {
    int response_code = send(rcvr_socket, message, lstrlen(&message) + 1, 0);
    return response_code;
}

int receive_message(int sender_socket, struct message_t* msg_handler) {
    char* incoming_message = calloc(BUFFSIZE, 1);
    size_t message_len = recv(sender_socket, incoming_message, BUFFSIZE, MSG_PEEK | MSG_TRUNC);
    free(incoming_message);
    incoming_message = calloc(message_len + 1, 1);
    free(msg_handler->text);
    msg_handler->text = calloc(message_len + 1, 1);
    int bytes_received = recv(sender_socket, incoming_message, message_len + 1, 0);
    if (bytes_received >= 0) {
        lstrip(&incoming_message);
        incoming_message[bytes_received] = '\0';
        decode(msg_handler, incoming_message);
        lstrip(&incoming_message);
    }
    free(incoming_message);
    return bytes_received;
}

#endif // MSGFORMAT_H_
