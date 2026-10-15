#include <stdio.h>     /* for setbuf, putc, stdout */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for fork */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

static void charatatime(const char *);

int main(void) {
    TELL_WAIT();

    const pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        WAIT_PARENT(); /* parent goes first */
        charatatime("output from child\n");
    } else {
        charatatime("output from parent\n");
        TELL_CHILD(pid);
    }
    exit(0);
}

static void charatatime(const char *str) {
    int c;

    setbuf(stdout, nullptr); /* set unbuffered */
    for (const char *ptr = str; (c = *ptr++) != 0;) {
        putc(c, stdout);
    }
}
