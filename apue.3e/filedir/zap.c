#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for close */
#include <fcntl.h>     /* for open, O_RDWR, O_TRUNC */
#include <time.h>      /* for struct timespec */
#include <sys/stat.h>  /* for struct stat, stat, futimens */

#include "apue.h"

int main(int argc, char *argv[]) {
    int i, fd;
    struct stat statbuf;
    struct timespec times[2];

    for (i = 1; i < argc; i++) {
        if (stat(argv[i], &statbuf) < 0) { /* fetch current times */
            err_ret("%s: stat error", argv[i]);
            continue;
        }
        if ((fd = open(argv[i], O_RDWR | O_TRUNC)) < 0) { /* truncate */
            err_ret("%s: open error", argv[i]);
            continue;
        }
        times[0] = statbuf.st_atim;
        times[1] = statbuf.st_mtim;
        if (futimens(fd, times) < 0) { /* reset times */
            err_ret("%s: futimens error", argv[i]);
        }
        close(fd);
    }
    exit(0);
}
