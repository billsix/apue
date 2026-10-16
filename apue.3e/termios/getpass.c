#include <stdio.h> /* for FILE, fopen, fclose, fputs, fileno, getc, putc, setbuf, EOF */
#include <signal.h> /* for sigset_t, sigemptyset, sigaddset, sigprocmask, SIGINT, SIGTSTP, SIG_BLOCK, SIG_SETMASK */
#include <termios.h> /* for struct termios, tcgetattr, tcsetattr, TCSAFLUSH, ECHO, ECHOE, ECHOK, ECHONL */

#define MAX_PASS_LEN 8 /* max #chars for user to enter */

char *getpass(const char *prompt) {
    static char buf[MAX_PASS_LEN + 1]; /* null byte at end */

    FILE *fp = fopen(ctermid(nullptr), "r+");
    if (fp == nullptr) {
        return (nullptr);
    }
    setbuf(fp, nullptr);

    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGINT);  /* block SIGINT */
    sigaddset(&sig, SIGTSTP); /* block SIGTSTP */
    sigset_t osig;
    sigprocmask(SIG_BLOCK, &sig, &osig); /* and save mask */

    struct termios ts;
    tcgetattr(fileno(fp), &ts); /* save tty state */
    struct termios ots = ts;    /* structure copy */
    ts.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    tcsetattr(fileno(fp), TCSAFLUSH, &ts);
    fputs(prompt, fp);

    char *ptr = buf;
    int c;
    while ((c = getc(fp)) != EOF && c != '\n') {
        if (ptr < &buf[MAX_PASS_LEN]) {
            *ptr++ = c;
        }
    }
    *ptr = 0;       /* null terminate */
    putc('\n', fp); /* we echo a newline */

    tcsetattr(fileno(fp), TCSAFLUSH, &ots);   /* restore TTY state */
    sigprocmask(SIG_SETMASK, &osig, nullptr); /* restore mask */
    fclose(fp);                               /* done with /dev/tty */
    return (buf);
}
