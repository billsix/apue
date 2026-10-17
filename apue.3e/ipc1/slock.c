#include <stdlib.h> /* for malloc, free */
#include <stdio.h>  /* for snprintf */
#include <unistd.h> /* for getpid */
#include <errno.h>  /* for errno, EEXIST */

#include "slock.h"

struct slock *s_alloc() {
    static int cnt;

    struct slock *const sp = malloc(sizeof(struct slock));
    if (sp == nullptr) {
        return (nullptr);
    }
    do {
        snprintf(sp->name, sizeof(sp->name), "/%ld.%d", (long)getpid(), cnt++);
        sp->semp = sem_open(sp->name, O_CREAT | O_EXCL, S_IRWXU, 1);
    } while ((sp->semp == SEM_FAILED) && (errno == EEXIST));
    if (sp->semp == SEM_FAILED) {
        free(sp);
        return (nullptr);
    }
    sem_unlink(sp->name);
    return (sp);
}

void s_free(struct slock *sp) {
    sem_close(sp->semp);
    free(sp);
}

int s_lock(const struct slock *sp) { return (sem_wait(sp->semp)); }

int s_trylock(const struct slock *sp) { return (sem_trywait(sp->semp)); }

int s_unlock(const struct slock *sp) { return (sem_post(sp->semp)); }
