#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> //for INT_MAX
#include "parser.h"
#include "utils.h"

#include <unistd.h> // for fork() and execvp()
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait()

#define MAX_JOBS 1024

// Structure to keep track of background jobs
struct job {
    pid_t pid;
    char *cmd;
    int running;
};

// Global array to store background jobs
struct job jobs[MAX_JOBS];
int job_count = 0;

void terminate(char *line) {
    if (line) {
        free(line); // release memory allocated to line pointer
    }

    // Free the memory allocated for each job's command in the jobs array
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].cmd) {
            free(jobs[i].cmd);
        }
    }

    printf("bye\n");
    exit(0);
}

// Add a new job to the jobs array
void add_job(pid_t pid, char *cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        jobs[job_count].cmd = strdup(cmd);
        jobs[job_count].running = 1;
        job_count++;
    } else {
        printf("Maximum number of background jobs reached.\n");
    }
}

// Update job statuses and display them
void list_jobs() {
    printf("Background jobs:\n");
    for (int i = 0; i < job_count;) { // Notice no increment here; it will be done conditionally
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result == 0) {
            // The job is still running
            printf("[%d] %d Running: %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
            i++; // Move to the next job
        } else if (result == jobs[i].pid) {
            // The job has finished
            jobs[i].running = 0;
            printf("[%d] %d Finished: %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
            
            // Free the memory for the command string
            free(jobs[i].cmd);

            // Shift all subsequent jobs left by one position
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }

            // Decrease the job count
            job_count--;
            // Do not increment `i` here because we need to check the next job which is now at position `i`
        } else if (result == -1) {
            perror("waitpid failed");
            i++; // Move to the next job to avoid getting stuck in an infinite loop
        }
    }
}

/* Read a line from standard input and put it in a char[] */
char* readline(const char *prompt) {
    size_t buf_len = 16;
    char *buf = xmalloc(buf_len * sizeof(char));

    printf("%s", prompt);
    if (fgets(buf, buf_len, stdin) == NULL) {
        free(buf);
        return NULL;
    }

    do {
        size_t l = strlen(buf);
        if ((l > 0) && (buf[l-1] == '\n')) {
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
        char *prompt = "myshell>";

        line = readline(prompt); // line is a pointer to char (string)
        if (line == 0 || !strncmp(line, "exit", 4)) {
            terminate(line);
        } else if (!strncmp(line, "jobs", 4)) {
            list_jobs(); // Call the jobs function when "jobs" command is entered
            continue;
        } else {
            l = parsecmd(&line);
            if (l == 0) {
                terminate(0);
            } else if (l->err != 0) {
                printf("error: %s\n", l->err);
                continue;
            } else {
                if (l->in != 0) printf("in: %s\n", l->in);
                if (l->out != 0) printf("out: %s\n", l->out);
                printf("bg: %d\n", l->bg);

                for (i = 0; l->seq[i] != 0; i++) {
                    char **cmd = l->seq[i];
                    printf("seq[%d]: ", i);
                    for (j = 0; cmd[j] != 0; j++) {
                        printf("'%s' ", cmd[j]);
                    }
                    printf("\n");
                }

                printf("\n[Parent ID = %d] \n", (int)getpid());

                pid_t pid = fork();

                if (pid == 0) { // In Child process
                    for (i = 0; l->seq[i] != 0; i++) {
                        char **cmd = l->seq[i];
                        printf("\nIn Child %d, Parent is [%d] \n", (int)getpid(), (int)getppid());
                        if (execvp(cmd[0], cmd) == -1) {
                            perror("execvp failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                } else if (pid > 0) { // In Parent process
                    if (!l->bg) {
                        printf("\nWaiting for child %d to finish\n", pid);
                        int status;
                        waitpid(pid, &status, 0);
                        printf("\nChild finished with status: %d\n", status);
                    } else {
                        printf("Started background process with PID: %d\n", pid);
                        add_job(pid, line); // Add the background job
                    }
                } else {
                    perror("fork failed");
                    exit(1);
                }
            }
        }
    }
}
