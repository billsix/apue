#include <stdio.h>     /* for printf, setbuf, stdout */
#include <stdlib.h>    /* for exit, system */
#include <unistd.h>    /* for sysconf, _SC_CLK_TCK */
#include <sys/times.h> /* for times, struct tms, clock_t */

#include "apue.h"

static void pr_times(clock_t, const struct tms *, const struct tms *);
static void do_cmd(const char *);

int main(int argc, char *argv[]) {
    setbuf(stdout, nullptr);
    for (int i = 1; i < argc; i++) {
        do_cmd(argv[i]); /* once for each command-line arg */
    }
    exit(0);
}

static void do_cmd(const char *cmd) /* execute and time the "cmd" */
{
    printf("\ncommand: %s\n", cmd);

    struct tms tmsstart;
    const clock_t start = times(&tmsstart); /* starting values */
    if (start == -1) {
        err_sys("times error");
    }

    const int status = system(cmd); /* execute command */
    if (status < 0) {
        err_sys("system() error");
    }

    struct tms tmsend;
    const clock_t end = times(&tmsend); /* ending values */
    if (end == -1) {
        err_sys("times error");
    }

    pr_times(end - start, &tmsstart, &tmsend);
    pr_exit(status);
}

static void pr_times(clock_t real, const struct tms *tmsstart,
                     const struct tms *tmsend) {
    static long clktck = 0;

    if (clktck == 0) { /* fetch clock ticks per second first time */
        clktck = sysconf(_SC_CLK_TCK);
        if (clktck < 0) {
            err_sys("sysconf error");
        }
    }

    printf("  real:  %7.2f\n", real / (double)clktck);
    printf("  user:  %7.2f\n",
           (tmsend->tms_utime - tmsstart->tms_utime) / (double)clktck);
    printf("  sys:   %7.2f\n",
           (tmsend->tms_stime - tmsstart->tms_stime) / (double)clktck);
    printf("  child user:  %7.2f\n",
           (tmsend->tms_cutime - tmsstart->tms_cutime) / (double)clktck);
    printf("  child sys:   %7.2f\n",
           (tmsend->tms_cstime - tmsstart->tms_cstime) / (double)clktck);
}
