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
    int length;
    char *text;
    time_t timestamp;
};

void encode(char* encoded_msg, size_t enc_size, char type, char *text) {
    char *buffer, *tok, *end;
    size_t text_len, line_len, lines_cnt;

    text_len = strlen(text);
    buffer = (char*) calloc(text_len + 1, 1);
    strcpy(buffer, text);
    lstrip(&buffer);
    text_len = strlen(buffer);
    snprintf(encoded_msg, BUFFSIZE, "[%c][%zu]", type, text_len);
    tok = end = buffer;
    lines_cnt = 0;
    while (tok != NULL) {
        strsep(&end, "\r\n");
        line_len = strlen(tok);
        char* encoded_line = (char*) calloc(line_len + 20, 1);
        snprintf(encoded_line, line_len + 20, "[%zu %s]", line_len, tok);

        strcat(encoded_msg, encoded_line);
        tok = end;
        ++lines_cnt;
    }
    free(buffer);

    // char *buffer;
    // int text_len, word_len;

    // text_len = strlen(text);
    // if (text[text_len - 1] != '\n') {
    //     ++text_len;
    // }
    // char* text_copy = (char*) calloc(text_len + 1, 1);
    // strcpy(text_copy, text);
    // tokenize(text_copy);

    // snprintf(encoded_msg, enc_size, "[%c][%d]", type, text_len);

    // buffer = (char*) calloc(BUFFSIZE, 1);
    // for (size_t i = 0; i < text_len; ) {
    //     word_len = strlen(text_copy + i);
    //     snprintf(buffer, BUFFSIZE, "[%d %s]", word_len + 1, text_copy + i);
    //     strcat(encoded_msg, buffer);
    //     i += word_len + 1;
    // }
    // free(buffer);
    // free(text_copy);
}

int decode(struct message_t* msg_handler, char* encoded_msg) {
    if (!msg_handler) {
        fprintf(stderr, "Message_t argument is not initialized");
        // т.к. такое может произойти только из-за ошибки в коде
        exit(EXIT_FAILURE);
    }
    char* lastn = encoded_msg;
    msg_handler->type = *(lastn + 1);
    lastn += 4;
    while (isdigit(*lastn)) {
        msg_handler->length = msg_handler->length * 10 + (*lastn - '0');
        ++lastn;
    }
    ++lastn;
    int i = 0, curr_len = 0, tot_len = -1, cnt = 0;
    while (1) {
        if (*lastn == '\0' && tot_len == -1) {
            // дошли до конца строки
            msg_handler->text[i] = '\0';
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
            while (curr_len < tot_len) {
                ++lastn;
                ++curr_len;
                msg_handler->text[i] = *lastn;
                ++i;
            }
            msg_handler->text[i] = '\n';
            ++i;
            tot_len = -1;
            lastn += 2;
        }
        ++cnt;
    }
}

int send_message(int rcvr_socket, char* message, char type) {
    char* outbuff = (char*) calloc(BUFFSIZE, 1);
    encode(outbuff, 65535 + 1, type, message);
    int response_code = send(rcvr_socket, outbuff, strlen(outbuff), 0);
    free(outbuff);
    return response_code;
}

int receive_message(int sender_socket, struct message_t* msg_handler) {
    char* incoming_message = (char*) calloc(BUFFSIZE, 1);
    int bytes_sent = recv(sender_socket, incoming_message, BUFFSIZE, 0);
    if (bytes_sent >= 0) {
        incoming_message[bytes_sent] = '\0';
        decode(msg_handler, incoming_message);
    }
    free(incoming_message);
    return bytes_sent;
}

#endif // MSGFORMAT_H_
