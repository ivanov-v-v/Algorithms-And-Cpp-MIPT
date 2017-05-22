// implement dynamic version of strcat

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef LSTRING_H_
#define LSTRING_H_

size_t lstrlen(char** buffer); // get string's length, or return 0 if it is NULL
void lstrip(char** buffer); // remove delimiter symbols from the end or beginning of the string
void lstrcpy(char** destination, char* source); // copy one string to another
void lstrncpy(char** destination, char* source, size_t chunk_len); // copy specified number of bits
void lstrcat(char** destination, char* source); // concatenate strings
void lgetline(char** buffer); // read a line from stdin
int lfread(FILE* fp, char** buffer); // read a word from file
int lfgets(FILE* fp, char** buffer); // read a line from file
size_t lsnprintf(char** message, const char* format, ...); // write formatted text to string

size_t lstrlen(char** buffer) {
    return ((*buffer) == NULL) ? 0 : strlen(*buffer);
}
void lstrcpy(char** destination, char* source) {
    free(*destination);
    size_t src_len = lstrlen(&source);
    *destination = calloc(src_len + 1, 1);
    for (size_t i = 0; i < src_len; ++i) {
        (*destination)[i] = source[i];
    }
    (*destination)[src_len] = '\0';
}
void lstrncpy(char** destination, char* source, size_t chunk_len) {
    free(*destination);
    size_t src_len = lstrlen(&source);
    *destination = calloc(chunk_len + 1, 1);
    for (size_t i = 0; i < chunk_len; ++i) {
        if (i > src_len) {
            (*destination)[i] = '\0';
        } else {
            (*destination)[i] = source[i];
        }
    }
    (*destination)[chunk_len] = '\0';
}
void lstrcat(char** destination, char* source) {
    size_t dest_len = lstrlen(destination);
    size_t src_len = lstrlen(&source);
    char *dest_copy =  calloc(dest_len + 1, 1);
    lstrcpy(&dest_copy, *destination);
    free(*destination);
    *destination =  calloc(dest_len + src_len + 1, 1);
    for (size_t i = 0; i < dest_len; ++i) {
        (*destination)[i] = dest_copy[i];
    }
    for (size_t i = 0; i < src_len; ++i) {
        (*destination)[dest_len + i] = source[i];
    }
    (*destination)[dest_len + src_len] = '\0';
}
void lstrip(char** str) {
    size_t len = lstrlen(str);
    if (!len) { return; }
    char *copy =  calloc(len + 1, 1);
    lstrcpy(&copy, *str);
    char *l = copy, *r = copy + len - 1;
    while (isspace(*l) || isspace(*r)) {
        if (isspace(*l)) {
            *l = '\0';
            ++l;
        }
        if (isspace(*r)) {
            *r = '\0';
            --r;
        }
    }
    bzero(*str, len);
    lstrcpy(str, l);
}
void lgetline(char** buffer) {
    size_t curr_bufflen, len;

    curr_bufflen = 1;
    free(*buffer);
    *buffer = calloc(curr_bufflen, 1);

    char ch;
    len = 0;
    while ((ch = getchar()) != '\n') {
        if (len == curr_bufflen) {
            curr_bufflen = 2 * curr_bufflen + 1;
            *buffer = realloc(*buffer, curr_bufflen);
        }
        (*buffer)[len] = ch;
        ++len;
    } 
    (*buffer)[len] = '\0';
}
int lfread(FILE* fp, char** buffer) {
    free(*buffer);
    *buffer = NULL;

    char ch;
    int lastn = 0;
    int bufflen = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (!bufflen || lastn == bufflen) {
            *buffer = realloc(*buffer, 2 * bufflen + 1);
            bufflen = 2 * bufflen + 1;
        }
        if (ch == '\n' || ch == ' ' || ch == '\t') {
            break;
        }
        (*buffer)[lastn] = ch;
        ++lastn;
    }
    if (*buffer != NULL) {
        (*buffer)[lastn] = '\0';
    }
    return !(ch == EOF);
}

int lfgets(FILE* fp, char** buffer) {
    free(*buffer);
    *buffer = NULL;

    char ch;
    int lastn = 0;
    int bufflen = 0;
    while ((ch = fgetc(fp)) != EOF) {
        if (!bufflen || lastn == bufflen) {
            *buffer = realloc(*buffer, 2 * bufflen + 1);
            bufflen = 2 * bufflen + 1;
        }
        if (ch == '\n') {
            break;
        }
        (*buffer)[lastn] = ch;
        ++lastn;
    }
    if (*buffer != NULL) {
        (*buffer)[lastn] = '\0';
    }
    return !(ch == EOF);
}

size_t lsnprintf(char** buffer, const char* format, ...) {
    free(*buffer);
    va_list args, dup_args;
    va_copy(dup_args, args);
    va_start (args, format);
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    va_start(dup_args, format);
    *buffer = calloc(len * 2, sizeof(char));
    vsnprintf(*buffer, len * 2, format, dup_args);
    va_end(dup_args);
    return lstrlen(buffer);
}
#endif // LSTRING_H
