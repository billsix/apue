#include <stdlib.h> /* for exit */
#include <unistd.h> /* for read, write, alarm, STDIN_FILENO, STDOUT_FILENO */
#include <signal.h> /* for signal, SIG_ERR, SIGALRM */
#include <setjmp.h> /* for jmp_buf, setjmp, longjmp */

#include "apue.h"

static void sig_alrm(int);
static jmp_buf env_alrm;

int main(void) {
    int n;
    char line[MAXLINE];

    if (signal(SIGALRM, sig_alrm) == SIG_ERR) {
        err_sys("signal(SIGALRM) error");
    }
    if (setjmp(env_alrm) != 0) {
        err_quit("read timeout");
    }

    alarm(10);
    /* n stays declared above: this function uses setjmp/longjmp, so its locals
     * are left in place (c-modernization-sweep.md hazard). Split the assignment
     * out of the test without moving the declaration. */
    n = read(STDIN_FILENO, line, MAXLINE);
    if (n < 0) {
        err_sys("read error");
    }
    alarm(0);

    write(STDOUT_FILENO, line, n);
    exit(0);
}

static void sig_alrm([[maybe_unused]] int signo) { longjmp(env_alrm, 1); }
