#include <stdlib.h>    /* for exit, abort */
#include <unistd.h>    /* for fork, sleep, execl, getpid */
#include <signal.h>    /* for kill, SIGKILL */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        err_sys("fork error");
    } else if (pid != 0) { /* parent */
        sleep(2);
        exit(2); /* terminate with exit status 2 */
    }

    pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid != 0) { /* first child */
        sleep(4);
        abort(); /* terminate with core dump */
    }

    pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid != 0) { /* second child */
        execl("/bin/dd", "dd", "if=/etc/passwd", "of=/dev/null", (char *)0);
        exit(7); /* shouldn't get here */
    }

    pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid != 0) { /* third child */
        sleep(8);
        exit(0); /* normal exit */
    }

    sleep(6);                /* fourth child */
    kill(getpid(), SIGKILL); /* terminate w/signal, no core dump */
    exit(6);                 /* shouldn't get here */
}
