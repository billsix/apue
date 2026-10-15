#include <stdio.h>  /* for printf, fflush, stdout */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for getpid, getppid, getpgrp, tcgetpgrp, fork, sleep, read, STDIN_FILENO */
#include <errno.h>     /* for errno */
#include <signal.h>    /* for signal, kill, SIGHUP, SIGTSTP */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

static void sig_hup([[maybe_unused]] int signo) {
    printf("SIGHUP received, pid = %ld\n", (long)getpid());
}

static void pr_ids(const char *name) {
    printf("%s: pid = %ld, ppid = %ld, pgrp = %ld, tpgrp = %ld\n", name,
           (long)getpid(), (long)getppid(), (long)getpgrp(),
           (long)tcgetpgrp(STDIN_FILENO));
    fflush(stdout);
}

int main(void) {
    pr_ids("parent");
    const pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid > 0) { /* parent */
        sleep(5);         /* sleep to let child stop itself */
    } else {              /* child */
        char c;
        pr_ids("child");
        signal(SIGHUP, sig_hup); /* establish signal handler */
        kill(getpid(), SIGTSTP); /* stop ourself */
        pr_ids("child");         /* prints only if we're continued */
        if (read(STDIN_FILENO, &c, 1) != 1) {
            printf("read error %d on controlling TTY\n", errno);
        }
    }
    exit(0);
}
