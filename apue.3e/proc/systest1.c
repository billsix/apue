#include <stdlib.h> /* for exit, system */

#include "apue.h"

int main(void) {
    int status = system("date");
    if (status < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    status = system("nosuchcommand");
    if (status < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    status = system("who; exit 44");
    if (status < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    exit(0);
}
