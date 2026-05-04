#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for write, lseek */
#include <fcntl.h>     /* for creat */
#include <sys/types.h> /* needed for apue.h */
#include <sys/stat.h>  /* for S_I* constants used by FILE_MODE */

#include "apue.h"

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";

int main(void) {
    int fd;

    if ((fd = creat("file.hole", FILE_MODE)) < 0) {
        err_sys("creat error");
    }

    if (write(fd, buf1, 10) != 10) {
        err_sys("buf1 write error");
    }
    /* offset now = 10 */

    if (lseek(fd, 16384, SEEK_SET) == -1) {
        err_sys("lseek error");
    }
    /* offset now = 16384 */

    if (write(fd, buf2, 10) != 10) {
        err_sys("buf2 write error");
    }
    /* offset now = 16394 */

    exit(0);
}
