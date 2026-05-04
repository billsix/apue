#include <stdlib.h>    /* for exit */
#include <fcntl.h>     /* for creat */
#include <sys/stat.h>  /* for umask, S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH, S_IWOTH */

#include "apue.h"

#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int main(void) {
    umask(0);
    if (creat("foo", RWRWRW) < 0) {
        err_sys("creat error for foo");
    }
    umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (creat("bar", RWRWRW) < 0) {
        err_sys("creat error for bar");
    }
    exit(0);
}
