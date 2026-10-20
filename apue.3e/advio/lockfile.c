#include <unistd.h> /* for SEEK_SET */
#include <fcntl.h>  /* for fcntl, struct flock, F_WRLCK, F_SETLK */

int lockfile(int fd) {
    struct flock fl = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    return (fcntl(fd, F_SETLK, &fl));
}
