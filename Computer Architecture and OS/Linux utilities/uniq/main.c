/* TODO:
1. Implement a library for string processing.
2. Add support of other flags from specification.
3. Write man-page.
4. Add comments.
5. Write unit-tests.
*/

#include <stdio.h>  
#include <stdlib.h> 
#include <string.h> // strcmp()
#include <err.h>    // err()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> // getopt()
#include <locale.h>

static void display_usage_format(void);
static int read_arbitrarily_long_line(FILE* fp, char** buffer);

int main(int argc, char** argv) {
    int cflag = 0;
    struct stat statbuf;

    FILE* input;
    FILE* output;
    int ch = 0;

    while ((ch = getopt(argc, argv, "c")) != -1) {
        switch (ch) {
            case 'c': 
                cflag = 1;
                break;
            case '?':
            default:
                display_usage_format();
        }
    }

    argc -= optind;
    argv += optind;
    if (argc > 2) {
        display_usage_format();
    }

    input = stdin;
    output = stdout;
    if (argc > 0 && strcmp(argv[0], "-")) {
        if (stat(argv[0], &statbuf)) {
            err(1, "input file is empty or corrupted");
        }
        input = fopen(argv[0], "r");
    }
    if (argc > 1 && argv[1]) {
        output = fopen(argv[1], "w");
    }

    char* prev_line = NULL;
    char* curr_line = NULL;
    if (!read_arbitrarily_long_line(input, &prev_line)) {
        err(1, "input file is empty or corrupted");
    }
    int counter = 1;
    while (read_arbitrarily_long_line(input, &curr_line)) {
        if (strcmp(prev_line, curr_line)) {
            if (cflag) {
                fprintf(output, "%d ", counter);
                counter = 0;
            }
            fprintf(output, "%s\n", prev_line);
            char *tmp = prev_line;
            prev_line = curr_line;
            curr_line = tmp;
        }
        free(curr_line);
        curr_line = (char*)malloc(sizeof(char));
        ++counter;
    }
    if (cflag) {
        fprintf(output, "%d ", counter);
    }
    fprintf(output, "%s\n", prev_line);

    free(prev_line);
    free(curr_line);
    fclose(input);
    fclose(output);
    return 0;
}

static void display_usage_format(void) {
    (void)fprintf(stderr, "usage: uniq [-c | -d | -u] [-i] [-f fields] [-s chars] [input [output]]");
    exit(1);   
}

static int read_arbitrarily_long_line(FILE* fp, char** buffer) {
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