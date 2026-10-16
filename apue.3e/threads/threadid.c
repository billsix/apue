#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getpid, sleep */
#include <pthread.h>   /* for pthread_create, pthread_self, pthread_t */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

pthread_t ntid;

void printids(const char *s) {
    const pid_t pid = getpid();
    const pthread_t tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid,
           (unsigned long)tid, (unsigned long)tid);
}

void *thr_fn([[maybe_unused]] void *arg) {
    printids("new thread: ");
    return ((void *)0);
}

int main(void) {
    int err = pthread_create(&ntid, nullptr, thr_fn, nullptr);
    if (err != 0) {
        err_exit(err, "can't create thread");
    }
    printids("main thread:");
    sleep(1);
    exit(0);
}
