#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <dirent.h> /* for opendir, readdir */

#include "apue.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        err_quit("usage: ls directory_name");
    }

    DIR *dp = opendir(argv[1]);
    if (dp == nullptr) {
        err_sys("can't open %s", argv[1]);
    }
    for (;;) {
        struct dirent *dirp = readdir(dp);
        if (dirp == nullptr) {
            break;
        }
        printf("%s\n", dirp->d_name);
    }

    closedir(dp);
    exit(0);
}
