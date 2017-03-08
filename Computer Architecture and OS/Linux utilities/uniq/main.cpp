/* TODO:
0. Add support of non-fixed string length.
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

int main(int argc, char** argv) {
    size_t MAX_LINE_LEN = sizeof(char) * 1024;
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

    char* prev_line = (char*)malloc(MAX_LINE_LEN);
    char* curr_line = (char*)malloc(MAX_LINE_LEN);
    if (prev_line == NULL || curr_line == NULL) {
        err(1, "unable to allocate buffer");
    }
    if (fgets(prev_line, MAX_LINE_LEN, input) == NULL) {
        err(1, "input file is empty or corrupted");
    }
    int counter = 1;
    while (fgets(curr_line, MAX_LINE_LEN, input) != NULL) {
        if (strcmp(prev_line, curr_line)) {
            if (cflag) {
                fprintf(output, "%d ", counter);
                counter = 0;
            }
            fprintf(output, "%s", prev_line);
            char *tmp = prev_line;
            prev_line = curr_line;
            curr_line = tmp;
        }
        ++counter;
    }
    if (cflag) {
        fprintf(output, "%d ", counter);
    }
    fprintf(output, "%s", prev_line);

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
