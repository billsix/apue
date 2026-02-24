#include <stdio.h> /* for perror, fprintf */
#include <unistd.h> /* for exit */
#include <sys/types.h> /* needed for apue.h */
#include "apue.h"
#include <errno.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "EACCES: %s\n", strerror(EACCES));
    errno = ENOENT;
    perror(argv[0]);
    exit(0);
}
