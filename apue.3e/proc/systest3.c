#include <stdlib.h> /* for exit, system */

#include "apue.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        err_quit("command-line argument required");
    }

    const int status = system(argv[1]);
    if (status < 0) {
        err_sys("system() error");
    }

    pr_exit(status);

    exit(0);
}
