#include <unistd.h>        /* for sysconf, _SC_OPEN_MAX */
#include <limits.h>        /* for LONG_MAX */
#include <sys/resource.h>  /* for getrlimit, RLIMIT_NOFILE, RLIM_INFINITY, struct rlimit */

#include "apue.h"

#define OPEN_MAX_GUESS 256

long open_max(void) {
    long openmax;
    struct rlimit rl;

    if ((openmax = sysconf(_SC_OPEN_MAX)) < 0 || openmax == LONG_MAX) {
        if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
            err_sys("can't get file limit");
        }
        if (rl.rlim_max == RLIM_INFINITY) {
            openmax = OPEN_MAX_GUESS;
        } else {
            openmax = rl.rlim_max;
        }
    }
    return (openmax);
}
