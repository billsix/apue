/* Create a named stream pipe.  Called by server on initialization. */

#include <stdio.h>      /* for snprintf */
#include <string.h>     /* for strlen */
#include <unistd.h>     /* for unlink */
#include <sys/socket.h> /* for bind, AF_UNIX, struct sockaddr */
#include <sys/un.h>     /* for struct sockaddr_un */

#include "apue.h"

int /* returns 0 if all OK, -1 if error (with errno set) */
ns_pipe(const char *name, int fd[2]) {
    int len;
    struct sockaddr_un unix_addr = {0};

    if (fd_pipe(fd) < 0) { /* create unnamed stream pipe */
        return (-1);
    }

    unlink(name); /* remove the name, if it already exists */

    unix_addr.sun_family = AF_UNIX;
    snprintf(unix_addr.sun_path, sizeof(unix_addr.sun_path), "%s", name);
    len = strlen(unix_addr.sun_path) + sizeof(unix_addr.sun_family);

    return (bind(fd[0], (struct sockaddr *)&unix_addr, len));
    /* fd[0] has the name bound to it */
}
