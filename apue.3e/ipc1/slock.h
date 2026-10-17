#include <semaphore.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>

struct slock {
    sem_t *semp;
    char name[_POSIX_NAME_MAX];
};

struct slock *s_alloc();
void s_free(struct slock *);
int s_lock(const struct slock *);
int s_trylock(const struct slock *);
int s_unlock(const struct slock *);
