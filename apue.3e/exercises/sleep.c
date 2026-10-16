#include <unistd.h>
#include <time.h>
#include <sys/select.h>

unsigned sleep(unsigned seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    time_t start;
    time(&start);
    const int n = select(0, nullptr, nullptr, nullptr, &tv);
    if (n == 0) {
        return (0);
    }
    time_t end;
    time(&end);
    const unsigned slept = end - start;
    if (slept >= seconds) {
        return (0);
    }
    return (seconds - slept);
}
