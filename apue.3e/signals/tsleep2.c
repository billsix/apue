#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <signal.h> /* for signal, SIG_ERR, SIGINT */

#include "apue.h"

unsigned int sleep2(unsigned int);
static void sig_int(int);

int main(void) {
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        err_sys("signal(SIGINT) error");
    }
    const unsigned int unslept = sleep2(5);
    printf("sleep2 returned: %u\n", unslept);
    exit(0);
}

static void sig_int([[maybe_unused]] int signo) {
    volatile int k = 0;

    /*
     * Tune these loops to run for more than 5 seconds
     * on whatever system this test program is run.
     */
    printf("\nsig_int starting\n");
    for (int i = 0; i < 300000; i++) {
        for (int j = 0; j < 4000; j++) {
            k += i * j;
        }
    }
    printf("sig_int finished\n");
}
