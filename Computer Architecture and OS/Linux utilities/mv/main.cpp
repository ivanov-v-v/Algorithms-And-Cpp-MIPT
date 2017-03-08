/* TODO: 
* 1. Add support of command line options (in particular, verbose-mode);
* 2. Write unit tests;
* 3. Check the code with valgrind;
* 4.* How to implement own class of strings in such a manner,
*     that it would be able to use them interchangeably whith c_strings?
* 5.* Implement recursive copy-and-removal to handle EXTDEV in dir-to-dir copying;
* 6. Get rid of magical constants.
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include <errno.h>
#include <ftw.h>

#define BUFFSIZE 1024

// removes the file pointed by fpath during recursive pre-order traversal of given directory
static int unlink_cb(const char *fpath, const struct stat *statbuf, int typeflag, struct FTW *ftwbuf);
static int remove_recursively(char *path);
static void copy_and_remove(char* srcpath, char* destpath);

static void print_message_and_exit(char* message);
static void print_error_and_exit();

// try to rename first, and copy if doesn't work
// assuming that srcpath exists and destpath doesn't
static void perform_move(char* srcpath, char* destpath);

/* If destination path points to non-existent file or directory:
*      If only one file or directory needs to be moved, perform move         
*      Otherwise exit with error message
*  Else if destination path points to file:
*      If there are more than one parameter to be moved, exit with error message
*      If source path to either non-existent file or directory, exit with error message
*      Remove destination file, if it exists
*      Perform move
*  Else if destination path points to directory:
*      If any of the source files is non-existent, exit with error message
*      Remove the destionation files of the same names, if any
*      Perform moves
*  Else:
*      Exit with error code
*/

// current version doesn't support command line options
int main(int argc, char** argv) {
    char* err_message = (char*)malloc(BUFFSIZE); // provides support of formatted output in error log
    if (argc < 3) {
        // at least three arguments must be provided: what and where to move;
        snprintf(err_message, BUFFSIZE, "Too few arguments: %d found, at least 3 required", argc - 1);
        print_message_and_exit(err_message);
    }
    struct stat statbuf;

    if (stat(argv[argc - 1], &statbuf)) { // destination path is non-existent
        if (argc != 3) {
            snprintf(err_message, BUFFSIZE, "%s: Destination path is not a directory", argv[argc - 1]);
            print_message_and_exit(err_message);
        }
        if (stat(argv[1], &statbuf)) { // check existense of the source file;
            snprintf(err_message, BUFFSIZE, "%s: Not a file or a directory", argv[1]);
            print_message_and_exit(err_message);
        }
        perform_move(argv[1], argv[2]);
    } else {
        if (S_ISREG(statbuf.st_mode)) { // file-to-file, w/o options;
            if (argc != 3) {
                snprintf(err_message, BUFFSIZE, "%s: Destination path is not a directory", argv[argc - 1]);
                print_message_and_exit(err_message);
            }
            if (stat(argv[1], &statbuf)) { // check existense of the source file;
                snprintf(err_message, BUFFSIZE, "%s: No such file or directory", argv[1]);
                print_message_and_exit(err_message);
            }
            if (!S_ISREG(statbuf.st_mode)) { // only files can be moved into files;
                snprintf(err_message, BUFFSIZE, "%s: Cannot overwrite destination directory", argv[1]);
                print_message_and_exit(err_message);
            }
            remove(argv[2]);
            perform_move(argv[1], argv[2]);
        } else { // destination path specifies a directory
            char* destdir = argv[argc - 1];         // dir to move into
            char* destpath = (char*)malloc(BUFFSIZE);   // new path for source file/directory
            for (size_t i = 1; i < argc - 1; ++i) {
                char* srcpath = argv[i]; // what to move
                if (stat(srcpath, &statbuf)) {
                    char* err_message = (char*)malloc(BUFFSIZE);
                    snprintf(err_message, BUFFSIZE, "%s: No such file or directory", srcpath);
                    print_message_and_exit(err_message);
                }
                snprintf(destpath, BUFFSIZE, "%s/%s", destdir, srcpath);
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

static void print_message_and_exit(char* message) {
    printf("mv: %s\n", message);
    exit(EXIT_FAILURE);
}

static void print_error_and_exit() {
    perror("mv: ");
    exit(EXIT_FAILURE);
}

static void perform_move(char* srcpath, char* destpath) {
    struct stat statbuf;
    if (rename(srcpath, destpath)) { // try to rename the srcpath
        if (errno == EXDEV) { // destination lies in another parition, copy the contents
            stat(srcpath, &statbuf); // check the 
            if (S_ISREG(statbuf.st_mode)) { // 
                copy_and_remove(srcpath, destpath);
            } else if (S_ISDIR(statbuf.st_mode)) {
                // copy recursively (not implemented yet: 
                // calls cp utility usng system() command;
                char* cmd = (char*)malloc(2 * BUFFSIZE + 5);
                snprintf(cmd, sizeof(cmd), "cp %s %s", srcpath, destpath);
                system(cmd);
                free(cmd);
                remove_recursively(srcpath);
            } else { // some other error occured
                print_error_and_exit();
            }
        } else { // handling other types of errors is currently not supported
            print_error_and_exit();
        }
    }
}