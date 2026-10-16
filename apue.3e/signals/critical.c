#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for sleep */
#include <signal.h> /* for signal, sigemptyset, sigaddset, sigprocmask, sigpending, sigismember, sigset_t, SIG_ERR, SIG_DFL, SIG_BLOCK, SIG_SETMASK, SIGQUIT */

#include "apue.h"

static void sig_quit(int);

int main(void) {
    if (signal(SIGQUIT, sig_quit) == SIG_ERR) {
        err_sys("can't catch SIGQUIT");
    }

    /*
     * Block SIGQUIT and save current signal mask.
     */
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGQUIT);
    sigset_t oldmask;
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        err_sys("SIG_BLOCK error");
    }

    sleep(5); /* SIGQUIT here will remain pending */

    sigset_t pendmask;
    if (sigpending(&pendmask) < 0) {
        err_sys("sigpending error");
    }
    if (sigismember(&pendmask, SIGQUIT)) {
        printf("\nSIGQUIT pending\n");
    }

    /*
     * Restore signal mask which unblocks SIGQUIT.
     */
    if (sigprocmask(SIG_SETMASK, &oldmask, nullptr) < 0) {
        err_sys("SIG_SETMASK error");
    }
    printf("SIGQUIT unblocked\n");

    sleep(5); /* SIGQUIT here will terminate with core file */
    exit(0);
}

static void sig_quit([[maybe_unused]] int signo) {
    printf("caught SIGQUIT\n");
    if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
        err_sys("can't reset SIGQUIT");
    }
}
