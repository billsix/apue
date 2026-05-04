#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for unlink */
#include <fcntl.h>     /* for open, O_RDONLY, O_WRONLY, O_NONBLOCK */
#include <sys/types.h> /* needed for apue.h */
#include <sys/stat.h>  /* for mkfifo, FILE_MODE constants */

#include "apue.h"

#define FIFO "temp.fifo"

int main(void) {
    int fdread, fdwrite;

    unlink(FIFO);
    if (mkfifo(FIFO, FILE_MODE) < 0) {
        err_sys("mkfifo error");
    }
    if ((fdread = open(FIFO, O_RDONLY | O_NONBLOCK)) < 0) {
        err_sys("open error for reading");
    }
    if ((fdwrite = open(FIFO, O_WRONLY)) < 0) {
        err_sys("open error for writing");
    }
    clr_fl(fdread, O_NONBLOCK);
    exit(0);
}
