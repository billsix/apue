#include <signal.h> /* for sigaction, sigemptyset, struct sigaction, SIG_ERR, SIGALRM, SA_RESTART */

#include "apue.h"

/* Reliable version of signal(), using POSIX sigaction().  */
Sigfunc *signal(int signo, Sigfunc *func) {
    struct sigaction act = {.sa_handler = func, .sa_flags = 0};
    sigemptyset(&act.sa_mask);
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
        act.sa_flags |= SA_RESTART;
    }
    struct sigaction oact;
    if (sigaction(signo, &act, &oact) < 0) {
        return (SIG_ERR);
    }
    return (oact.sa_handler);
}
