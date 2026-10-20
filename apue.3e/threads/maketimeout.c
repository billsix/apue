#include <sys/time.h> /* for gettimeofday, struct timeval */
#include <time.h>     /* for struct timespec */

void maketimeout(struct timespec *tsp, long minutes) {
    /* get the current time */
    struct timeval now;
    gettimeofday(&now, nullptr);
    tsp->tv_sec = now.tv_sec;
    tsp->tv_nsec = now.tv_usec * 1000; /* usec to nsec */
    /* add the offset to get timeout value */
    tsp->tv_sec += minutes * 60;
}
