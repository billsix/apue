#include <stdlib.h> /* for exit */
#include <string.h> /* for strrchr */
#include <signal.h> /* for sigaction, sigemptyset, sigfillset, sigwait, sigset_t, SIG_DFL, SIGHUP, SIGTERM */
#include <syslog.h> /* for syslog, LOG_ERR, LOG_INFO */
#include <pthread.h> /* for pthread_t, pthread_create, pthread_sigmask, SIG_BLOCK */

#include "apue.h"

sigset_t mask;

extern int already_running(void);

void reread(void) { /* ... */ }

void *thr_fn([[maybe_unused]] void *arg) {
    for (;;) {
        int signo;
        const int err = sigwait(&mask, &signo);
        if (err != 0) {
            syslog(LOG_ERR, "sigwait failed");
            exit(1);
        }

        switch (signo) {
            case SIGHUP:
                syslog(LOG_INFO, "Re-reading configuration file");
                reread();
                break;

            case SIGTERM:
                syslog(LOG_INFO, "got SIGTERM; exiting");
                exit(0);

            default:
                syslog(LOG_INFO, "unexpected signal %d\n", signo);
        }
    }
    return (nullptr);
}

int main([[maybe_unused]] int argc, char *argv[]) {
    const char *cmd = strrchr(argv[0], '/');
    if (cmd == nullptr) {
        cmd = argv[0];
    } else {
        cmd++;
    }

    /*
     * Become a daemon.
     */
    daemonize(cmd);

    /*
     * Make sure only one copy of the daemon is running.
     */
    if (already_running()) {
        syslog(LOG_ERR, "daemon already running");
        exit(1);
    }

    /*
     * Restore SIGHUP default and block all signals.
     */
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, nullptr) < 0) {
        err_quit("%s: can't restore SIGHUP default");
    }
    sigfillset(&mask);
    int err = pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    if (err != 0) {
        err_exit(err, "SIG_BLOCK error");
    }

    /*
     * Create a thread to handle SIGHUP and SIGTERM.
     */
    pthread_t tid;
    err = pthread_create(&tid, nullptr, thr_fn, nullptr);
    if (err != 0) {
        err_exit(err, "can't create thread");
    }

    /*
     * Proceed with the rest of the daemon.
     */
    /* ... */
    exit(0);
}
