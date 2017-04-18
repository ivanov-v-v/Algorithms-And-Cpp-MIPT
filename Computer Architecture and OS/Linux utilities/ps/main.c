#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>     // readlink(), getuid()

#include <dirent.h>     // DIR, dirent...

#include <asm/param.h>

#define STAT_PARAMS 52                  // # of parameters in /proc/[pid]/stat dir

static int read_arbitrarily_long_word(FILE* fp, char** buffer);
static void write_formatted_message_to_string(char** message, const char* format, ...);

static void __parse_dev_directory(void);    // as the list of tty devices is system-dependent, ps parses /dev to get it
static void __remove_parsed_dev_data(void); // file containing this data gets removed in the end

static void display_usage_format(void);     // human-readable format of supportedps call synthax

struct process_t {

    int p_id;           // process identifier assigned by system
    int p_uid;          // identifier of the used who called the process
    int p_cutime;       // cumulative CPU time the process used so far

    char* p_name;       // human-readable process name
    char* p_tty_name;   // human-readable controlling tty name
    char* __p_buffer;   // hidden buffer used throughout the process_t methods

    char p_state;       // current state of the process
};

void allocate_log_storage(struct process_t* proc);
void deallocate_log_storage(struct process_t* proc);
void __deduce_tty_name_by_tty_nr(struct process_t* proc, int tty_nr);
void __deduce_uid(struct process_t* proc);
void parse_proc_parameters(struct process_t* proc, char* process_pid_str);

int main(int argc, char** argv) {
    int ch = 0;
    int aflag = 0;

    while ((ch = getopt(argc, argv, "A")) != -1) {
        switch (ch) {
            case 'A':
                aflag = 1;
                break;
            case '?':
            default:
                display_usage_format();
        }
    }

    argc -= optind;
    argv += optind;

    DIR *dir;
    struct dirent *entry;
    dir = opendir("/proc");
    if (!dir) {
        perror("diropen");
        exit(1);
    }

    __parse_dev_directory();

    int curr_uid = getuid();
    printf("PID\tTTY\tSTATE\t    TIME CMD\n");
    while((entry = readdir(dir)) != NULL) {
        if (!strtod(entry->d_name, NULL)) { // numeric dir name points to process
            continue;
        }

        struct process_t* process_ptr;
        process_ptr = (struct process_t*)malloc(sizeof(struct process_t));
        allocate_log_storage(process_ptr);
        parse_proc_parameters(process_ptr, entry->d_name);

        int hours = (process_ptr->p_cutime / 60 / 60) % 24;
        int minutes = (process_ptr->p_cutime / 60) % 60;
        int seconds = process_ptr->p_cutime % 60;

        if (aflag || (process_ptr->p_uid == curr_uid && process_ptr->p_state == 'R')) {
            printf("%d\t%s\t%02d:%02d:%02d %s\n",
                   process_ptr->p_id,
                   process_ptr->p_tty_name,
                   hours,
                   minutes,
                   seconds,
                   process_ptr->p_name
            );
        }

        deallocate_log_storage(process_ptr);
        free(process_ptr);
    }

    __remove_parsed_dev_data();
    closedir(dir);
    return EXIT_SUCCESS;
}

static int read_arbitrarily_long_word(FILE* fp, char** buffer) {
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

static void __parse_dev_directory() {
    DIR *dir;
    struct dirent *entry;
    dir = opendir("/dev");
    if (!dir) {
        perror("unable to read /dev/");
        exit(1);
    }
    char* buffer = NULL;
    while((entry = readdir(dir)) != NULL) {
        write_formatted_message_to_string(&buffer, "echo $(echo /dev/%s && stat -c '%%t %%T' /dev/%s) >> dev_tty_data.txt",
                                          entry->d_name, entry->d_name);
        system(buffer);
    }
    free(buffer);
    closedir(dir);
}

static void __remove_parsed_dev_data() {
    remove("dev_tty_data.txt");
}

static void display_usage_format(void) {
    (void)fprintf(stderr, "usage: ps [-A]");
    exit(EXIT_FAILURE);
}

void allocate_log_storage(struct process_t* proc) {
    if (proc == NULL) {
        return;
    }
    proc->p_name = NULL;
    proc->p_tty_name = NULL;
    proc->__p_buffer = NULL;
}


void deallocate_log_storage(struct process_t* proc) {
    if (proc == NULL) {
        return;
    }
    free(proc->p_name);
    free(proc->p_tty_name);
    free(proc->__p_buffer);
}

void __deduce_tty_name_by_tty_nr(struct process_t* proc, int tty_nr) {
    FILE* tty_dataf = fopen("dev_tty_data.txt", "r");
    char* minor_tty_str = NULL;
    char* major_tty_str = NULL;

    while (
            read_arbitrarily_long_word(tty_dataf, &proc->__p_buffer) &&
            read_arbitrarily_long_word(tty_dataf, &major_tty_str) &&
            read_arbitrarily_long_word(tty_dataf, &minor_tty_str)
    ) {
        if (!strtod(major_tty_str, NULL) || !strtod(minor_tty_str, NULL)) {
            continue;
        }
        if (major(tty_nr) == atoi(major_tty_str) && minor(tty_nr) == atoi(minor_tty_str)) {
            write_formatted_message_to_string(&proc->p_tty_name, "%s", proc->__p_buffer);
            break;
        }
    }

    fclose(tty_dataf);
    free(major_tty_str);
    free(minor_tty_str);

    if (proc->p_tty_name == NULL || !strlen(proc->p_tty_name)) {
        write_formatted_message_to_string(&proc->__p_buffer, "/proc/%d/fd/0", proc->p_id);

        struct stat sb;
        lstat(proc->__p_buffer, &sb);
        int len = sb.st_size + 1;
        free(proc->p_tty_name);
        proc->p_tty_name = (char*)calloc(len, sizeof(char));

        int len_written = readlink(proc->__p_buffer, proc->p_tty_name, len);
        if (len_written != -1) {
            proc->p_tty_name[len_written] = '\0';
        } else {
            write_formatted_message_to_string(&proc->p_tty_name, "?");
        }
    }

    write_formatted_message_to_string(&proc->__p_buffer, "%s", proc->p_tty_name);
    char* pch = strchr(proc->__p_buffer + 1, '/');
    if (pch == NULL || !strcmp(pch + 1, "null")) {
        write_formatted_message_to_string(&proc->p_tty_name, "?");
    } else {
        write_formatted_message_to_string(&proc->p_tty_name, "%s", pch + 1);
    }
}

void __deduce_uid(struct process_t* proc) {
    write_formatted_message_to_string(&proc->__p_buffer, "/proc/%d/status", proc->p_id);
    FILE* statf = fopen(proc->__p_buffer, "r");
    while (read_arbitrarily_long_word(statf, &proc->__p_buffer)) {
        if (!strcmp(proc->__p_buffer, "Uid:")) {
            fscanf(statf, "%d", &proc->p_uid);
            break;
        }
        char ch;
        while ((ch = fgetc(statf)) != EOF && ch != '\n');
    }
    fclose(statf);
}

void parse_proc_parameters(struct process_t* proc, char* process_pid_str) {
    write_formatted_message_to_string(&proc->__p_buffer, "/proc/%s/stat", process_pid_str);
    FILE* statf = fopen(proc->__p_buffer, "r");

    fscanf(statf, "%d ", &proc->p_id);
    read_arbitrarily_long_word(statf, &proc->__p_buffer);
    fscanf(statf, " %c", &proc->p_state);

    char* sep_beg = strchr(proc->__p_buffer, '(');
    char* sep_end = strrchr(proc->__p_buffer, ')');
    int len = sep_end - sep_beg - 1;

    proc->p_name = (char*) calloc(len + 1, sizeof(char));
    strncpy(proc->p_name, sep_beg + 1, len);
    proc->p_name[len] = '\0';

    int tty_nr;
    long utime, cutime;
    for (size_t i = 3; i < STAT_PARAMS; ++i) {
        if (i == 6) {
            fscanf(statf, "%d ", &tty_nr);
        } else if (i == 13) {
            fscanf(statf, "%ld ", &utime);
        } else if (i == 15) {
            fscanf(statf, "%ld ", &cutime);
        } else {
            read_arbitrarily_long_word(statf, &proc->__p_buffer);
        }
    }
    fclose(statf);

    proc->p_cutime = (utime + cutime) / HZ;
    __deduce_tty_name_by_tty_nr(proc, tty_nr);
    __deduce_uid(proc);
}
