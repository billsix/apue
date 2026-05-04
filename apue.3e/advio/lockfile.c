#include <unistd.h> /* for SEEK_SET */
#include <fcntl.h>  /* for fcntl, struct flock, F_WRLCK, F_SETLK */

int lockfile(int fd) {
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}
