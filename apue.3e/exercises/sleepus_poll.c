#include <poll.h>

void sleep_us(unsigned int nusecs) {
    int timeout = nusecs / 1000;
    if (timeout <= 0) {
        timeout = 1;
    }
    struct pollfd dummy;
    poll(&dummy, 0, timeout);
}
