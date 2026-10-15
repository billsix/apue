#include <stdlib.h> /* for exit */
#include <signal.h> /* for signal, sigemptyset, sigaddset, sigprocmask, sigsuspend, sigset_t, SIG_ERR, SIG_BLOCK, SIG_SETMASK, SIGINT, SIGUSR1 */

#include "apue.h"

static void sig_int(int);

int main(void) {
    pr_mask("program start: ");

    if (signal(SIGINT, sig_int) == SIG_ERR) {
        err_sys("signal(SIGINT) error");
    }
    sigset_t waitmask;
    sigemptyset(&waitmask);
    sigaddset(&waitmask, SIGUSR1);
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);

    /*
     * Block SIGINT and save current signal mask.
     */
    sigset_t oldmask;
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        err_sys("SIG_BLOCK error");
    }

    /*
     * Critical region of code.
     */
    pr_mask("in critical region: ");

    /*
     * Pause, allowing all signals except SIGUSR1.
     */
    if (sigsuspend(&waitmask) != -1) {
        err_sys("sigsuspend error");
    }

    pr_mask("after return from sigsuspend: ");

    /*
     * Reset signal mask which unblocks SIGINT.
     */
    if (sigprocmask(SIG_SETMASK, &oldmask, nullptr) < 0) {
        err_sys("SIG_SETMASK error");
    }

    /*
     * And continue processing ...
     */
    pr_mask("program exit: ");

    exit(0);
}

static void sig_int([[maybe_unused]] int signo) { pr_mask("\nin sig_int: "); }
