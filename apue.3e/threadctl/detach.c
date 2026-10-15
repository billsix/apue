#include <pthread.h> /* for pthread_attr_init, pthread_attr_setdetachstate, pthread_attr_destroy, pthread_create, pthread_attr_t, pthread_t, PTHREAD_CREATE_DETACHED */

int makethread(void *(*fn)(void *), void *arg) {
    pthread_attr_t attr;
    int err = pthread_attr_init(&attr);
    if (err != 0) {
        return (err);
    }
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err == 0) {
        pthread_t tid;
        err = pthread_create(&tid, &attr, fn, arg);
    }
    pthread_attr_destroy(&attr);
    return (err);
}
