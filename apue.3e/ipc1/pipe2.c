#include <stdio.h>  /* for FILE, fopen, fgets, ferror */
#include <stdlib.h> /* for exit, getenv */
#include <string.h> /* for strlen, strrchr */
#include <unistd.h> /* for pipe, fork, close, write, dup2, execl, STDIN_FILENO */
#include <sys/wait.h>  /* for waitpid */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

#define DEF_PAGER "/bin/more" /* default pager program */

int main(int argc, char *argv[]) {
    int fd[2];

    if (argc != 2) {
        err_quit("usage: a.out <pathname>");
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == nullptr) {
        err_sys("can't open %s", argv[1]);
    }
    if (pipe(fd) < 0) {
        err_sys("pipe error");
    }

    const pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid > 0) { /* parent */
        close(fd[0]);     /* close read end */

        /* parent copies argv[1] to pipe */
        char line[MAXLINE];
        while (fgets(line, MAXLINE, fp) != nullptr) {
            int n = strlen(line);
            if (write(fd[1], line, n) != n) {
                err_sys("write error to pipe");
            }
        }
        if (ferror(fp)) {
            err_sys("fgets error");
        }

        close(fd[1]); /* close write end of pipe for reader */

        if (waitpid(pid, nullptr, 0) < 0) {
            err_sys("waitpid error");
        }
        exit(0);
    } else {          /* child */
        close(fd[1]); /* close write end */
        if (fd[0] != STDIN_FILENO) {
            if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO) {
                err_sys("dup2 error to stdin");
            }
            close(fd[0]); /* don't need this after dup2 */
        }

        /* get arguments for execl() */
        const char *pager = getenv("PAGER");
        if (pager == nullptr) {
            pager = DEF_PAGER;
        }
        const char *argv0 = strrchr(pager, '/');
        if (argv0 != nullptr) {
            argv0++; /* step past rightmost slash */
        } else {
            argv0 = pager; /* no slash in pager */
        }

        if (execl(pager, argv0, (char *)0) < 0) {
            err_sys("execl error for %s", pager);
        }
    }
    exit(0);
}
