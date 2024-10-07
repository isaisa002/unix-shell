#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> //for INT_MAX
#include "parser.h"
#include "utils.h"


#include <unistd.h> // for fork() and execvp()
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait()

/*---------------------------------------------------------PART 2-------------------------------------------------*/
#define JOBS_LIST_SIZE 100


struct job{
pid_t pid;          //Process ID
char *command;       // Process command
int status;         // Flag to check if running or finished
};

struct job *job_table[JOBS_LIST_SIZE]={NULL}; // array of pointers to job struct
int job_count = 0; // Track number of jobs and their position in table

// Adding a process in list of background processes
void add_job(pid_t pid, char *command){
//Ensure that less than 100 jobs in background
    if (job_count >= JOBS_LIST_SIZE){
    fprintf(stderr, "Exceeds maximum capacity of background jobs\n");
    exit(1);
}
    //Allocate memory for a job struct
    struct job *new_job = (struct job *)malloc(sizeof(struct job));
    if (new_job == NULL) {
        perror("malloc failed");
        exit(1);
    }

    //Create a new background job 
    new_job->pid = pid;
    new_job->command = strdup(command);   // copied command passed
    new_job->status = 1; // 1 for running process

    // Add the job to the next available slot
    job_table[job_count] = new_job;
    job_count++;  // Increment the job count and go to next position of table
    printf("Job added with PID: %d at index: %d\n", pid, job_count - 1);
    

}


// Print all background processes
void print_jobs(){
printf("\n------LIST OF JOBS IN BACKGROUNG--------:\n");
    for (int i = 0; i < job_count; i++) {
        
        if (job_table[i] != NULL && job_table[i]->status) {
            printf("Index: %d, PID: %d | Command: %s | Status: %s\n", i, job_table[i]->pid, job_table[i]->command,
                   job_table[i]->status ? "Running" : "Finished");
        }
    }
}


// Flag a job as Finished
void job_finished(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (job_table[i] != NULL && job_table[i]->pid == pid) {
            job_table[i]->status = 0;  // Mark the job as finished
            printf("Index: %d, PID: %d | Command: %s | Status: %s\n", i, job_table[i]->pid, job_table[i]->command,
                   job_table[i]->status ? "Running" : "Finished");
        }            return;
        }
    }


//Delete a job in the list when finished
void delete_job(int index) {
   //check if index exists
    if (index < 0 || index >= job_count || job_table[index] == NULL) {
        fprintf(stderr, "Error: Invalid job index.\n");
        return;
    }

    struct job *job_to_delete = job_table[index];
    free(job_to_delete->command);  // Free the memory for the command string
    free(job_to_delete);  // Free the job struct itself
    job_table[index] = NULL;  // Set the job slot to NULL

    printf("Job at index %d deleted.\n", index);
}



/*--------------------------------------------------------PART 2--------------------------------------------------------------*/




// Terminate shell and clean memory
void terminate(char *line) {
    if (line)
        free(line); //release memory allocated to line pointer
    printf("bye\n");

// Free all jobs in the table
    for (int i = 0; i < job_count; i++) {
        if (job_table[i] != NULL) {
            free(job_table[i]->command);
            free(job_table[i]);
        }
    }


    
    exit(0);
}


/* Read a line from standard input and put it in a char[] */
char* readline(const char *prompt)
{
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
        char *line=0;
        int i, j;
        char *prompt = "myshell>";

        /* Readline use some internal memory structure that
           can not be cleaned at the end of the program. Thus
           one memory leak per command seems unavoidable yet */
        line = readline(prompt); // line is a pointer to char (string)
        if (line == 0 || ! strncmp(line,"exit", 4)) {
            terminate(line);
        }else if(strncmp(line, "jobs", 4) == 0){
            //PART 2: if command enters is jobs, print list of background jobs
            print_jobs();
            
        }
        else {
            /* parsecmd, free line, and set it up to 0 */
            l = parsecmd( & line);

            /* If input stream closed, normal termination */
            if (l == 0) {

                terminate(0);
            }
            else if (l->err != 0) {
                /* Syntax error, read another command */
                printf("error: %s\n", l->err);
                continue;
            }
            else {
       /* Print the sequence once (in the parent process before fork) */
                if(l->in !=0) printf("in: %s\n", l->in);
                if(l->out != 0) printf("out: %s\n", l->out);
                printf("bg: %d\n", l->bg);

                /* Display each command of the pipe */
                for (i=0; l->seq[i]!=0; i++) {
                    char **cmd = l->seq[i];
                    printf("seq[%d]: ", i);
                    for (j=0; cmd[j]!=0; j++) {
                        printf("'%s' ", cmd[j]);
                    }
                    printf("\n");
                }

                printf("\n[Parent ID = %d] \n", (int)getpid());
/* ------------------------------------------------------PART 1&2 -------------------------------------------------------------------*/
                pid_t pid = fork();

                if (pid == 0) { //  In Child process
                    for (i = 0; l->seq[i] != 0; i++) {
                        char **cmd = l->seq[i];
                        printf("\nIn Child %d, Parent is [%d] \n", (int)getpid(), (int)getppid());
                        // Execute the command and return error if failed
                        if (execvp(cmd[0], cmd) == -1) {
                            perror("execvp failed"); // Error handling for execvp
                            exit(EXIT_FAILURE); // Exit child process on failure
                        }
                        
                    }
                } else if (pid > 0) { // In Parent process
                    if (!l->bg) { // Not a background process
                        printf("\nWaiting for command to be executed by child %d \n", pid);
                        int status;
                        waitpid(pid, &status, 0); // Wait for the child process to finish
                        printf("\nCommand Complete by Child %d with status: %d\n", pid, status);
                        job_finished(pid);
                        delete_job(pid);
                        
                    } else {
                        // bg == 0, command followed by & meaning should run in background
                        printf("Started background process with PID: %d\n", pid); // Notify about background process
                        add_job(pid,line); 
                    }
                } else {
                    perror("fork failed"); // Error handling for fork
                    exit(1);
                }




                }

            }
        }
    }


