#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for fork, execle, execlp */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h>  /* for waitpid */

#include "apue.h"

char *env_init[] = {"USER=unknown", "PATH=/tmp", nullptr};

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* specify pathname, specify environment */
        if (execle("/home/sar/bin/echoall", "echoall", "myarg1", "MY ARG2",
                   (char *)0, env_init) < 0) {
            err_sys("execle error");
        }
    }

    if (waitpid(pid, nullptr, 0) < 0) {
        err_sys("wait error");
    }

    pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid == 0) { /* specify filename, inherit environment */
        if (execlp("echoall", "echoall", "only 1 arg", (char *)0) < 0) {
            err_sys("execlp error");
        }
    }

    exit(0);
}
