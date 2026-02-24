#include <stdio.h> /* for getc and putc */
#include <sys/types.h> /* for uid_t */
#include <stdlib.h> /* for exit */

int main(void) {
    int c;

    while ((c = getc(stdin)) != EOF) {
        if (putc(c, stdout) == EOF) {
            err_sys("output error");
        }
    }

    if (ferror(stdin)) {
        err_sys("input error");
    }

    exit(0);
}
