#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <signal.h> /* for signal, sigemptyset, sigaddset, sigprocmask, sigsuspend, sig_atomic_t, sigset_t, SIG_ERR, SIG_BLOCK, SIG_SETMASK, SIGINT, SIGQUIT */

#include "apue.h"

volatile sig_atomic_t quitflag; /* set nonzero by signal handler */

static void sig_int(int signo) /* one signal handler for SIGINT and SIGQUIT */
{
    if (signo == SIGINT) {
        printf("\ninterrupt\n");
    } else if (signo == SIGQUIT) {
        quitflag = 1; /* set flag for main loop */
    }
}

int main(void) {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        err_sys("signal(SIGINT) error");
    }
    if (signal(SIGQUIT, sig_int) == SIG_ERR) {
        err_sys("signal(SIGQUIT) error");
    }

    sigset_t zeromask;
    sigemptyset(&zeromask);
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGQUIT);

    /*
     * Block SIGQUIT and save current signal mask.
     */
    sigset_t oldmask;
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        err_sys("SIG_BLOCK error");
    }

    while (quitflag == 0) {
        sigsuspend(&zeromask);
    }

    /*
     * SIGQUIT has been caught and is now blocked; do whatever.
     */
    quitflag = 0;

    /*
     * Reset signal mask which unblocks SIGQUIT.
     */
    if (sigprocmask(SIG_SETMASK, &oldmask, nullptr) < 0) {
        err_sys("SIG_SETMASK error");
    }

    exit(0);
}
