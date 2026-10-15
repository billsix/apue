#include <stdio.h> /* for FILE, L_tmpnam, tmpnam, tmpfile, fputs, fgets, rewind, printf, stdout */
#include <stdlib.h> /* for exit */

#include "apue.h"

int main(void) {
    printf("%s\n", tmpnam(nullptr)); /* first temp name */

    char name[L_tmpnam];
    tmpnam(name); /* second temp name */
    printf("%s\n", name);

    FILE *fp = tmpfile(); /* create temp file */
    if (fp == nullptr) {
        err_sys("tmpfile error");
    }
    fputs("one line of output\n", fp); /* write to temp file */
    rewind(fp);                        /* then read it back */

    char line[MAXLINE];
    if (fgets(line, sizeof(line), fp) == nullptr) {
        err_sys("fgets error");
    }
    fputs(line, stdout); /* print the line we wrote */

    exit(0);
}
