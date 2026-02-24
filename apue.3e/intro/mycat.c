#include <unistd.h> /* for read, write */
#include <stdlib.h> /* for exit */
#include <sys/types.h> /* needed for apue.h */
#include "apue.h" /* for err_sys */

#define BUFFSIZE 4096

int main(void) {
    int n;
    char buf[BUFFSIZE];

    while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            err_sys("write error");
        }
    }

    if (n < 0) {
        err_sys("read error");
    }

    exit(0);
}
