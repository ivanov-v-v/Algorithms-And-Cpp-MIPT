// implement dynamic version of strcat

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef LSTRING_H_
#define LSTRING_H_

// remove delimiter symbols from the end or beginning of the string
void lstrip(char** buffer);
// copy one string to another
void lstrcpy(char** destination, char* source);
// concatenate strings
void lstrcat(char** destination, char* source);
// read a line from stdin
void lgetline(char** buffer);
// read a word from file
int lfread(FILE* fp, char** buffer);
// read a line from file
int lfgets(FILE* fp, char** buffer);
// write formatted text to string
void lsnprintf(char** message, const char* format, ...);

void lstrip(char** str) {
    int len = strlen(*str);
    if (!len) { return; }
    char *copy = (char*) calloc(len + 1, 1);
    strcpy(copy, *str);
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
    strcpy(*str, l);
}
void lstrcpy(char** destination, char* source) {
    free(*destination);
    size_t src_len = strlen(source);
    *destination = calloc(src_len + 1, 1);
    for (size_t i = 0; i < src_len; ++i) {
        (*destination)[i] = source[i];
    }
    (*destination)[src_len] = '\0';
}
void lstrcat(char** destination, char* source) {
    size_t dest_len = strlen(*destination);
    size_t src_len = strlen(source);
    char *dest_copy = (char*) calloc(dest_len + 1, 1);
    strcpy(dest_copy, *destination);
    free(*destination);
    *destination = (char*) calloc(dest_len + src_len + 1, 1);
    for (size_t i = 0; i < dest_len; ++i) {
        (*destination)[i] = dest_copy[i];
    }
    for (size_t i = 0; i < src_len; ++i) {
        (*destination)[dest_len + i] = source[i]; 
    }
    (*destination)[dest_len + src_len] = '\0';
}
void lgetline(char** buffer) {
    int len = 0;
    char ch;
    while ((ch = getchar()) != '\n') {
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
            *buffer = (char*)realloc(*buffer, 2 * bufflen + 1);
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
            *buffer = (char*)realloc(*buffer, 2 * bufflen + 1);
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

void lsnprintf(char** buffer, const char* format, ...) {
    free(*buffer);
    va_list args, dup_args;
    va_copy(dup_args, args);
    va_start (args, format);
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    va_start(dup_args, format);
    *buffer = (char*)calloc(len * 2, sizeof(char));
    vsnprintf(*buffer, len * 2, format, dup_args);
    va_end(dup_args);
}
#endif // LSTRING_H
