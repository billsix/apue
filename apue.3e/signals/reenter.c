#include <stdio.h>     /* for printf */
#include <string.h>    /* for strcmp */
#include <unistd.h>    /* for alarm */
#include <signal.h>    /* for signal, SIGALRM */
#include <pwd.h>       /* for getpwnam, struct passwd */

#include "apue.h"

static void my_alarm(int signo) {
    struct passwd *rootptr;

    printf("in signal handler\n");
    if ((rootptr = getpwnam("root")) == NULL) {
        err_sys("getpwnam(root) error");
    }
    alarm(1);
}

int main(void) {
    struct passwd *ptr;

    signal(SIGALRM, my_alarm);
    alarm(1);
    for (;;) {
        if ((ptr = getpwnam("sar")) == NULL) {
            err_sys("getpwnam error");
        }
        if (strcmp(ptr->pw_name, "sar") != 0) {
            printf("return value corrupted!, pw_name = %s\n", ptr->pw_name);
        }
    }
}
