#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for unlink, sleep */
#include <fcntl.h>     /* for open, O_RDWR */

#include "apue.h"

int main(void) {
    if (open("tempfile", O_RDWR) < 0) {
        err_sys("open error");
    }
    if (unlink("tempfile") < 0) {
        err_sys("unlink error");
    }
    printf("file unlinked\n");
    sleep(15);
    printf("done\n");
    exit(0);
}
