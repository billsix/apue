#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for fork, execl */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h>  /* for waitpid */

#include "apue.h"

int main(void) {
    const pid_t pid = fork();

    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* child */
        if (execl("/home/sar/bin/testinterp", "testinterp", "myarg1", "MY ARG2",
                  (char *)0) < 0) {
            err_sys("execl error");
        }
    }
    if (waitpid(pid, nullptr, 0) < 0) { /* parent */
        err_sys("waitpid error");
    }
    exit(0);
}
