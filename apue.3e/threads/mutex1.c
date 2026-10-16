#include <stdlib.h>  /* for malloc, free */
#include <pthread.h> /* for pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_unlock, pthread_mutex_t */

struct foo {
    int f_count;
    pthread_mutex_t f_lock;
    int f_id;
    /* ... more stuff here ... */
};

struct foo *foo_alloc(int id) /* allocate the object */
{
    struct foo *fp = malloc(sizeof(struct foo));
    if (fp != nullptr) {
        fp->f_count = 1;
        fp->f_id = id;
        if (pthread_mutex_init(&fp->f_lock, nullptr) != 0) {
            free(fp);
            return (nullptr);
        }
        /* ... continue initialization ... */
    }
    return (fp);
}

void foo_hold(struct foo *fp) /* add a reference to the object */
{
    pthread_mutex_lock(&fp->f_lock);
    fp->f_count++;
    pthread_mutex_unlock(&fp->f_lock);
}

void foo_rele(struct foo *fp) /* release a reference to the object */
{
    pthread_mutex_lock(&fp->f_lock);
    if (--fp->f_count == 0) { /* last reference */
        pthread_mutex_unlock(&fp->f_lock);
        pthread_mutex_destroy(&fp->f_lock);
        free(fp);
    } else {
        pthread_mutex_unlock(&fp->f_lock);
    }
}
