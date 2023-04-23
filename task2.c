#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define STAT_MSG "Number success lock: "
#define ERROR_DIFF_PID "Error: Pid in a file is different from its\n"
#define ERROR_MISSING_ARG "Error: No argument passed: -f <myfile>\n"
#define ERROR_OPEN  "Error: Failed to open file\n"
#define ERROR_WRITE "Error: Failed to write down\n"
#define ERROR_CLOSE "Error: Failed to close\n"
#define UNKNOWN_ARG "Error: Unknown option\n"

static volatile int success_lock = 0;
static volatile int running = 1;


static void write_stat(void)
{
    running = 0;
    int fd = open("stat.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    int pid = getpid();
    int success_locks = 18;
    char msg[60] = {0};
    sprintf(msg, "%s: %d - PID[%d]\n", STAT_MSG, success_locks, pid);
    write(fd, msg, strlen(msg));
    close(fd);
}

static void handler(int sig)
{
    write_stat();
    running = 0;
}

static char* parse_args(int argc, char **argv)
{
    char *filename = NULL;
    int r;
    while ((r = getopt(argc, argv, "f:")) != -1) {
        switch (r) {
            case 'f':
                filename = optarg;
                break;
            case '?':
                fprintf(stderr, UNKNOWN_ARG);
                exit(EXIT_FAILURE);
        }
    }
    return filename;
}



int main(int argc, char **argv)
{
    signal(SIGINT, handler);

    char *flname_argv = parse_args(argc, argv);
    if (flname_argv == NULL) {
        fprintf(stderr, ERROR_MISSING_ARG);
        exit(errno);
    }
    char *filename = malloc(sizeof(char) * strlen(flname_argv) + 5);
    strcpy(filename, flname_argv);
    strcat(filename, ".lck");
    
    while (running) {
        int in = open(filename, O_CREAT | O_EXCL | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
        int pid = getpid();
        if (in == -1) {
            while (in == -1) {
                in = open(filename, O_CREAT | O_EXCL | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
                sleep(1);

                if (running == 0) {
                    exit(errno);
                }
            }
        }
        write(in, &pid, sizeof(int));
        close(in);

        sleep(1);
        in = open(filename, O_RDONLY);
        if (in == -1) { // существование
            fprintf(stderr, ERROR_OPEN); 
            write_stat();
            exit(errno);
        } 
        int actual_pid = 0;
        read(in, &actual_pid, sizeof(int));
        if (actual_pid != pid) {
            fprintf(stderr, ERROR_DIFF_PID);
            close(in);
            write_stat();
            exit(errno);
        }
    
        close(in);         
        remove(filename);
        success_lock += 1;
    }

    return errno;
}

