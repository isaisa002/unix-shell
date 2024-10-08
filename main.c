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

struct job jobs[JOBS_LIST_SIZE]; // array of pointers to job struct
int job_count = 0; // Track number of jobs and their position in table

// Adding a process in list of background processes
void add_job(pid_t pid, char *command){
//Ensure that less than 100 jobs in background
    if (job_count >= JOBS_LIST_SIZE){
    fprintf(stderr, "Exceeds maximum capacity of background jobs\n");
    exit(1);
}else {
       jobs[job_count].pid = pid;
       jobs[job_count].command=strdup(command);
       jobs[job_count].status=1; // 1 for running
       job_count++;
    printf("Job added with PID: %d at index: %d\n", pid, job_count - 1);

}

}


// Print all background processes
void print_jobs(){
printf("\n------LIST OF JOBS IN BACKGROUNG--------:\n");
    for (int i = 0; i < job_count; i++) {
        //check if jobs finished before printinf
        int status;
        //Options: WNOHANG makes call non-blocking, return if child not finished yet 
        pid_t check_result = waitpid(jobs[i].pid,&status,WNOHANG);    
        if(check_result==0){
            //job running
            printf("\n[JOB ID = %d] %s --> Running\n", jobs[i].pid, jobs[i].command);
        } else if(check_result==jobs[i].pid){
            //job finished
            jobs[i].status = 0; //0 for finished
            printf("\n[JOB ID = %d] %s --> DONE \n", jobs[i].pid, jobs[i].command);

            //free memory for command string
            free(jobs[i].command);

            //Delete job in array by shifting all jobs left by one position
            for(int j=i;j<job_count-1;j++){
                jobs[j]=jobs[j+1];
            }
            //Decrease job count
            job_count--;


        } else if(check_result==-1){
            printf("\n[JOB ID = %d]", jobs[i].pid);
            perror(" Check of Background job status failed");
            i++; // Don't get stuck, move to the next job to check its status
        }
        
    }
}








/*--------------------------------------------------------PART 2--------------------------------------------------------------*/




// Terminate shell and clean memory
void terminate(char *line) {
    if (line)
        free(line); //release memory allocated to line pointer
    

// Free all jobs in the array
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].command) {
            free(jobs[i].command);
        }
    }


    printf("bye\n");
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
                            exit(1); // Exit child process on failure
                        }
                        
                    }
                } else if (pid > 0) { // In Parent process
                    if (!l->bg) { // Not a background process
                        printf("\nWaiting for command to be executed by child %d \n", pid);
                        int status;
                        waitpid(pid, &status, 0); // Wait for the child process to finish
                        printf("\nCommand Complete by Child %d with status: %d\n", pid, status);
                       
                        
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


