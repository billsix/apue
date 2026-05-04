#include <fcntl.h>     /* for fcntl, F_GETFD, F_SETFD, FD_CLOEXEC */

#include "apue.h"

int set_cloexec(int fd) {
    int val;

    if ((val = fcntl(fd, F_GETFD, 0)) < 0) {
        return (-1);
    }

    val |= FD_CLOEXEC; /* enable close-on-exec */

    return (fcntl(fd, F_SETFD, val));
}
