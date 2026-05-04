#include <stdio.h>      /* for printf */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for strcpy, strlen */
#include <stddef.h>     /* for offsetof */
#include <sys/socket.h> /* for socket, bind, AF_UNIX, SOCK_STREAM, struct sockaddr */
#include <sys/un.h>     /* for struct sockaddr_un */

#include "apue.h"

int main(void) {
    int fd, size;
    struct sockaddr_un un;

    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, "foo.socket");
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        err_sys("socket failed");
    }
    size = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
    if (bind(fd, (struct sockaddr *)&un, size) < 0) {
        err_sys("bind failed");
    }
    printf("UNIX domain socket bound\n");
    exit(0);
}
