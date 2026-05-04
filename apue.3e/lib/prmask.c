#include <stdio.h>     /* for printf */
#include <errno.h>     /* for errno */
#include <signal.h>    /* for sigprocmask, sigismember, sigset_t, SIGINT, SIGQUIT, SIGUSR1, SIGALRM */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

void pr_mask(const char *str) {
    sigset_t sigset;
    int errno_save;

    errno_save = errno; /* we can be called by signal handlers */
    if (sigprocmask(0, NULL, &sigset) < 0) {
        err_ret("sigprocmask error");
    } else {
        printf("%s", str);
        if (sigismember(&sigset, SIGINT)) {
            printf(" SIGINT");
        }
        if (sigismember(&sigset, SIGQUIT)) {
            printf(" SIGQUIT");
        }
        if (sigismember(&sigset, SIGUSR1)) {
            printf(" SIGUSR1");
        }
        if (sigismember(&sigset, SIGALRM)) {
            printf(" SIGALRM");
        }

        /* remaining signals can go here  */

        printf("\n");
    }

    errno = errno_save; /* restore errno */
}
