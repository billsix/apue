#include <stdlib.h> /* for exit */
#include <unistd.h> /* for alarm, pause */
#include <signal.h> /* for signal, SIG_ERR, sig_atomic_t, SIGUSR1, SIGALRM */
#include <setjmp.h> /* for sigjmp_buf, sigsetjmp, siglongjmp */
#include <time.h>   /* for time, time_t */

#include "apue.h"

static void sig_usr1(int);
static void sig_alrm(int);
static sigjmp_buf jmpbuf;
static volatile sig_atomic_t canjump;

int main(void) {
    if (signal(SIGUSR1, sig_usr1) == SIG_ERR) {
        err_sys("signal(SIGUSR1) error");
    }
    if (signal(SIGALRM, sig_alrm) == SIG_ERR) {
        err_sys("signal(SIGALRM) error");
    }

    pr_mask("starting main: "); /* {Prog prmask} */

    if (sigsetjmp(jmpbuf, 1)) {
        pr_mask("ending main: ");

        exit(0);
    }
    canjump = 1; /* now sigsetjmp() is OK */

    while (true) {
        pause();
    }
}

static void sig_usr1([[maybe_unused]] int signo) {
    time_t starttime;

    if (canjump == 0) {
        return; /* unexpected signal, ignore */
    }

    pr_mask("starting sig_usr1: ");

    alarm(3); /* SIGALRM in 3 seconds */
    starttime = time(nullptr);
    while (true) { /* busy wait for 5 seconds */
        if (time(nullptr) > starttime + 5) {
            break;
        }
    }

    pr_mask("finishing sig_usr1: ");

    canjump = 0;
    siglongjmp(jmpbuf, 1); /* jump back to main, don't return */
}

static void sig_alrm([[maybe_unused]] int signo) { pr_mask("in sig_alrm: "); }
