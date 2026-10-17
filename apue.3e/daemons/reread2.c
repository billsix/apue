#include <stdlib.h> /* for exit */
#include <string.h> /* for strrchr, strerror */
#include <errno.h>  /* for errno */
#include <signal.h> /* for sigaction, sigemptyset, sigaddset, SIGTERM, SIGHUP */
#include <syslog.h> /* for syslog, LOG_INFO, LOG_ERR */

#include "apue.h"

extern int lockfile(int);
extern int already_running(void);

void reread(void) { /* ... */ }

void sigterm([[maybe_unused]] int signo) {
    syslog(LOG_INFO, "got SIGTERM; exiting");
    exit(0);
}

void sighup([[maybe_unused]] int signo) {
    syslog(LOG_INFO, "Re-reading configuration file");
    reread();
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
     * Handle signals of interest.
     */
    struct sigaction sa;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGHUP);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, nullptr) < 0) {
        syslog(LOG_ERR, "can't catch SIGTERM: %s", strerror(errno));
        exit(1);
    }
    sa.sa_handler = sighup;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, nullptr) < 0) {
        syslog(LOG_ERR, "can't catch SIGHUP: %s", strerror(errno));
        exit(1);
    }

    /*
     * Proceed with the rest of the daemon.
     */
    /* ... */
    exit(0);
}
