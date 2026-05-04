#include <stddef.h>     /* for NULL */
#include <sys/select.h> /* for select, struct timeval */

void sleep_us(unsigned int nusecs) {
    struct timeval tval;

    tval.tv_sec = nusecs / 1000000;
    tval.tv_usec = nusecs % 1000000;
    select(0, NULL, NULL, NULL, &tval);
}
