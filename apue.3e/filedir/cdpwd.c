#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for chdir, getcwd */

#include "apue.h"

int main(void) {
    if (chdir("/usr/spool/uucppublic") < 0) {
        err_sys("chdir failed");
    }

    size_t size;
    char *const ptr = path_alloc(&size); /* our own function */
    if (getcwd(ptr, size) == nullptr) {
        err_sys("getcwd failed");
    }

    printf("cwd = %s\n", ptr);
    exit(0);
}
