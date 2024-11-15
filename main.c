#include <fcntl.h>   // For open()
#include <limits.h>  // for INT_MAX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  // for pid_t
#include <sys/wait.h>   // for wait()
#include <unistd.h>     // for fork(), execvp(), dup(), dup2(), close()

#include "parser.h"
#include "utils.h"

//----------------------------------------PART2-------------------------------------------------
#define MAX_JOBS 100

// Structure to keep track of background jobs
struct job {
    pid_t pid;      // process ID
    char *command;  // Process command
    int status;     // Flag to check if running or finished
};

// Global array to store background jobs
struct job jobs[MAX_JOBS];
int job_count = 0;  // Track number of jobs and position in table

// Add a new job to the jobs array
void add_job(pid_t pid, char *command) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        jobs[job_count].command = strdup(command);
        jobs[job_count].status = 1;  // 1 For running process
        job_count++;
        printf("[JOB ID = %d] Added in background list\n", pid);
    } else {
        printf("Maximum number of background jobs reached.\n");
    }
}

// Update job statuses and display them
void print_jobs() {
    printf("------------------Background jobs------------------\n");
    for (int i = 0; i < job_count; i++) {
        // Check if job finished before printing
        //  Waitpid options: WNOHANG makes call non blocking, return if process not finished
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result == 0) {
            // Job still running
            printf("[JOB ID = %d] Running: %s\n", jobs[i].pid, jobs[i].command);
        } else if (result == jobs[i].pid) {
            // Job has finished
            jobs[i].status = 0;
            printf("[JOB ID = %d] Finished: %s\n", jobs[i].pid, jobs[i].command);

            // free memory for command string
            free(jobs[i].command);

            // Delete job in array by shifting all jobs left by one position
            for (int j = i; j < job_count; j++) {
                jobs[j] = jobs[j + 1];
            }

            // Decrease job count
            job_count--;

        } else if (result == -1) {
            perror("Check of job status failed");
        }
    }
}

//-------------------------------------------------------------------------------------------

void terminate(char *line) {
    if (line) free(line);  // release memory allocated to line pointer
    printf("bye\n");
    exit(0);
}

/* Read a line from standard input and put it in a char[] */
char *readline(const char *prompt) {
    size_t buf_len = 16;
    char *buf = xmalloc(buf_len * sizeof(char));

    printf("%s", prompt);
    if (fgets(buf, buf_len, stdin) == NULL) {
        free(buf);
        return NULL;
    }

    do {
        size_t l = strlen(buf);
        if ((l > 0) && (buf[l - 1] == '\n')) {
            l--;
            buf[l] = 0;
            return buf;
        }
        if (buf_len >= (INT_MAX / 2)) memory_error();
        buf_len *= 2;
        buf = xrealloc(buf, buf_len * sizeof(char));
        if (fgets(buf + l, buf_len - l, stdin) == NULL) return buf;
    } while (1);
}

int main(void) {
    while (1) {
        struct cmdline *l;
        char *line = 0;
        int i, j;
        char *prompt = "\nmyshell>";

        /* Readline use some internal memory structure that
        can not be cleaned at the end of the program. Thus one memory
        leak per command seems unavoidable*/
        line = readline(prompt);
        if (line == 0 || !strncmp(line, "exit", 4)) {
            terminate(line);
        } else if (!strncmp(line, "jobs", 4)) {
            print_jobs();  // PART 2: Print list of bg jobs  when "jobs" command is entered
            continue;
        } else {
            l = parsecmd(&line);
            if (l == 0) {
                terminate(0);
            } else if (l->err != 0) {
                printf("error: %s\n", l->err);
                continue;
            } else {
                // Print input and output redirections if specified
                if (l->in != 0) printf("in: %s\n", l->in);
                if (l->out != 0) printf("out: %s\n", l->out);
                printf("bg: %d\n", l->bg);

// ---------------------------------------------------PART 4-5----------------------------------------

                /*To hold pipe file descriptors:
                        pipe_fds[0]:  Read part of pipe
                        pipe_fds[1]: Write part of pipe*/
                int pipe_fds[2];
                int prev_cmd = -1;  // Previous read end for chaining pipes
                                    // Initialized at -1 bcs first command don't have previous

//------------------------------------------------------PART 1 to 5 -----------------------------------

                // loop through each command in sequence and print it once
                for (i = 0; l->seq[i] != 0; i++) {
                    char **command = l->seq[i];
                    // Print sequence
                    printf("seq[%d]: ", i);
                    for (j = 0; command[j] != 0; j++) {
                        printf("'%s' ", command[j]);
                    }
                    printf("\n");
                    printf("PARENT ID = %d\n", (int)getppid());

        // PART 4-5: CREATE PIPE IF THERE'S ANOTHER COMMAND IN SEQUENCE
                    if (l->seq[i + 1] != 0) {
                        // Create a pipe and return error if failed
                        if (pipe(pipe_fds) == -1) {
                            perror("pipe failed");
                            exit(EXIT_FAILURE);
                        }
                    }

                    pid_t pid = fork();
                    if (pid == 0) {  // In Child process

        // PART 4-5: SIMPLE PIPE: for cmnd1 | cmd2

                        // Currently at cmd1
                        if (l->seq[i + 1] != 0) {
                            // Redirects output of cmd1 to write end of pipe instead ofterminal
                            dup2(pipe_fds[1], STDOUT_FILENO);
                            close(pipe_fds[0]);  // Close read end of cmd1 because using read end of pipe
                            close(pipe_fds[1]);  // Close original write end after duplicating it
                        }
                        // Currently at cmd2
                        if (i > 0) {  // Not at the first command of the sequence
                            // Redirects input of cmd2 to read end of cmd1 instead of keyboard
                            dup2(prev_cmd, STDIN_FILENO);
                            close(prev_cmd);  // Close file descriptor for previous pipe
                        }

        // PART 3: HANDLE INPUT REDIRECTION
                        if (l->in != 0) {
                            int fd_in = open(l->in, O_RDONLY);
                            if (fd_in == -1) {
                                perror("Error opening input file");
                                exit(EXIT_FAILURE);
                            }
                            if (dup2(fd_in, STDIN_FILENO) == -1) {
                                perror("Error redirecting input");
                                exit(EXIT_FAILURE);
                            }
                            close(fd_in);
                        }

        // PART 3: HANDLE OUTPUT REDIRECTION
                        if (l->out != 0) {
                            int fd_out = open(l->out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (fd_out == -1) {
                                perror("Error opening output file");
                                exit(EXIT_FAILURE);
                            }
                            if (dup2(fd_out, STDOUT_FILENO) == -1) {
                                perror("Error redirecting output");
                                exit(EXIT_FAILURE);
                            }
                            close(fd_out);
                        }

        // PART1: Execute the command and return error if failed
                        if (execvp(command[0], command) == -1) {
                            perror("execvp failed");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid > 0) {  // In Parent process

        // PART 2: Handle background processes
                        if (!l->bg) {  // Not a background process
                            int status;
                            printf("Command being executed by Child %d\n", pid);
                            waitpid(pid, &status, 0);
                            printf("Command completed by Child %d\n", pid);
                        } else {
                            // bg = 0: Background process since entered command followed by &
                            printf("[JOB ID = %d]Started in background\n", pid);
                            add_job(pid, command[0]);  // Add the background job
                        }

        // PART 4-5: CLOSE
                        // currently at cmd1
                        if (l->seq[i + 1] != 0) {
                            close(pipe_fds[1]);     // Close write part of cmd1
                            prev_cmd = pipe_fds[0];  // Save read end for next command
                        }

                    } else {
                        perror("fork failed");
                        exit(1);
                    }
                }
            }
        }
    }
}
