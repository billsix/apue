#include <stdlib.h>    /* for exit, system */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

int main(void) {
    int status;

    if ((status = system("date")) < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    if ((status = system("nosuchcommand")) < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    if ((status = system("who; exit 44")) < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    exit(0);
}
