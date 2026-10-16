#include <stdlib.h> /* for exit */
#include <unistd.h> /* for read, write, alarm, STDIN_FILENO, STDOUT_FILENO */
#include <signal.h> /* for signal, SIG_ERR, SIGALRM */

#include "apue.h"

static void sig_alrm(int);

int main(void) {
    if (signal(SIGALRM, sig_alrm) == SIG_ERR) {
        err_sys("signal(SIGALRM) error");
    }

    alarm(10);
    char line[MAXLINE];
    const int n = read(STDIN_FILENO, line, MAXLINE);
    if (n < 0) {
        err_sys("read error");
    }
    alarm(0);

    write(STDOUT_FILENO, line, n);
    exit(0);
}

static void sig_alrm([[maybe_unused]] int signo) {
    /* nothing to do, just return to interrupt the read */
}
