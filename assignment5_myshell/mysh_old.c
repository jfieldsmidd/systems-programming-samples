/*
 * mysh.c
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 4096

const char prompt[3] = "$ ";

int
main(int argc, char *argv[]) {
    char input_buf[BUF_SIZE];
    write(1, prompt, sizeof(prompt));

    // infinite loop of reading from terminal and handling the data
    while(fgets(input_buf, BUF_SIZE, stdin)) {
        char program_name[BUF_SIZE];
        char input_name[BUF_SIZE];
        input_name[0] = '\0';
        char output_name[BUF_SIZE];
        output_name[0] = '\0';
        int append_to_output = 0;
        char *args_buf[12];
        char pipe_to_name[BUF_SIZE];
        pipe_to_name[0] = '\0';
        char *pipe_to_args_buf[12];
        int args_count;
        char *current_arg;
        int fds[2];
        pid_t child_pid;

        // handle program name and args
        strncpy(program_name, strtok(input_buf, " \n"), BUF_SIZE);
        args_buf[0] = program_name;
        args_buf[1] = NULL;
        args_count = 1;
        while((current_arg = strtok(NULL, " \n"))) {
            if (*current_arg == '<' || *current_arg == '>' || *current_arg == '|') {
                break;
            } else {
                args_buf[args_count] = current_arg;
                args_buf[args_count + 1] =   NULL;
                args_count ++;
            }
        }

        // handle args specifying input and output locations
        while (current_arg) {
            if (*current_arg == '<') {
                current_arg = strtok(NULL, " \n");
                snprintf(input_name, BUF_SIZE, "%s", current_arg);
                current_arg = strtok(NULL, " \n");
            } else if (*current_arg == '>') {
                if (*(current_arg + 1) == '>') {
                    append_to_output = 1;
                }
                current_arg = strtok(NULL, " \n");
                snprintf(output_name, BUF_SIZE, "%s", current_arg);
                current_arg = strtok(NULL, " \n");
            } else if (*current_arg == '|') {
                if (pipe(fds) == -1) {
                    perror("pipe");
                    exit(1);
                }
                strncpy(pipe_to_name, strtok(NULL, " \n"), BUF_SIZE);
                pipe_to_args_buf[0] = pipe_to_name;
                pipe_to_args_buf[1] = NULL;
                args_count = 1;
                while((current_arg = strtok(NULL, " \n"))) {
                    if (*current_arg == '<' || *current_arg == '>' || *current_arg == '|') {
                        break;
                    } else {
                        pipe_to_args_buf[args_count] = current_arg;
                        pipe_to_args_buf[args_count + 1] =   NULL;
                        args_count ++;
                    }
                }
            } else {
                break;
            }
        }
        child_pid = fork();
        if (child_pid == 0) {
            // child process
            if (pipe_to_name[0] != '\0') {
                dup2(fds[1], 1);
            }
            if (output_name[0] != '\0') {
                int flags = O_WRONLY | O_CREAT | O_TRUNC;
                if (append_to_output) {
                    flags = O_WRONLY | O_CREAT | O_APPEND;
                }
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP;
                int out_fd = open(output_name, flags, mode);
                if (out_fd == -1) {
                    perror("open");
                    exit(1);
                }
                if (dup2(out_fd, 1) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }
            if (input_name[0] != '\0') {
                int in_fd = open(input_name, O_RDONLY);
                if (in_fd == -1) {
                    perror("open");
                    exit(1);
                }
                if (dup2(in_fd, 0) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }
            if (execvp(program_name, args_buf) == -1) {
                perror("execv");
                exit(1);
            }
        } else if (child_pid == -1){
            perror("fork");
            exit(1);
        } else {
            if (pipe_to_name[0] != '\0') {
                if (close(fds[1]) == -1) {
                    perror("close");
                    exit(1);
                }
                child_pid = fork();
                if (child_pid == 0) {
                    dup2(fds[0], 0);
                    if (execvp(pipe_to_name, pipe_to_args_buf) == -1) {
                        perror("execv");
                        exit(1);
                    }
                } else if (child_pid == -1){
                    perror("fork");
                    exit(1);
                } else {
                    wait(NULL);
                }
            }
            wait(NULL);
        }
        write(1, prompt, sizeof(prompt));
    }
    return 0;
}
