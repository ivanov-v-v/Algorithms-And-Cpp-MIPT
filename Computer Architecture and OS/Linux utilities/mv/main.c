/* TODO: 
* 1. get_current_dir_name(), unlink_cb(), fcloseall() are all GNU-dependent,
     hence current code version is not portable
*/

 #define _XOPEN_SOURCE 500 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <errno.h>
#include <ftw.h>

// removes the file pointed by fpath during recursive pre-order traversal of given directory
static int unlink_cb(const char *fpath, const struct stat *statbuf, int typeflag, struct FTW *ftwbuf);
static int remove_recursively(char *path);
static void copy_and_remove(char* srcpath, char* destpath);

static void write_formatted_message_to_string(char** message, const char* format, ...);
static void print_message_and_exit(char* message);
static void print_error_and_exit();

static int longest_common_prefix_length(char* path1, char* path2);
static void perform_move(char* srcpath, char* destpath);

/* Convert all arguments to absolute paths
*  If destination path points to non-existent file or directory:
*      If only one file or directory needs to be moved, perform move         
*      Otherwise exit with error message
*  Else if destination path points to file:
*      If there are more than one parameter to be moved, exit with error message
*      If source path to either non-existent file or directory, exit with error message
*      If file is going to be moved to itself, exit with error message
*      Remove destination file, if it exists
*      Perform move
*  Else if destination path points to directory:
*      If any of the source files is non-existent, exit with error message
*      If some directory is going to be moved to its own subdirectory, exit with error message
*      Remove the destionation files of the same names, if any
*      Perform moves
*  Else:
*      Exit with error code
*/

// current version doesn't support command line options
int main(int argc, char** argv) {
    char* err_message = NULL;  // provides support of formatted output in error log
    /* TODO:
    *  Transform arguments into their real paths
    *  And process them taking into consideration their longest common prefix.
    */
    if (argc < 3) {
        // at least three arguments must be provided: what and where to move;
        write_formatted_message_to_string(&err_message, "too few arguments: %d found, at least 3 expected", argc - 1);
        print_message_and_exit(err_message);
    }

    struct stat statbuf;

    char** absolute_paths = (char**)malloc(argc * sizeof(char*));
    for (int i = 1; i < argc; ++i) {
        absolute_paths[i] = NULL;
        char* parent_dir_path = NULL;
        char* ptr_to_last_slash = strrchr(argv[i], '/');
        if (ptr_to_last_slash == NULL) {
            parent_dir_path = get_current_dir_name();
        } else {
            parent_dir_path = (char*)malloc((ptr_to_last_slash - argv[i] + 1) * sizeof(char));
            strncpy(parent_dir_path, argv[i], ptr_to_last_slash - argv[i]);
        }
        absolute_paths[i] = realpath(parent_dir_path, NULL);
        if (!absolute_paths[i]) {
            write_formatted_message_to_string(&err_message, "\'%s\': invalid path", argv[i]);
            print_message_and_exit(err_message);
        }
        if (ptr_to_last_slash != NULL) {
            absolute_paths[i] = (char*)realloc(absolute_paths[i], sizeof(char) * (strlen(absolute_paths[i]) + strlen(ptr_to_last_slash) + 1));
            strcat(absolute_paths[i], ptr_to_last_slash);
        } else {
            // to be refactored
            absolute_paths[i] = (char*)realloc(absolute_paths[i], sizeof(char) * (strlen(absolute_paths[i]) + 1 +  strlen(argv[i]) + 1));
            strcat(absolute_paths[i], "/");
            strcat(absolute_paths[i], argv[i]);
        }
        printf("%d: %s\n", i, absolute_paths[i]);
        free(parent_dir_path);
    }
    if (stat(absolute_paths[argc - 1], &statbuf)) { // destination path is non-existent
        puts("HERE");
        if (argc != 3) {
            write_formatted_message_to_string(&err_message, "\'%s\': destination path is not a directory", argv[argc - 1]);
            print_message_and_exit(err_message);
        }
        if (stat(absolute_paths[1], &statbuf)) { // check existense of the source file;
            write_formatted_message_to_string(&err_message, "\'%s\': not a file or a directory", argv[1]);
            print_message_and_exit(err_message);
        }
        perform_move(absolute_paths[1], absolute_paths[2]);
    } else {
        if (S_ISREG(statbuf.st_mode)) { // file-to-file, w/o options;
            if (argc != 3) {
                write_formatted_message_to_string(&err_message, "\'%s\': destination path is not a directory", argv[argc - 1]);
                print_message_and_exit(err_message);
            }
            if (stat(absolute_paths[1], &statbuf)) { // check existense of the source file;
                write_formatted_message_to_string(&err_message, "\'%s\': not a file or directory", argv[1]);
                print_message_and_exit(err_message);
            }
            if (!S_ISREG(statbuf.st_mode)) { // only files can be moved into files;
                write_formatted_message_to_string(&err_message, "\'%s\': cannot overwrite destination directory", argv[1]);
                print_message_and_exit(err_message);
            }
            if (!strcmp(absolute_paths[1], absolute_paths[2])) {
                write_formatted_message_to_string(&err_message, "\'%s\' and \'%s\' are the same file", absolute_paths[1], absolute_paths[1]);
                print_message_and_exit(err_message);
            }
            remove(absolute_paths[2]);
            perform_move(absolute_paths[1], absolute_paths[2]);
        } else { // destination path specifies a directory
            char* destdir = absolute_paths[argc - 1];         // dir to move into
            char* destpath = NULL;   // new path for source file/directory
            for (int i = 1; i < argc - 1; ++i) {
                char* srcpath = absolute_paths[i]; // what to move
                if (stat(srcpath, &statbuf)) {
                    write_formatted_message_to_string(&err_message, "%s: not a file or directory", srcpath);
                    print_message_and_exit(err_message);
                }
                int lcp_len = longest_common_prefix_length(srcpath, destdir);
                if (lcp_len == strlen(srcpath)) {
                    write_formatted_message_to_string(&err_message, "cannot move \'%s\' to a subdirectory of itself", absolute_paths[i]);
                    print_message_and_exit(err_message);
                }
                if (srcpath[strlen(srcpath) - 1] == '/') {
                    srcpath[strlen(srcpath) - 1] = '\0';
                }
                char* name_ptr = strrchr(srcpath, '/') + 1;
                write_formatted_message_to_string(&destpath, "%s/%s", destdir, name_ptr);
                printf("—----->%s\n", destpath);
                if (!stat(destpath, &statbuf)) {    // if destpath is occupied
                    if (S_ISREG(statbuf.st_mode)) { // with a file, 
                        remove(destpath); // remove it;
                    } else if (S_ISDIR(statbuf.st_mode)) { // with a folder —
                        remove_recursively(destpath); // remove it with all the contents;
                    } else { // some other error occured
                        print_error_and_exit();
                    }
                }
                perform_move(srcpath, destpath);
            }
            free(destpath);
        }
    }
    for (size_t i = 1; i < argc; ++i) {
        free(absolute_paths[i]);
    }
    free(absolute_paths);
    free(err_message);
    return EXIT_SUCCESS;
}

// removes the file pointed by fpath during recursive pre-order traversal of given directory
static int unlink_cb(const char *fpath, const struct stat *statbuf, int typeflag, struct FTW *ftwbuf) {
    int rv = remove(fpath);
    if (rv) {
        perror(fpath);
    }
    return rv;
}

static int remove_recursively(char *path) {
    // "ftw() walks through the directory tree that is located under the directory dirpath,
    // and calls unlink() once for each entry in the tree. By default, directories are handled
    // before the files and subdirectories they contain (preorder traversal)."
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

static void copy_and_remove(char* srcpath, char* destpath) {
    FILE* inpf = fopen(srcpath, "r");
    FILE* outf = fopen(destpath, "w");

    char curr;
    while ((curr = fgetc(inpf)) != EOF) {
        fputc(curr, outf);
    }

    fcloseall();
    remove(srcpath);
}

static void write_formatted_message_to_string(char** buffer, const char* format, ...) {
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

static void print_message_and_exit(char* message) {
    printf("mv: %s\n", message);
    exit(EXIT_FAILURE);
}

static void print_error_and_exit() {
    perror("mv: ");
    exit(EXIT_FAILURE);
}

static int longest_common_prefix_length(char* path1, char* path2) {
    int n = strlen(path1), 
            m = strlen(path2);
    int lcp_len = 0;
    for (size_t i = 0; i < n && i < m; ++i) {
        if (path1[i] != path2[i]) {
            break;
        } 
        ++lcp_len;
    }
    return lcp_len;
}

static void perform_move(char* srcpath, char* destpath) {
    struct stat statbuf;
    if (rename(srcpath, destpath)) { // try to rename the srcpath
        if (errno == EXDEV) { // destination lies in another parition, copy the contents
            stat(srcpath, &statbuf); // check the existence of the source file
            if (S_ISREG(statbuf.st_mode)) { 
                copy_and_remove(srcpath, destpath);
            } else if (S_ISDIR(statbuf.st_mode)) {
                // copy recursively (not implemented yet: 
                // calls cp utility usng system() command),
                // to be rewritten
                char* cmd = NULL; 
                write_formatted_message_to_string(&cmd, "cp %s %s", srcpath, destpath);
                system(cmd);
                free(cmd);
                remove_recursively(srcpath);
            } else { // some other error occured
                print_error_and_exit();
            }
        } else { // handling other types of errors is currently not supported
            printf("Error occured: %s %s\n", srcpath, destpath);
            print_error_and_exit();
        }
    }
}
