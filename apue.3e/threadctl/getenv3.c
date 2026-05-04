#include <stdlib.h>  /* for malloc, free, NULL */
#include <string.h>  /* for strlen, strncmp, strncpy */
#include <pthread.h> /* for pthread_key_create, pthread_once, pthread_getspecific, pthread_setspecific, pthread_mutex_lock, pthread_mutex_unlock, pthread_key_t, pthread_once_t, pthread_mutex_t, PTHREAD_ONCE_INIT, PTHREAD_MUTEX_INITIALIZER */

#define MAXSTRINGSZ 4096

static pthread_key_t key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;
pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;

extern char **environ;

static void thread_init(void) { pthread_key_create(&key, free); }

char *getenv(const char *name) {
    int i, len;
    char *envbuf;

    pthread_once(&init_done, thread_init);
    pthread_mutex_lock(&env_mutex);
    envbuf = (char *)pthread_getspecific(key);
    if (envbuf == NULL) {
        envbuf = malloc(MAXSTRINGSZ);
        if (envbuf == NULL) {
            pthread_mutex_unlock(&env_mutex);
            return (NULL);
        }
        pthread_setspecific(key, envbuf);
    }
    len = strlen(name);
    for (i = 0; environ[i] != NULL; i++) {
        if ((strncmp(name, environ[i], len) == 0) && (environ[i][len] == '=')) {
            strncpy(envbuf, &environ[i][len + 1], MAXSTRINGSZ - 1);
            pthread_mutex_unlock(&env_mutex);
            return (envbuf);
        }
    }
    pthread_mutex_unlock(&env_mutex);
    return (NULL);
}
