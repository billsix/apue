#include <stdio.h>     /* for printf, perror */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for fork, _exit, sleep, pause */
#include <signal.h>    /* for signal, SIG_ERR, SIGCHLD */
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h>  /* for wait */

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif

static void sig_cld(int);

int main(void) {
    if (signal(SIGCLD, sig_cld) == SIG_ERR) {
        perror("signal error");
    }
    const pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
    } else if (pid == 0) { /* child */
        sleep(2);
        _exit(0);
    }

    pause(); /* parent */
    exit(0);
}

static void sig_cld([[maybe_unused]] int signo) /* interrupts pause() */
{
    printf("SIGCLD received\n");

    if (signal(SIGCLD, sig_cld) == SIG_ERR) { /* reestablish handler */
        perror("signal error");
    }

    int status;
    const pid_t pid = wait(&status); /* fetch child status */
    if (pid < 0) {
        perror("wait error");
    }

    printf("pid = %d\n", pid);
}
