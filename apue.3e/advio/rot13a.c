#include <stddef.h>    /* for size_t */
#include <sys/types.h> /* for ssize_t */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for fsync, read, write */
#include <ctype.h>     /* for isalpha */
#include <fcntl.h>     /* for open, O_RDONLY, O_RDWR, O_CREAT, O_TRUNC */

#include "apue.h"

#define BSZ 4096

unsigned char buf[BSZ];

unsigned char translate(unsigned char c) {
    if (isalpha(c)) {
        if (c >= 'n') {
            c -= 13;
        } else if (c >= 'a') {
            c += 13;
        } else if (c >= 'N') {
            c -= 13;
        } else {
            c += 13;
        }
    }
    return (c);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        err_quit("usage: rot13 infile outfile");
    }
    const int ifd = open(argv[1], O_RDONLY);
    if (ifd < 0) {
        err_sys("can't open %s", argv[1]);
    }
    const int ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
    if (ofd < 0) {
        err_sys("can't create %s", argv[2]);
    }

    for (;;) {
        int n = read(ifd, buf, BSZ);
        if (n <= 0) {
            break;
        }
        for (int i = 0; i < n; i++) {
            buf[i] = translate(buf[i]);
        }
        int nw = write(ofd, buf, n);
        if (nw != n) {
            if (nw < 0) {
                err_sys("write failed");
            } else {
                err_quit("short write (%d/%d)", nw, n);
            }
        }
    }

    fsync(ofd);
    exit(0);
}
