#include <stdio.h>     /* for perror, fprintf, stderr */
#include <stdlib.h>    /* for exit */
#include <string.h>    /* for strerror */
#include <errno.h>     /* for errno, EACCES, ENOENT */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

int main(int argc, char *argv[]) {
    fprintf(stderr, "EACCES: %s\n", strerror(EACCES));
    errno = ENOENT;
    perror(argv[0]);
    exit(0);
}
