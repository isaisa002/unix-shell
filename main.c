#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> //for INT_MAX
#include "parser.h"
#include "utils.h"


#include <unistd.h> // for fork() and execvp()
#include <sys/types.h> // for pid_t
#include <sys/wait.h> // for wait()




void terminate(char *line) {
    if (line)
        free(line); //release memory allocated to line pointer
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
/* ------------------------------------------------------PART 1 -------------------------------------------------------------------*/
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
                        printf("\nWaiting for child %d to finish\n", pid);
                        int status;
                        waitpid(pid, &status, 0); // Wait for the child process to finish
                        printf("\nChild finished with status: %d\n", status);
                    } else {
                        // bg == 0, command followed by & meaning should run in background
                        printf("Started background process with PID: %d\n", pid); // Notify about background process
                    }
                } else {
                    perror("fork failed"); // Error handling for fork
                    exit(1);
                }




                }

            }
        }
    }


