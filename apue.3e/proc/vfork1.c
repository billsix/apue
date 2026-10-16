#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for vfork, _exit, getpid */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

int globvar = 6; /* external variable in initialized data */

int main(void) {
    int var = 88; /* automatic variable on the stack */

    printf("before vfork\n"); /* we don't flush stdio */
    const pid_t pid = vfork();
    if (pid < 0) {
        err_sys("vfork error");
    } else if (pid == 0) { /* child */
        globvar++;         /* modify parent's variables */
        var++;
        _exit(0); /* child terminates */
    }

    /* parent continues here */
    printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);
    exit(0);
}
