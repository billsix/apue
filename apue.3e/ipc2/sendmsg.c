#include <stdio.h>   /* for fprintf, stderr */
#include <stdlib.h>  /* for exit, strtol */
#include <string.h>  /* for strlen */
#include <sys/ipc.h> /* for key_t */
#include <sys/msg.h> /* for msgget, msgsnd */

#include "apue.h"

#define MAXMSZ 512

struct mymesg {
    long mtype;
    char mtext[MAXMSZ];
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: sendmsg KEY message\n");
        exit(1);
    }
    const key_t key = strtol(argv[1], nullptr, 0);
    const long qid = msgget(key, 0);
    if (qid < 0) {
        err_sys("can't open queue key %s", argv[1]);
    }
    struct mymesg m = {0};
    snprintf(m.mtext, sizeof(m.mtext), "%s", argv[2]);
    const size_t nbytes = strlen(m.mtext);
    m.mtype = 1;
    if (msgsnd(qid, &m, nbytes, 0) < 0) {
        err_sys("can't send message");
    }
    exit(0);
}
