#include <stdlib.h> /* for exit */
#include <unistd.h> /* for fork, getppid, close */
#include <fcntl.h>  /* for open, O_RDWR */
#include <sys/mman.h> /* for mmap, PROT_READ, PROT_WRITE, MAP_SHARED, MAP_FAILED */
#include <sys/types.h> /* for pid_t */

#include "apue.h"

#define NLOOPS 1000
#define SIZE sizeof(long) /* size of shared memory area */

static int update(long *ptr) {
    return ((*ptr)++); /* return value before increment */
}

int main(void) {
    const int fd = open("/dev/zero", O_RDWR);
    if (fd < 0) {
        err_sys("open error");
    }
    void *area = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (area == MAP_FAILED) {
        err_sys("mmap error");
    }
    close(fd); /* can close /dev/zero now that it's mapped */

    TELL_WAIT();

    const pid_t pid = fork();
    if (pid < 0) {
        err_sys("fork error");
    } else if (pid > 0) { /* parent */
        for (int i = 0; i < NLOOPS; i += 2) {
            const int counter = update((long *)area);
            if (counter != i) {
                err_quit("parent: expected %d, got %d", i, counter);
            }

            TELL_CHILD(pid);
            WAIT_CHILD();
        }
    } else { /* child */
        for (int i = 1; i < NLOOPS + 1; i += 2) {
            WAIT_PARENT();

            const int counter = update((long *)area);
            if (counter != i) {
                err_quit("child: expected %d, got %d", i, counter);
            }

            TELL_PARENT(getppid());
        }
    }

    exit(0);
}
