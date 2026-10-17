#include <stdio.h>  /* for printf */
#include <string.h> /* for strcmp */
#include <unistd.h> /* for alarm */
#include <signal.h> /* for signal, SIGALRM */
#include <pwd.h>    /* for getpwnam, struct passwd */

#include "apue.h"

static void my_alarm([[maybe_unused]] int signo) {
    printf("in signal handler\n");
    const struct passwd *const rootptr = getpwnam("root");
    if (rootptr == nullptr) {
        err_sys("getpwnam(root) error");
    }
    alarm(1);
}

int main(void) {
    signal(SIGALRM, my_alarm);
    alarm(1);
    for (;;) {
        const struct passwd *const ptr = getpwnam("sar");
        if (ptr == nullptr) {
            err_sys("getpwnam error");
        }
        if (strcmp(ptr->pw_name, "sar") != 0) {
            printf("return value corrupted!, pw_name = %s\n", ptr->pw_name);
        }
    }
}
