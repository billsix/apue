#include <stdio.h>      /* for printf */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for memset */
#include <unistd.h>     /* for read, write */
#include <poll.h>       /* for poll, struct pollfd, POLLIN */
#include <pthread.h>    /* for pthread_create, pthread_t */
#include <sys/ipc.h>    /* for IPC_CREAT */
#include <sys/msg.h>    /* for msgget, msgrcv, MSG_NOERROR */
#include <sys/socket.h> /* for socketpair, AF_UNIX, SOCK_DGRAM */

#include "apue.h"

#define NQ 3       /* number of queues */
#define MAXMSZ 512 /* maximum message size */
#define KEY 0x123  /* key for first message queue */

struct threadinfo {
    int qid;
    int fd;
};

struct mymesg {
    long mtype;
    char mtext[MAXMSZ];
};

void *helper(void *arg) {
    struct mymesg m;
    const struct threadinfo *tip = arg;

    for (;;) {
        memset(&m, 0, sizeof(m));
        int n = msgrcv(tip->qid, &m, MAXMSZ, 0, MSG_NOERROR);
        if (n < 0) {
            err_sys("msgrcv error");
        }
        if (write(tip->fd, m.mtext, n) < 0) {
            err_sys("write error");
        }
    }
}

int main(void) {
    int fd[2];
    int qid[NQ];
    struct pollfd pfd[NQ];
    struct threadinfo ti[NQ];
    pthread_t tid[NQ];
    char buf[MAXMSZ];

    for (int i = 0; i < NQ; i++) {
        qid[i] = msgget((KEY + i), IPC_CREAT | 0666);
        if (qid[i] < 0) {
            err_sys("msgget error");
        }

        printf("queue ID %d is %d\n", i, qid[i]);

        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) < 0) {
            err_sys("socketpair error");
        }
        pfd[i].fd = fd[0];
        pfd[i].events = POLLIN;
        ti[i].qid = qid[i];
        ti[i].fd = fd[1];
        int err = pthread_create(&tid[i], nullptr, helper, &ti[i]);
        if (err != 0) {
            err_exit(err, "pthread_create error");
        }
    }

    for (;;) {
        if (poll(pfd, NQ, -1) < 0) {
            err_sys("poll error");
        }
        for (int i = 0; i < NQ; i++) {
            if (pfd[i].revents & POLLIN) {
                int n = read(pfd[i].fd, buf, sizeof(buf));
                if (n < 0) {
                    err_sys("read error");
                }
                buf[n] = 0;
                printf("queue id %d, message %s\n", qid[i], buf);
            }
        }
    }

    exit(0);
}
