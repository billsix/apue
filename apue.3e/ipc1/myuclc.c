#include <stdio.h>     /* for getchar, putchar, fflush, stdout, EOF */
#include <stdlib.h>    /* for exit */
#include <ctype.h>     /* for isupper, tolower */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

int main(void) {
    int c;

    while ((c = getchar()) != EOF) {
        if (isupper(c)) {
            c = tolower(c);
        }
        if (putchar(c) == EOF) {
            err_sys("output error");
        }
        if (c == '\n') {
            fflush(stdout);
        }
    }
    exit(0);
}
