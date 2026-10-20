#include <stdio.h>  /* for getc, putc, ferror, stdin, stdout, EOF */
#include <stdlib.h> /* for exit */

#include "apue.h"

int main(void) {
    while (true) {
        int c = getc(stdin);
        if (c == EOF) {
            break;
        }
        if (putc(c, stdout) == EOF) {
            err_sys("output error");
        }
    }

    if (ferror(stdin)) {
        err_sys("input error");
    }

    exit(0);
}
