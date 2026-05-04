#include <stdio.h>     /* for setbuf, putc, stdout */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for fork */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

static void charatatime(char *);

int main(void) {
    pid_t pid;

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        charatatime("output from child\n");
    } else {
        charatatime("output from parent\n");
    }
    exit(0);
}

static void charatatime(char *str) {
    char *ptr;
    int c;

    setbuf(stdout, NULL); /* set unbuffered */
    for (ptr = str; (c = *ptr++) != 0;) {
        putc(c, stdout);
    }
}
