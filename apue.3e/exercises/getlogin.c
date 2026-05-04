#include <stdio.h>     /* for FILE, fopen, fprintf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getlogin */
#include <sys/types.h> /* needed for apue.h */

#include "apue.h"

int main(void) {
    FILE *fp;
    char *p;

    daemonize("getlog");
    p = getlogin();
    fp = fopen("/tmp/getlog.out", "w");
    if (fp != NULL) {
        if (p == NULL) {
            fprintf(fp, "no login name\n");
        } else {
            fprintf(fp, "login name: %s\n", p);
        }
    }
    exit(0);
}
