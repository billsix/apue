#include <stdio.h>     /* for printf, setbuf, stdout */
#include <stdlib.h>    /* for exit, strtol */
#include <unistd.h>    /* for fork, nice, sysconf, _SC_NZERO */
#include <errno.h>     /* for errno */
#include <sys/time.h>  /* for gettimeofday, struct timeval */
#include <sys/types.h> /* for pid_t */

#if defined(MACOS)
#include <sys/syslimits.h>
#elif defined(SOLARIS)
#include <limits.h>
#elif defined(BSD)
#include <sys/param.h>
#endif

#include "apue.h"

unsigned long long count;
struct timeval end;

void checktime(const char *str) {
    struct timeval tv;

    gettimeofday(&tv, nullptr);
    if (tv.tv_sec >= end.tv_sec && tv.tv_usec >= end.tv_usec) {
        printf("%s count = %lld\n", str, count);
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, nullptr);
#if defined(NZERO)
    const int nzero = NZERO;
#elif defined(_SC_NZERO)
    const int nzero = sysconf(_SC_NZERO);
#else
#error NZERO undefined
#endif
    printf("NZERO = %d\n", nzero);
    int adj = 0;
    if (argc == 2) {
        adj = strtol(argv[1], nullptr, 10);
    }
    gettimeofday(&end, nullptr);
    end.tv_sec += 10; /* run for 10 seconds */

    const char *s;
    const pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork failed");
    } else if (pid == 0) { /* child */
        s = "child";
        printf("current nice value in child is %d, adjusting by %d\n",
               nice(0) + nzero, adj);
        errno = 0;
        const int ret = nice(adj);
        if (ret == -1 && errno != 0) {
            err_sys("child set scheduling priority");
        }
        printf("now child nice value is %d\n", ret + nzero);
    } else { /* parent */
        s = "parent";
        printf("current nice value in parent is %d\n", nice(0) + nzero);
    }
    for (;;) {
        if (++count == 0) {
            err_quit("%s counter wrap", s);
        }
        checktime(s);
    }
}
