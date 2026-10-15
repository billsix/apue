#include <stdio.h>  /* for FILE, fopen, fprintf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for getlogin */

#include "apue.h"

int main(void) {
    daemonize("getlog");
    const char *p = getlogin();
    FILE *fp = fopen("/tmp/getlog.out", "w");
    if (fp != nullptr) {
        if (p == nullptr) {
            fprintf(fp, "no login name\n");
        } else {
            fprintf(fp, "login name: %s\n", p);
        }
    }
    exit(0);
}
