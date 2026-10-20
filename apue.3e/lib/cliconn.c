#include <stddef.h>   /* for offsetof */
#include <stdio.h>    /* for printf, snprintf */
#include <string.h>   /* for strlen */
#include <errno.h>    /* for errno, ENAMETOOLONG */
#include <unistd.h>   /* for getpid, unlink, close */
#include <sys/stat.h> /* for chmod, S_IRWXU */
#include <sys/socket.h> /* for socket, bind, connect, AF_UNIX, SOCK_STREAM, struct sockaddr */
#include <sys/un.h> /* for struct sockaddr_un */

#include "apue.h"

#define CLI_PATH "/var/tmp/"
#define CLI_PERM S_IRWXU /* rwx for user only */

/*
 * Create a client endpoint and connect to a server.
 * Returns fd if all OK, <0 on error.
 */
int cli_conn(const char *name) {
    int rval;
    struct sockaddr_un un = {0};
    bool do_unlink = false;

    if (strlen(name) >= sizeof(un.sun_path)) {
        errno = ENAMETOOLONG;
        return (-1);
    }

    /* create a UNIX domain stream socket */
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        return (-1);
    }

    /* fill socket address structure with our address */
    un.sun_family = AF_UNIX;
    snprintf(un.sun_path, sizeof(un.sun_path), "%s%05ld", CLI_PATH,
             (long)getpid());
    printf("file is %s\n", un.sun_path);
    int len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);

    unlink(un.sun_path); /* in case it already exists */
    if (bind(fd, (struct sockaddr *)&un, len) < 0) {
        rval = -2;
        goto errout;
    }
    if (chmod(un.sun_path, CLI_PERM) < 0) {
        rval = -3;
        do_unlink = true;
        goto errout;
    }

    /* fill socket address structure with server's address */
    struct sockaddr_un sun = {0};
    sun.sun_family = AF_UNIX;
    snprintf(sun.sun_path, sizeof(sun.sun_path), "%s", name);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
    if (connect(fd, (struct sockaddr *)&sun, len) < 0) {
        rval = -4;
        do_unlink = true;
        goto errout;
    }
    return (fd);

errout:
    int err = errno;
    close(fd);
    if (do_unlink) {
        unlink(un.sun_path);
    }
    errno = err;
    return (rval);
}
