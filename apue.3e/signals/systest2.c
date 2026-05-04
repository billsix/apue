#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit, system */
#include <signal.h>    /* for signal, SIG_ERR, SIGINT, SIGCHLD */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

static void sig_int(int signo) { printf("caught SIGINT\n"); }

static void sig_chld(int signo) { printf("caught SIGCHLD\n"); }

int main(void) {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        err_sys("signal(SIGINT) error");
    }
    if (signal(SIGCHLD, sig_chld) == SIG_ERR) {
        err_sys("signal(SIGCHLD) error");
    }
    if (system("/bin/ed") < 0) {
        err_sys("system() error");
    }
    exit(0);
}
