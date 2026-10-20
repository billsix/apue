#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <string.h> /* for memset */
#include <unistd.h> /* for read, write */
#include <poll.h>   /* for poll, struct pollfd, POLLIN */
#include <pthread.h> /* for pthread_create, pthread_mutex_*, pthread_cond_*, pthread_t */
#include <sys/ipc.h>    /* for IPC_CREAT */
#include <sys/msg.h>    /* for msgget, msgrcv, MSG_NOERROR */
#include <sys/socket.h> /* for socketpair, AF_UNIX, SOCK_DGRAM */

#include "apue.h"

#define NQ 3       /* number of queues */
#define MAXMSZ 512 /* maximum message size */
#define KEY 0x123  /* key for first message queue */

struct mymesg {
    long mtype;
    char mtext[MAXMSZ + 1];
};

struct threadinfo {
    int qid;
    int fd;
    int len;
    pthread_mutex_t mutex;
    pthread_cond_t ready;
    struct mymesg m;
};

void *helper(void *arg) {
    struct threadinfo *const tip = arg;

    while (true) {
        memset(&tip->m, 0, sizeof(struct mymesg));
        const int n = msgrcv(tip->qid, &tip->m, MAXMSZ, 0, MSG_NOERROR);
        if (n < 0) {
            err_sys("msgrcv error");
        }
        tip->len = n;
        pthread_mutex_lock(&tip->mutex);
        if (write(tip->fd, "a", sizeof(char)) < 0) {
            err_sys("write error");
        }
        pthread_cond_wait(&tip->ready, &tip->mutex);
        pthread_mutex_unlock(&tip->mutex);
    }
}

int main(void) {
    int qid[NQ];
    struct pollfd pfd[NQ];
    struct threadinfo ti[NQ];
    pthread_t tid[NQ];

    for (int i = 0; i < NQ; i++) {
        qid[i] = msgget((KEY + i), IPC_CREAT | 0666);
        if (qid[i] < 0) {
            err_sys("msgget error");
        }

        printf("queue ID %d is %d\n", i, qid[i]);

        int fd[2];
        if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) < 0) {
            err_sys("socketpair error");
        }
        pfd[i].fd = fd[0];
        pfd[i].events = POLLIN;
        ti[i].qid = qid[i];
        ti[i].fd = fd[1];
        if (pthread_cond_init(&ti[i].ready, nullptr) != 0) {
            err_sys("pthread_cond_init error");
        }
        if (pthread_mutex_init(&ti[i].mutex, nullptr) != 0) {
            err_sys("pthread_mutex_init error");
        }
        const int err = pthread_create(&tid[i], nullptr, helper, &ti[i]);
        if (err != 0) {
            err_exit(err, "pthread_create error");
        }
    }

    while (true) {
        if (poll(pfd, NQ, -1) < 0) {
            err_sys("poll error");
        }
        for (int i = 0; i < NQ; i++) {
            if (pfd[i].revents & POLLIN) {
                char c;
                const int n = read(pfd[i].fd, &c, sizeof(char));
                if (n < 0) {
                    err_sys("read error");
                }
                ti[i].m.mtext[ti[i].len] = 0;
                printf("queue id %d, message %s\n", qid[i], ti[i].m.mtext);
                pthread_mutex_lock(&ti[i].mutex);
                pthread_cond_signal(&ti[i].ready);
                pthread_mutex_unlock(&ti[i].mutex);
            }
        }
    }

    exit(0);
}
