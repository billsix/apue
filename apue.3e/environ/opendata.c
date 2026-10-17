#include <stdio.h> /* for FILE, fopen, setvbuf, BUFSIZ, _IOLBF */

FILE *open_data(void) {
    char databuf[BUFSIZ]; /* setvbuf makes this the stdio buffer */

    FILE *const fp = fopen("datafile", "r");
    if (fp == nullptr) {
        return (nullptr);
    }
    if (setvbuf(fp, databuf, _IOLBF, BUFSIZ) != 0) {
        return (nullptr);
    }
    return (fp); /* error */
}
