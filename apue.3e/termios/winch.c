#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for isatty, pause, STDIN_FILENO */
#include <signal.h>    /* for signal, SIGWINCH, SIG_ERR */
#include <termios.h>   /* for TIOCGWINSZ on platforms that define it here */
#ifndef TIOCGWINSZ
#include <sys/ioctl.h> /* for ioctl, TIOCGWINSZ, struct winsize */
#endif

#include "apue.h"

static void pr_winsize(int fd) {
    struct winsize size;

    if (ioctl(fd, TIOCGWINSZ, (char *)&size) < 0) {
        err_sys("TIOCGWINSZ error");
    }
    printf("%d rows, %d columns\n", size.ws_row, size.ws_col);
}

static void sig_winch(int signo) {
    printf("SIGWINCH received\n");
    pr_winsize(STDIN_FILENO);
}

int main(void) {
    if (isatty(STDIN_FILENO) == 0) {
        exit(1);
    }
    if (signal(SIGWINCH, sig_winch) == SIG_ERR) {
        err_sys("signal error");
    }
    pr_winsize(STDIN_FILENO); /* print initial size */
    for (;;) {                /* and sleep forever */
        pause();
    }
}
