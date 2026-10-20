#include <signal.h> /* for sigaction, sigemptyset, struct sigaction, SIG_ERR */

#include "apue.h"

Sigfunc *signal_intr(int signo, Sigfunc *func) {
    struct sigaction act = {.sa_handler = func, .sa_flags = 0};
    sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
    struct sigaction oact;
    if (sigaction(signo, &act, &oact) < 0) {
        return (SIG_ERR);
    }
    return (oact.sa_handler);
}
