#include <stdio.h>  /* for printf, fprintf, stderr, EOF */
#include <stdlib.h> /* for exit, atexit */
#include <unistd.h> /* for isatty, getopt, execvp, optarg, optind, opterr, optopt, STDIN_FILENO */
#include <termios.h> /* for struct termios, tcgetattr, tcsetattr, TCSANOW, ECHO, ECHOE, ECHOK, ECHONL, ONLCR */
#include <sys/types.h> /* for pid_t */
#include <sys/ioctl.h> /* for ioctl, TIOCGWINSZ, struct winsize */

#include "apue.h"

#ifdef LINUX
#define OPTSTR "+d:einv"
#else
#define OPTSTR "d:einv"
#endif

static void set_noecho(int);  /* at the end of this file */
void do_driver(const char *); /* in the file driver.c */
void loop(int, bool);         /* in the file loop.c */

int main(int argc, char *argv[]) {
    int fdm;
    bool interactive = isatty(STDIN_FILENO);
    bool ignoreeof = false;
    bool noecho = false;
    bool verbose = false;
    pid_t pid;
    char *driver = nullptr;
    char slave_name[20];

    opterr = 0; /* don't want getopt() writing to stderr */
    for (;;) {
        int c = getopt(argc, argv, OPTSTR);
        if (c == EOF) {
            break;
        }
        switch (c) {
            case 'd': /* driver for stdin/stdout */
                driver = optarg;
                break;

            case 'e': /* noecho for slave pty's line discipline */
                noecho = true;
                break;

            case 'i': /* ignore EOF on standard input */
                ignoreeof = true;
                break;

            case 'n': /* not interactive */
                interactive = false;
                break;

            case 'v': /* verbose */
                verbose = true;
                break;

            case '?':
                err_quit("unrecognized option: -%c", optopt);
        }
    }
    if (optind >= argc) {
        err_quit("usage: pty [ -d driver -einv ] program [ arg ... ]");
    }

    if (interactive) { /* fetch current termios and window size */
        struct termios orig_termios;
        struct winsize size;
        if (tcgetattr(STDIN_FILENO, &orig_termios) < 0) {
            err_sys("tcgetattr error on stdin");
        }
        if (ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&size) < 0) {
            err_sys("TIOCGWINSZ error");
        }
        pid = pty_fork(&fdm, slave_name, sizeof(slave_name), &orig_termios,
                       &size);
    } else {
        pid = pty_fork(&fdm, slave_name, sizeof(slave_name), nullptr, nullptr);
    }

    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* child */
        if (noecho) {
            set_noecho(STDIN_FILENO); /* stdin is slave pty */
        }

        if (execvp(argv[optind], &argv[optind]) < 0) {
            err_sys("can't execute: %s", argv[optind]);
        }
    }

    if (verbose) {
        fprintf(stderr, "slave name = %s\n", slave_name);
        if (driver != nullptr) {
            fprintf(stderr, "driver = %s\n", driver);
        }
    }

    if (interactive && driver == nullptr) {
        if (tty_raw(STDIN_FILENO) < 0) { /* user's tty to raw mode */
            err_sys("tty_raw error");
        }
        if (atexit(tty_atexit) < 0) { /* reset user's tty on exit */
            err_sys("atexit error");
        }
    }

    if (driver) {
        do_driver(driver); /* changes our stdin/stdout */
    }

    loop(fdm, ignoreeof); /* copies stdin -> ptym, ptym -> stdout */

    exit(0);
}

static void set_noecho(int fd) /* turn off echo (for slave pty) */
{
    struct termios stermios;

    if (tcgetattr(fd, &stermios) < 0) {
        err_sys("tcgetattr error");
    }

    stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

    /*
     * Also turn off NL to CR/NL mapping on output.
     */
    stermios.c_oflag &= ~(ONLCR);

    if (tcsetattr(fd, TCSANOW, &stermios) < 0) {
        err_sys("tcsetattr error");
    }
}
