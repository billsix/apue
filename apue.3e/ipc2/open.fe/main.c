#include <stdio.h>  /* for fgets, stdin */
#include <stdlib.h> /* for exit */
#include <string.h> /* for strlen */
#include <unistd.h> /* for read, write, close, STDOUT_FILENO */
#include <fcntl.h>  /* for O_RDONLY */

#include "open.h"

#define BUFFSIZE 8192

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    int n;
    char buf[BUFFSIZE];
    char line[MAXLINE];

    /* read filename to cat from stdin */
    while (fgets(line, MAXLINE, stdin) != nullptr) {
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = 0; /* replace newline with null */
        }

        /* open the file */
        int fd = csopen(line, O_RDONLY);
        if (fd < 0) {
            continue; /* csopen() prints error from server */
        }

        /* and cat to stdout */
        for (;;) {
            n = read(fd, buf, BUFFSIZE);
            if (n <= 0) {
                break;
            }
            if (write(STDOUT_FILENO, buf, n) != n) {
                err_sys("write error");
            }
        }
        if (n < 0) {
            err_sys("read error");
        }
        close(fd);
    }

    exit(0);
}
