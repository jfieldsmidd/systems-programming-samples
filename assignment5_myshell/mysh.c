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
        char *program_name = NULL;
        int pipe_to_input = 0;
        int pipe_to_output = 0;
        char *args_buf[12];
        int former_fds[2] = {0, 1};
        int fds[2] = {0, 1};
        pid_t child_pid;
        int fork_count = 0;

        program_name = strtok(input_buf, " \n");

        // runs program according to input and output specifications
        // (one iteration handles one function call)
        while (program_name) {

            if (!strcmp(program_name, "exit")) {
                exit(0);
            }

            // for iterations other than first
            if (pipe_to_output) {
                pipe_to_input = 1;
                former_fds[0] = fds[0];
                former_fds[1] = fds[1];
                fds[0] = 0;
                fds[1] = 1;
            }
            pipe_to_output = 0;

            // get args
            args_buf[0] = program_name;
            args_buf[1] = NULL;
            int args_count = 1;
            char *current_arg;
            while((current_arg = strtok(NULL, " \n"))) {
                if (!strcmp(current_arg, "<") || !strcmp(current_arg, ">") ||
                                !strcmp(current_arg, ">>") || !strcmp(current_arg, "|")) {
                    break;
                } else {
                    args_buf[args_count] = current_arg;
                    args_buf[args_count + 1] =   NULL;
                    args_count ++;
                }
            }

            // check if input or output are specified
            char *input_name = NULL;
            char *output_name = NULL;
            int append_to_output = 0;
            while (current_arg) {
                if (!strcmp(current_arg, "<")) {
                    current_arg = strtok(NULL, " \n");
                    input_name = current_arg;
                    current_arg = strtok(NULL, " \n");
                } else if (!strncmp(current_arg, ">", 1)) {
                    if (!strcmp(current_arg, ">>")) {
                        append_to_output = 1;
                    }
                    current_arg = strtok(NULL, " \n");
                    output_name = current_arg;
                    current_arg = strtok(NULL, " \n");
                } else if (*current_arg == '|') {
                    pipe_to_output = 1;
                    break;
                }
            }

            // make sure write end of pipe we're reading from is closed
            if (pipe_to_input) {
                if (close(former_fds[1]) == -1) {
                    perror("close");
                    exit(1);
                }
            }

            // open pipe to write output to
            if (pipe_to_output) {
                if (pipe(fds) == -1) {
                    perror("pipe");
                    exit(1);
                }
            }
            fork_count += 1;
            child_pid = fork();
            if (child_pid == -1) {
                perror("child");
                exit(1);
            }
            // child process
            if (child_pid == 0) {

                // set fd for input to program
                if (pipe_to_input) {
                    if (dup2(former_fds[0], 0) == -1) {
                        perror("dup2");
                        exit(1);
                    }
                } else if (input_name) {
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

                // set fd for output from program
                if (pipe_to_output) {
                    if (dup2(fds[1], 1) == -1) {
                        perror("dup2");
                        exit(1);
                    }
                } else if (output_name) {
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
                // execute the program
                if (execvp(program_name, args_buf) == -1) {
                    perror("execv");
                    exit(1);
                }
            }
            // go to next piped program, if applicable
            program_name = strtok(NULL, " \n");
        }
        // wait for all the children to terminate
        for (int i = fork_count; i > 0 ; i--) {
            wait(NULL);
        }
        write(1, prompt, sizeof(prompt));
    }
    return 0;
}
