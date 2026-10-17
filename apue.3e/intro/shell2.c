#include <stdio.h>     /* for fgets, printf, stdin */
#include <string.h>    /* for strlen */
#include <unistd.h>    /* for fork, execlp */
#include <stdlib.h>    /* for exit */
#include <signal.h>    /* for signal, SIGINT, SIG_ERR */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h>  /* for waitpid */

#include "apue.h"

static void sig_int(int); /* our signal-catching function */

int main(void) {
    char buf[MAXLINE]; /* from apue.h */

    if (signal(SIGINT, sig_int) == SIG_ERR) {
        err_sys("signal error");
    }

    printf("%% "); /* print prompt (printf requires %% to print %) */
    while (fgets(buf, MAXLINE, stdin) != nullptr) {
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = 0; /* replace newline with null */
        }

        pid_t pid = fork();
        if (pid < 0) {
            err_sys("fork error");
        } else if (pid == 0) { /* child */
            execlp(buf, buf, (char *)0);
            err_ret("couldn't execute: %s", buf);
            exit(127);
        }

        /* parent */
        int status;
        pid = waitpid(pid, &status, 0);
        if (pid < 0) {
            err_sys("waitpid error");
        }
        printf("%% ");
    }
    exit(0);
}

void sig_int([[maybe_unused]] int signo) { printf("interrupt\n%% "); }
