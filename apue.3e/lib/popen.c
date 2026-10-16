#include <stdio.h>  /* for FILE, fdopen, fileno, fclose, EOF */
#include <stdlib.h> /* for calloc */
#include <errno.h>  /* for errno, EINVAL, EMFILE, EINTR */
#include <unistd.h> /* for pipe, fork, close, dup2, execl, _exit, STDIN_FILENO, STDOUT_FILENO */
#include <sys/wait.h> /* for waitpid */

#include "apue.h"

/*
 * Pointer to array allocated at run-time.
 */
static pid_t *childpid = nullptr;

/*
 * From our open_max(), {Prog openmax}.
 */
static int maxfd;

FILE *popen(const char *cmdstring, const char *type) {
    int i;
    int pfd[2];
    FILE *fp;

    /* only allow "r" or "w" */
    if ((type[0] != 'r' && type[0] != 'w') || type[1] != 0) {
        errno = EINVAL;
        return (nullptr);
    }

    if (childpid == nullptr) { /* first time through */
        /* allocate zeroed out array for child pids */
        maxfd = open_max();
        childpid = calloc(maxfd, sizeof(pid_t));
        if (childpid == nullptr) {
            return (nullptr);
        }
    }

    if (pipe(pfd) < 0) {
        return (nullptr); /* errno set by pipe() */
    }
    if (pfd[0] >= maxfd || pfd[1] >= maxfd) {
        close(pfd[0]);
        close(pfd[1]);
        errno = EMFILE;
        return (nullptr);
    }

    pid_t pid = fork();
    if (pid < 0) {
        return (nullptr);  /* errno set by fork() */
    } else if (pid == 0) { /* child */
        if (*type == 'r') {
            close(pfd[0]);
            if (pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
        } else {
            close(pfd[1]);
            if (pfd[0] != STDIN_FILENO) {
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
            }
        }

        /* close all descriptors in childpid[] */
        for (i = 0; i < maxfd; i++) {
            if (childpid[i] > 0) {
                close(i);
            }
        }

        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127);
    }

    /* parent continues... */
    if (*type == 'r') {
        close(pfd[1]);
        fp = fdopen(pfd[0], type);
        if (fp == nullptr) {
            return (nullptr);
        }
    } else {
        close(pfd[0]);
        fp = fdopen(pfd[1], type);
        if (fp == nullptr) {
            return (nullptr);
        }
    }

    childpid[fileno(fp)] = pid; /* remember child pid for this fd */
    return (fp);
}

int pclose(FILE *fp) {
    int stat;

    if (childpid == nullptr) {
        errno = EINVAL;
        return (-1); /* popen() has never been called */
    }

    int fd = fileno(fp);
    if (fd >= maxfd) {
        errno = EINVAL;
        return (-1); /* invalid file descriptor */
    }
    pid_t pid = childpid[fd];
    if (pid == 0) {
        errno = EINVAL;
        return (-1); /* fp wasn't opened by popen() */
    }

    childpid[fd] = 0;
    if (fclose(fp) == EOF) {
        return (-1);
    }

    while (waitpid(pid, &stat, 0) < 0) {
        if (errno != EINTR) {
            return (-1); /* error other than EINTR from waitpid() */
        }
    }

    return (stat); /* return child's termination status */
}
