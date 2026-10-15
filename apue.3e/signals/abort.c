#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void abort(void) /* POSIX-style abort() function */
{
    /* Caller can't ignore SIGABRT, if so reset to default */
    struct sigaction action;
    sigaction(SIGABRT, nullptr, &action);
    if (action.sa_handler == SIG_IGN) {
        action.sa_handler = SIG_DFL;
        sigaction(SIGABRT, &action, nullptr);
    }
    if (action.sa_handler == SIG_DFL) {
        fflush(nullptr); /* flush all open stdio streams */
    }

    /* Caller can't block SIGABRT; make sure it's unblocked */
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGABRT); /* mask has only SIGABRT turned off */
    sigprocmask(SIG_SETMASK, &mask, nullptr);
    kill(getpid(), SIGABRT); /* send the signal */

    /* If we're here, process caught SIGABRT and returned */
    fflush(nullptr); /* flush all open stdio streams */
    action.sa_handler = SIG_DFL;
    sigaction(SIGABRT, &action, nullptr);     /* reset to default */
    sigprocmask(SIG_SETMASK, &mask, nullptr); /* just in case ... */
    kill(getpid(), SIGABRT);                  /* and one more time */
    exit(1); /* this should never be executed ... */
}
