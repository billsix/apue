#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for pipe, fork, close, read, write, STDOUT_FILENO */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

int main(void) {
    int fd[2];

    if (pipe(fd) < 0) {
        err_sys("pipe error");
    }

    const pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid > 0) { /* parent */
        close(fd[0]);
        write(fd[1], "hello world\n", 12);
    } else { /* child */
        close(fd[1]);
        char line[MAXLINE];
        const int n = read(fd[0], line, MAXLINE);
        write(STDOUT_FILENO, line, n);
    }
    exit(0);
}
