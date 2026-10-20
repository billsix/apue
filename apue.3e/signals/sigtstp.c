#include <stdlib.h> /* for exit */
#include <unistd.h> /* for read, write, getpid, STDIN_FILENO, STDOUT_FILENO */
#include <signal.h> /* for signal, sigemptyset, sigaddset, sigprocmask, kill, sigset_t, SIG_DFL, SIG_IGN, SIG_UNBLOCK, SIGTSTP */

#include "apue.h"

#define BUFFSIZE 1024

static void sig_tstp(
    [[maybe_unused]] int signo) /* signal handler for SIGTSTP */
{
    /* ... move cursor to lower left corner, reset tty mode ... */

    /*
     * Unblock SIGTSTP, since it's blocked while we're handling it.
     */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &mask, nullptr);

    signal(SIGTSTP, SIG_DFL); /* reset disposition to default */

    kill(getpid(), SIGTSTP); /* and send the signal to ourself */

    /* we won't return from the kill until we're continued */

    signal(SIGTSTP, sig_tstp); /* reestablish signal handler */

    /* ... reset tty mode, redraw screen ... */
}

int main(void) {
    int n;

    /*
     * Only catch SIGTSTP if we're running with a job-control shell.
     */
    if (signal(SIGTSTP, SIG_IGN) == SIG_DFL) {
        signal(SIGTSTP, sig_tstp);
    }

    char buf[BUFFSIZE];
    while (true) {
        n = read(STDIN_FILENO, buf, BUFFSIZE);
        if (n <= 0) {
            break;
        }
        if (write(STDOUT_FILENO, buf, n) != n) {
            err_sys("write error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }

    exit(0);
}
