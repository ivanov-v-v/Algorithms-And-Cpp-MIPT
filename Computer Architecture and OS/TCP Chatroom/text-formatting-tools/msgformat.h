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

void encode_hexstr(char** result, size_t num) {
    free(*result);
    *result = NULL;
    num = htonl(num);
    char* hex_str = NULL;
    size_t bytes_filled = 0;
    while (num) {
        char *hex_byte = NULL;
        lsnprintf(&hex_byte, "0x%02x", num & 0x00FF);
        lstrcat(&hex_str, hex_byte);
        if (num >> 8) {
            lstrcat(&hex_str, " ");
        }
        num >>= 8;
        ++bytes_filled;
    }
    while (bytes_filled != 8) {
        lstrcat(result, "0x00");
        if (bytes_filled + 1 != 8) {
            lstrcat(result, " ");
        }
        ++bytes_filled;
    }
    if (hex_str) {
        lstrcat(result, " ");
        lstrcat(result, hex_str);
    }
//    printf("%d\n", lstrlen(result));
}
size_t decode_hexstr(char** hexstr) {
    size_t result = 0;
    char* curr_pos = *hexstr;
    for (size_t i = 0; i < 8; ++i) {
        char *byte_end = curr_pos + 5;
        size_t tail = strtoul(curr_pos, &byte_end, 16);
        result = (result << 8) + tail;
        curr_pos = byte_end;
    }
    return result;
}

void encode(char** encoded_msg, char type, char *text) {
    char *buffer, *tok, *end, *hex_repr = NULL;
    size_t text_len, line_len, lines_cnt;

    text_len = lstrlen(&text);
    buffer = calloc(text_len + 1, 1);
    lstrcpy(&buffer, text);
    lstrip(&buffer);
    text_len = lstrlen(&buffer);
    encode_hexstr(&hex_repr, text_len);
//    printf("%d <> %d\n", text_len, decode_hexstr(&hex_repr));
    lsnprintf(encoded_msg, "[%c][%s]", type, hex_repr);
    tok = end = buffer;
    lines_cnt = 0;
    while (tok != NULL) {
        strsep(&end, "\r\n");
        line_len = lstrlen(&tok);
        char* encoded_line = calloc(line_len + 20, 1);
        encode_hexstr(&hex_repr, line_len);
        lsnprintf(&encoded_line, "[%s][%s]", hex_repr, tok);
        lstrcat(encoded_msg, encoded_line);
        tok = end;
        ++lines_cnt;
        free(encoded_line);
    }
    free(buffer);
}

int decode(struct message_t* msg_handler, char* encoded_msg) {
//    printf("ENCODED: %s\n", encoded_msg);
    if (!msg_handler) {
        fprintf(stderr, "Message_t argument is not initialized");
        // т.к. такое может произойти только из-за ошибки в коде
        exit(EXIT_FAILURE);
    }
    char* lastn = encoded_msg;
    msg_handler->type = *(lastn + 1);
    lastn += 4;
    msg_handler->length = decode_hexstr(&lastn);
    free(msg_handler->text);
    msg_handler->text = calloc(msg_handler->length + 1, 1);
    lastn += 40;
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
            tot_len = decode_hexstr(&lastn);
            lastn += 40;
            ++lastn;

            curr_len = 0;
            while (curr_len < tot_len) {
                msg_handler->text[i] = *lastn;
                ++lastn;
                ++curr_len;
                ++i;
            }
            msg_handler->text[i] = '\n';
            ++i;
            tot_len = -1;
            ++lastn;
        }
        ++cnt;
    }
}

int send_message(int rcvr_socket, char* message, char type) {
    size_t message_len = lstrlen(&message);
    char* outbuff = calloc(message_len + 1, 1);
    encode(&outbuff, type, message);
    int response_code = send(rcvr_socket, outbuff, lstrlen(&outbuff) + 1, 0);
    free(outbuff);
    return response_code;
}

int receive_message(int sender_socket, struct message_t* msg_handler) {
    char* incoming_message = calloc(BUFFSIZE, 1);
    size_t message_len = recv(sender_socket, incoming_message, BUFFSIZE, MSG_PEEK | MSG_TRUNC);
    incoming_message = realloc(incoming_message, message_len + 1);
    msg_handler->text = realloc(msg_handler->text, message_len + 1);
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
