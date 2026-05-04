#include <stdlib.h>    /* for exit, system */
#include <unistd.h>    /* for fork, sleep */
#include <sys/types.h> /* for pid_t, needed for apue.h */

#include "apue.h"

#ifdef SOLARIS
#define PSCMD "ps -a -o pid,ppid,s,tty,comm"
#else
#define PSCMD "ps -o pid,ppid,state,tty,command"
#endif

int main(void) {
    pid_t pid;

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* child */
        exit(0);
    }

    /* parent */
    sleep(4);
    system(PSCMD);

    exit(0);
}
