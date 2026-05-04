#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <string.h>    /* for strerror */
#include <time.h>      /* for clock_gettime, localtime, strftime, struct timespec, struct tm, CLOCK_REALTIME */
#include <pthread.h>   /* for pthread_mutex_lock, pthread_mutex_timedlock, pthread_mutex_t, PTHREAD_MUTEX_INITIALIZER */

#include "apue.h"

int main(void) {
    int err;
    struct timespec tout;
    struct tm *tmp;
    char buf[64];
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&lock);
    printf("mutex is locked\n");
    clock_gettime(CLOCK_REALTIME, &tout);
    tmp = localtime(&tout.tv_sec);
    strftime(buf, sizeof(buf), "%r", tmp);
    printf("current time is %s\n", buf);
    tout.tv_sec += 10; /* 10 seconds from now */
    /* caution: this could lead to deadlock */
    err = pthread_mutex_timedlock(&lock, &tout);
    clock_gettime(CLOCK_REALTIME, &tout);
    tmp = localtime(&tout.tv_sec);
    strftime(buf, sizeof(buf), "%r", tmp);
    printf("the time is now %s\n", buf);
    if (err == 0) {
        printf("mutex locked again!\n");
    } else {
        printf("can't lock mutex again: %s\n", strerror(err));
    }
    exit(0);
}
