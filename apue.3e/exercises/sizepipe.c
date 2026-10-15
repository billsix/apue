#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for pipe, write */
#include <fcntl.h>  /* for O_NONBLOCK */

#include "apue.h"

int main(void) {
    int fd[2];

    if (pipe(fd) < 0) {
        err_sys("pipe error");
    }
    set_fl(fd[1], O_NONBLOCK);

    /* write 1 byte at a time until pipe is full */
    int n;
    for (n = 0;; n++) {
        const int i = write(fd[1], "a", 1);
        if (i != 1) {
            printf("write ret %d, ", i);
            break;
        }
    }
    printf("pipe capacity = %d\n", n);
    exit(0);
}
