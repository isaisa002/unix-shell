#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> // for INT_MAX
#include "parser.h"
#include "utils.h"

#include <unistd.h> // for fork() and execvp()
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait()




//--------------------------------------------------------PART 2-------------------------------------------------
#define MAX_JOBS 100

// Structure to keep track of background jobs
struct job {
    pid_t pid;          // process ID
    char *command;      // Process command
    int status;         // Flag to check if running or finished
};

// Global array to store background jobs
struct job jobs[MAX_JOBS];                
int job_count = 0;             //Track number of jobs and position in table



// Add a new job to the jobs array
void add_job(pid_t pid, char *command) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        jobs[job_count].command = strdup(command);
        jobs[job_count].status = 1;               // 1 For running process
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
        //Check if job finished before printing
        // Waitpid options: WNOHANG makes call non blocking, return if process not finished
        int status;
        pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
        if (result == 0) {
            // Job still running
            printf("[JOB ID = %d] Running: %s\n", jobs[i].pid, jobs[i].command);
        } else if (result == jobs[i].pid) {
            // The job has finished
            jobs[i].status = 0;
            printf("[JOB ID = %d] Finished: %s\n", jobs[i].pid, jobs[i].command);
        

        //free memory for command string
        free(jobs[i].command);

        //Delete job in array by shifting all jobs left by one position
        for(int j = i ; j<job_count;j++){
            jobs[j]=jobs[j+1];

        }

        //Decrease job count
        job_count--;
        
        } else if(result==-1){
            perror("Check of job status failed");
        }
    }
}


void terminate(char *line) {
    if (line)
        free(line); // release memory allocated to line pointer
    printf("bye\n");
    exit(0);
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

        /* Readline use some internal memory structure that
        can not be cleaned at the end of the program. Thus one memory 
        leak per command seems unavoidable*/
        line = readline(prompt);
        if (line == 0 || !strncmp(line, "exit", 4)) {
            terminate(line);
        } else if (!strncmp(line, "jobs", 4)) {
            print_jobs(); // PART 2: Call the jobs function when "jobs" command is entered
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




//------------------------------------------------------PART 1&2 -----------------------------------
                
                

                for (i = 0; l->seq[i] != 0; i++) {
                    char **command = l->seq[i];
                    printf("seq[%d]: ", i);
                    for (j = 0; command[j] != 0; j++) {
                        printf("'%s' ", command[j]);
                    }
                    printf("\n");
                    printf("PARENT ID = %d\n",(int)getppid());
                    pid_t pid = fork();
                    if (pid == 0) { // In Child process 
                        if (execvp(command[0], command) == -1) {
                            perror("execvp failed");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid > 0) { // In Parent process
                        if (!l->bg) {   // Not a background process
                            int status;
                            printf("Command beign executed by Child %d\n", pid);
                            waitpid(pid, &status, 0);
                            printf("Command completed by Child %d\n", pid);
                        } else {
                            // bg = 0: Background process since entered command followed by &
                            printf("[JOB ID = %d]Started in background\n", pid);
                            add_job(pid, command[0]); // Add the background job
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
