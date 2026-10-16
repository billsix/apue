#include <sys/stat.h> /* for struct stat */
#include <stdlib.h>   /* for exit */
#include <string.h>   /* for memcpy */
#include <unistd.h>   /* for ftruncate */
#include <fcntl.h>
#include <sys/mman.h>

#include "apue.h"

#define COPYINCR (1024 * 1024 * 1024) /* 1 GB */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        err_quit("usage: %s <fromfile> <tofile>", argv[0]);
    }

    const int fdin = open(argv[1], O_RDONLY);
    if (fdin < 0) {
        err_sys("can't open %s for reading", argv[1]);
    }

    const int fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
    if (fdout < 0) {
        err_sys("can't creat %s for writing", argv[2]);
    }

    struct stat sbuf;
    if (fstat(fdin, &sbuf) < 0) { /* need size of input file */
        err_sys("fstat error");
    }

    if (ftruncate(fdout, sbuf.st_size) < 0) { /* set output file size */
        err_sys("ftruncate error");
    }

    off_t fsz = 0;
    while (fsz < sbuf.st_size) {
        size_t copysz;
        if ((sbuf.st_size - fsz) > COPYINCR) {
            copysz = COPYINCR;
        } else {
            copysz = sbuf.st_size - fsz;
        }

        void *src = mmap(nullptr, copysz, PROT_READ, MAP_SHARED, fdin, fsz);
        if (src == MAP_FAILED) {
            err_sys("mmap error for input");
        }
        void *dst = mmap(nullptr, copysz, PROT_READ | PROT_WRITE, MAP_SHARED,
                         fdout, fsz);
        if (dst == MAP_FAILED) {
            err_sys("mmap error for output");
        }

        memcpy(dst, src, copysz); /* does the file copy */
        munmap(src, copysz);
        munmap(dst, copysz);
        fsz += copysz;
    }
    exit(0);
}
