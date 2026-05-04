#include <stdio.h>     /* for fputs, stdout */
#include <stdlib.h>    /* for exit */
#include <time.h>      /* for time, localtime, strftime, time_t, struct tm */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

int main(void) {
    time_t caltime;
    struct tm *tm;
    char line[MAXLINE];

    if ((caltime = time(NULL)) == -1) {
        err_sys("time error");
    }
    if ((tm = localtime(&caltime)) == NULL) {
        err_sys("localtime error");
    }
    if (strftime(line, MAXLINE, "%a %b %d %X %Z %Y\n", tm) == 0) {
        err_sys("strftime error");
    }
    fputs(line, stdout);
    exit(0);
}
