#include <unistd.h> /* for alarm */
#include <signal.h> /* for sigaction, sigprocmask, sigemptyset, sigaddset, sigdelset, sigsuspend, sigset_t, struct sigaction, SIGALRM, SIG_BLOCK, SIG_SETMASK */

static void sig_alrm(int signo) {
    /* nothing to do, just returning wakes up sigsuspend() */
}

unsigned int sleep(unsigned int seconds) {
    /* set our handler, save previous information */
    struct sigaction newact = {.sa_handler = sig_alrm, .sa_flags = 0};
    sigemptyset(&newact.sa_mask);
    struct sigaction oldact;
    sigaction(SIGALRM, &newact, &oldact);

    /* block SIGALRM and save current signal mask */
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGALRM);
    sigset_t oldmask;
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    alarm(seconds);
    sigset_t suspmask = oldmask;

    /* make sure SIGALRM isn't blocked */
    sigdelset(&suspmask, SIGALRM);

    /* wait for any signal to be caught */
    sigsuspend(&suspmask);

    /* some signal has been caught, SIGALRM is now blocked */

    const unsigned int unslept = alarm(0);

    /* reset previous action */
    sigaction(SIGALRM, &oldact, nullptr);

    /* reset signal mask, which unblocks SIGALRM */
    sigprocmask(SIG_SETMASK, &oldmask, nullptr);
    return (unslept);
}
