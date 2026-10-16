#include <stdio.h>  /* for fputs, stdout */
#include <stdlib.h> /* for exit */
#include <time.h>   /* for time, localtime, strftime, time_t, struct tm */

#include "apue.h"

int main(void) {
    time_t caltime = time(nullptr);
    if (caltime == -1) {
        err_sys("time error");
    }
    const struct tm *tm = localtime(&caltime);
    if (tm == nullptr) {
        err_sys("localtime error");
    }
    char line[MAXLINE];
    if (strftime(line, MAXLINE, "%a %b %d %X %Z %Y\n", tm) == 0) {
        err_sys("strftime error");
    }
    fputs(line, stdout);
    exit(0);
}
