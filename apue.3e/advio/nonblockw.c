#include <stddef.h>    /* for size_t */
#include <sys/types.h> /* for ssize_t */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for read */
#include <stdio.h>     /* for fprintf */
#include <errno.h>
#include <fcntl.h>

#include "apue.h"

char buf[500000];

int main(void) {
    int ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
    fprintf(stderr, "read %d bytes\n", ntowrite);

    set_fl(STDOUT_FILENO, O_NONBLOCK); /* set nonblocking */

    char *ptr = buf;
    while (ntowrite > 0) {
        errno = 0;
        int nwrite = write(STDOUT_FILENO, ptr, ntowrite);
        fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);

        if (nwrite > 0) {
            ptr += nwrite;
            ntowrite -= nwrite;
        }
    }

    clr_fl(STDOUT_FILENO, O_NONBLOCK); /* clear nonblocking */

    exit(0);
}
