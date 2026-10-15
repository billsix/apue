#include <unistd.h> /* for _exit, vfork */

#include "apue.h"

static void f1(void), f2(void);

int main(void) {
    f1();
    f2();
    _exit(0);
}

static void f1(void) {
    pid_t pid = vfork();
    if (pid < 0) {
        err_sys("vfork error");
    }
    /* child and parent both return */
}

static void f2(void) {
    char buf[1000]; /* automatic variables */

    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = 0;
    }
}
