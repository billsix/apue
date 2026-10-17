#include <stdio.h> /* for FILE, popen, pclose, fputs, fflush, fgets, putchar, stdout, EOF */
#include <stdlib.h> /* for exit */

#include "apue.h"

int main(void) {
    FILE *const fpin = popen("myuclc", "r");
    if (fpin == nullptr) {
        err_sys("popen error");
    }
    char line[MAXLINE];
    for (;;) {
        fputs("prompt> ", stdout);
        fflush(stdout);
        if (fgets(line, MAXLINE, fpin) == nullptr) { /* read from pipe */
            break;
        }
        if (fputs(line, stdout) == EOF) {
            err_sys("fputs error to pipe");
        }
    }
    if (pclose(fpin) == -1) {
        err_sys("pclose error");
    }
    putchar('\n');
    exit(0);
}
