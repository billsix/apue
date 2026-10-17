#include <stdlib.h> /* for exit */
#include <unistd.h> /* for read, write, STDIN_FILENO, STDOUT_FILENO */

#include "apue.h"

#define BUFFSIZE 4096

int main(void) {
    int n;
    char buf[BUFFSIZE];

    for (;;) {
        n = read(STDIN_FILENO, buf, BUFFSIZE);
        if (n <= 0) {
            break;
        }
        if (write(STDOUT_FILENO, buf, n) != n) {
            err_sys("write error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }

    exit(0);
}
