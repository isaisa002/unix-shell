#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> // for INT_MAX
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
    if (line)
        free(line); // release memory allocated to line pointer
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
    for (int i = 0; i < job_count; i++) {
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result == 0) {
            // The job is still running
            printf("[%d] %d Running: %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
        } else if (result == jobs[i].pid) {
            // The job has finished
            jobs[i].running = 0;
            printf("[%d] %d Finished: %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
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

        line = readline(prompt);
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

                pid_t pid = fork();
                printf("A child has been created\n");

                for (i = 0; l->seq[i] != 0; i++) {
                    char **cmd = l->seq[i];
                    printf("seq[%d]: ", i);
                    for (j = 0; cmd[j] != 0; j++) {
                        printf("'%s' ", cmd[j]);
                    }
                    printf("\n");

                    if (pid == 0) { // In Child process 
                        if (execvp(cmd[0], cmd) == -1) {
                            perror("execvp failed");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid > 0) { // In Parent process
                        if (!l->bg) {
                            int status;
                            waitpid(pid, &status, 0);
                        } else {
                            printf("Started background process with PID: %d\n", pid);
                            add_job(pid, cmd[0]); // Add the background job
                        }
                    } else {
                        perror("fork failed");
                    }
                }
            }
        }
    }
}
