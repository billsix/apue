#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for exit */
#include <unistd.h>  /* for fork, sleep, pause */
#include <pthread.h> /* for pthread_atfork, pthread_create, pthread_mutex_lock, pthread_mutex_unlock, pthread_mutex_t, pthread_t, PTHREAD_MUTEX_INITIALIZER */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void prepare(void) {
    printf("preparing locks...\n");
    int err = pthread_mutex_lock(&lock1);
    if (err != 0) {
        err_cont(err, "can't lock lock1 in prepare handler");
    }
    err = pthread_mutex_lock(&lock2);
    if (err != 0) {
        err_cont(err, "can't lock lock2 in prepare handler");
    }
}

void parent(void) {
    printf("parent unlocking locks...\n");
    int err = pthread_mutex_unlock(&lock1);
    if (err != 0) {
        err_cont(err, "can't unlock lock1 in parent handler");
    }
    err = pthread_mutex_unlock(&lock2);
    if (err != 0) {
        err_cont(err, "can't unlock lock2 in parent handler");
    }
}

void child(void) {
    printf("child unlocking locks...\n");
    int err = pthread_mutex_unlock(&lock1);
    if (err != 0) {
        err_cont(err, "can't unlock lock1 in child handler");
    }
    err = pthread_mutex_unlock(&lock2);
    if (err != 0) {
        err_cont(err, "can't unlock lock2 in child handler");
    }
}

void *thr_fn([[maybe_unused]] void *arg) {
    printf("thread started...\n");
    pause();
    return (nullptr);
}

int main(void) {
    int err = pthread_atfork(prepare, parent, child);
    if (err != 0) {
        err_exit(err, "can't install fork handlers");
    }
    pthread_t tid;
    err = pthread_create(&tid, nullptr, thr_fn, nullptr);
    if (err != 0) {
        err_exit(err, "can't create thread");
    }

    sleep(2);
    printf("parent about to fork...\n");

    const pid_t pid = fork();
    if (pid < 0) {
        err_quit("fork failed");
    } else if (pid == 0) { /* child */
        printf("child returned from fork\n");
    } else { /* parent */
        printf("parent returned from fork\n");
    }
    exit(0);
}
