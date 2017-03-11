#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>     // readlink(), getuid()

#include <dirent.h>     // DIR, dirent...

#include <asm/param.h>

#define STAT_PARAMS 52                  // # of parameters in /proc/[pid]/stat dir
#define BUFFSIZE 1024 * sizeof(char)    // maximal possible buffer length

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

    void allocate_log_storage() {
        p_name = (char*)malloc(BUFFSIZE); p_name[0] = '\0';
        p_tty_name = (char*)malloc(BUFFSIZE); p_tty_name[0] = '\0';
        __p_buffer = (char*)malloc(BUFFSIZE); __p_buffer[0] = '\0';
    }

    void deallocate_log_storage() {
        free(p_name);
        free(p_tty_name);
        free(__p_buffer);
    }

    void __deduce_tty_name_by_tty_nr(int tty_nr) {
        FILE* tty_dataf = fopen("dev_tty_data.txt", "r");
        char* minor_tty_str = (char*)malloc(BUFFSIZE);
        char* major_tty_str = (char*)malloc(BUFFSIZE);

        while (fscanf(tty_dataf, "%s %s %s", __p_buffer, major_tty_str, minor_tty_str) != EOF) {
            if (!strtod(major_tty_str, NULL) || !strtod(minor_tty_str, NULL)) {
                continue;
            }
            if (major(tty_nr) == atoi(major_tty_str) && minor(tty_nr) == atoi(minor_tty_str)) {
                snprintf(p_tty_name, BUFFSIZE, "%s", __p_buffer); 
                break;
            }
        } 

        fclose(tty_dataf);
        free(major_tty_str);
        free(minor_tty_str);

        if (strlen(p_tty_name) == 0) {
            snprintf(__p_buffer, BUFFSIZE, "/proc/%d/fd/0", p_id);
            ssize_t len = readlink(__p_buffer, p_tty_name, BUFFSIZE);
            if (len != -1) {
                p_tty_name[len] = '\0';
            } else {
                snprintf(p_tty_name, BUFFSIZE, "?");
            }
        }

        snprintf(__p_buffer, BUFFSIZE, "%s", p_tty_name);
        char* pch = strchr(__p_buffer + 1, '/');
        if (pch == NULL || !strcmp(pch + 1, "null")) {
            snprintf(p_tty_name, BUFFSIZE, "?");
        } else {
            snprintf(p_tty_name, BUFFSIZE, "%s", pch + 1);
        }
    }

    void __deduce_uid() {
        snprintf(__p_buffer, BUFFSIZE, "/proc/%d/status", p_id);
        FILE* statf = fopen(__p_buffer, "r");
        while (fscanf(statf, "%s", __p_buffer) != EOF) {
            if (!strcmp(__p_buffer, "Uid:")) {
                fscanf(statf, "%d", &p_uid);
                break;
            } 
            fscanf(statf, "%[^\n]", __p_buffer);
        }
        fclose(statf);
    }

    void parse_proc_parameters(char* process_pid_str) {
        snprintf(__p_buffer, BUFFSIZE, "/proc/%s/stat", process_pid_str);
        FILE* statf = fopen(__p_buffer, "r");

        fscanf(statf, "%d %s %c", &p_id, __p_buffer, &p_state);
        char* sep_beg = strchr(__p_buffer, '(');
        char* sep_end = strrchr(__p_buffer, ')');
        int len = sep_end - sep_beg - 1;
        strncpy(p_name, sep_beg + 1, sep_end - sep_beg - 1);
        p_name[len] = '\0';

        int tty_nr;
        long utime, cutime;
        for (size_t i = 3; i < STAT_PARAMS; ++i) {
            if (i == 6) {
                fscanf(statf, "%d", &tty_nr);
            } else if (i == 13) {
                fscanf(statf, "%ld", &utime);
            } else if (i == 15) {
                fscanf(statf, "%ld", &cutime);
            } else {
                fscanf(statf, "%s", __p_buffer);
            }
        }
        fclose(statf);

        p_cutime = (utime + cutime) / HZ;
        __deduce_tty_name_by_tty_nr(tty_nr);
        __deduce_uid();
    }
};

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

        struct process_t* process_ptr = (process_t*)malloc(sizeof(process_t));
        process_ptr->allocate_log_storage();
        process_ptr->parse_proc_parameters(entry->d_name);

        int hours = (process_ptr->p_cutime / 60 / 60) % 24;
        int minutes = (process_ptr->p_cutime / 60) % 60;
        int seconds = process_ptr->p_cutime % 60;

        if (aflag || (process_ptr->p_uid == curr_uid && process_ptr->p_state == 'R')) {
            printf("%d\t%s\t%c\t%02d:%02d:%02d %s\n", 
                process_ptr->p_id, 
                process_ptr->p_tty_name, 
                process_ptr->p_state, 
                hours, 
                minutes, 
                seconds, 
                process_ptr->p_name
            );
        }

        process_ptr->deallocate_log_storage();
        free(process_ptr);
    }

    __remove_parsed_dev_data();
    closedir(dir);
    return EXIT_SUCCESS;
}

static void __parse_dev_directory() {
    DIR *dir;
    struct dirent *entry;
    dir = opendir("/dev");
    if (!dir) {
        perror("unable to read /dev/");
        exit(1);
    }
    char* buffer = (char*)malloc(BUFFSIZE);
    while((entry = readdir(dir)) != NULL) {
        snprintf(buffer, BUFFSIZE, "echo $(echo /dev/%s && stat -c '%%t %%T' /dev/%s) >> dev_tty_data.txt", entry->d_name, entry->d_name);
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
