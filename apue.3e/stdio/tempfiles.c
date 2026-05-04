#include <stdio.h>     /* for FILE, L_tmpnam, tmpnam, tmpfile, fputs, fgets, rewind, printf, stdout */
#include <stdlib.h>    /* for exit */

#include "apue.h"

int main(void) {
    char name[L_tmpnam], line[MAXLINE];
    FILE *fp;

    printf("%s\n", tmpnam(NULL)); /* first temp name */

    tmpnam(name); /* second temp name */
    printf("%s\n", name);

    if ((fp = tmpfile()) == NULL) { /* create temp file */
        err_sys("tmpfile error");
    }
    fputs("one line of output\n", fp); /* write to temp file */
    rewind(fp);                        /* then read it back */
    if (fgets(line, sizeof(line), fp) == NULL) {
        err_sys("fgets error");
    }
    fputs(line, stdout); /* print the line we wrote */

    exit(0);
}
