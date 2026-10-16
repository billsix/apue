#include <stdio.h> /* for FILE, fopen, popen, pclose, fgets, fputs, ferror, EOF */
#include <stdlib.h> /* for exit */

#include "apue.h"

#define PAGER "${PAGER:-more}" /* environment variable, or default */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        err_quit("usage: a.out <pathname>");
    }
    FILE *fpin = fopen(argv[1], "r");
    if (fpin == nullptr) {
        err_sys("can't open %s", argv[1]);
    }

    FILE *fpout = popen(PAGER, "w");
    if (fpout == nullptr) {
        err_sys("popen error");
    }

    /* copy argv[1] to pager */
    char line[MAXLINE];
    while (fgets(line, MAXLINE, fpin) != nullptr) {
        if (fputs(line, fpout) == EOF) {
            err_sys("fputs error to pipe");
        }
    }
    if (ferror(fpin)) {
        err_sys("fgets error");
    }
    if (pclose(fpout) == -1) {
        err_sys("pclose error");
    }

    exit(0);
}
