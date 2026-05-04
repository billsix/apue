#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for realloc, exit */
#include <string.h>    /* for strlen */
#include <unistd.h>    /* for chdir, getcwd */
#include <fcntl.h>     /* for creat */
#include <sys/types.h> /* needed for apue.h */
#include <sys/stat.h>  /* for mkdir, FILE_MODE/DIR_MODE constants */

#include "apue.h"

#define DEPTH 1000 /* directory depth */
#define STARTDIR "/tmp"
#define NAME "alonglonglonglonglonglonglonglonglonglongname"
#define MAXSZ (10 * 8192)

int main(void) {
    int i;
    size_t size;
    char *path;

    if (chdir(STARTDIR) < 0) {
        err_sys("chdir error");
    }

    for (i = 0; i < DEPTH; i++) {
        if (mkdir(NAME, DIR_MODE) < 0) {
            err_sys("mkdir failed, i = %d", i);
        }
        if (chdir(NAME) < 0) {
            err_sys("chdir failed, i = %d", i);
        }
    }

    if (creat("afile", FILE_MODE) < 0) {
        err_sys("creat error");
    }

    /*
     * The deep directory is created, with a file at the leaf.
     * Now let's try to obtain its pathname.
     */
    path = path_alloc(&size);
    for (;;) {
        if (getcwd(path, size) != NULL) {
            break;
        } else {
            err_ret("getcwd failed, size = %ld", (long)size);
            size += 100;
            if (size > MAXSZ) {
                err_quit("giving up");
            }
            if ((path = realloc(path, size)) == NULL) {
                err_sys("realloc error");
            }
        }
    }
    printf("length = %ld\n%s\n", (long)strlen(path), path);

    exit(0);
}
