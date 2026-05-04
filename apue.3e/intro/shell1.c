#include <stdio.h>     /* for fgets, printf, stdin */
#include <string.h>    /* for strlen */
#include <unistd.h>    /* for fork, execlp */
#include <stdlib.h>    /* for exit */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h>  /* for waitpid */

#include "apue.h"

int main(void) {
    char buf[MAXLINE]; /* from apue.h */
    pid_t pid;
    int status;

    printf("%% "); /* print prompt (printf requires %% to print %) */
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = 0; /* replace newline with null */
        }

        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid == 0) { /* child */
            execlp(buf, buf, (char *)0);
            err_ret("couldn't execute: %s", buf);
            exit(127);
        }

        /* parent */
        if ((pid = waitpid(pid, &status, 0)) < 0) {
            err_sys("waitpid error");
        }
        printf("%% ");
    }
    exit(0);
}
