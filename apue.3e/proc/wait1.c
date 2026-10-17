#include <stdlib.h>    /* for exit, abort */
#include <unistd.h>    /* for fork */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h>  /* for wait */

#include "apue.h"

int main(void) {
    int status;

    pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* child */
        exit(7);
    }

    if (wait(&status) != pid) { /* wait for child */
        err_sys("wait error");
    }
    pr_exit(status); /* and print its status */

    pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* child */
        abort();           /* generates SIGABRT */
    }

    if (wait(&status) != pid) { /* wait for child */
        err_sys("wait error");
    }
    pr_exit(status); /* and print its status */

    pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* child */
        /* divide by 0 generates SIGFPE; a volatile divisor keeps the fault at
         * runtime (the point of the demo) without compile-time UB. */
        volatile int zero = 0;
        status /= zero;
    }

    if (wait(&status) != pid) { /* wait for child */
        err_sys("wait error");
    }
    pr_exit(status); /* and print its status */

    exit(0);
}
