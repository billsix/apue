#include <unistd.h>     /* for read, close */
#include <sys/select.h> /* for select, fd_set, FD_ZERO, FD_SET, FD_ISSET, FD_CLR */

#include "opend.h"

void loop(void) {
    int i;
    int maxfd;
    int maxi;
    int clifd;
    fd_set rset;
    fd_set allset;

    FD_ZERO(&allset);

    /* obtain fd to listen for client requests on */
    const int listenfd = serv_listen(CS_OPEN);
    if (listenfd < 0) {
        log_sys("serv_listen error");
    }
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;

    while (true) {
        rset = allset; /* rset gets modified each time around */
        const int n = select(maxfd + 1, &rset, nullptr, nullptr, nullptr);
        if (n < 0) {
            log_sys("select error");
        }

        if (FD_ISSET(listenfd, &rset)) {
            /* accept new client request */
            uid_t uid;
            clifd = serv_accept(listenfd, &uid);
            if (clifd < 0) {
                log_sys("serv_accept error: %d", clifd);
            }
            i = client_add(clifd, uid);
            FD_SET(clifd, &allset);
            if (clifd > maxfd) {
                maxfd = clifd; /* max fd for select() */
            }
            if (i > maxi) {
                maxi = i; /* max index in client[] array */
            }
            log_msg("new connection: uid %d, fd %d", uid, clifd);
            continue;
        }

        for (i = 0; i <= maxi; i++) { /* go through client[] array */
            clifd = client[i].fd;
            if (clifd < 0) {
                continue;
            }
            if (FD_ISSET(clifd, &rset)) {
                /* read argument buffer from client */
                char buf[MAXLINE];
                const int nread = read(clifd, buf, MAXLINE);
                if (nread < 0) {
                    log_sys("read error on fd %d", clifd);
                } else if (nread == 0) {
                    log_msg("closed: uid %d, fd %d", client[i].uid, clifd);
                    client_del(clifd); /* client has closed cxn */
                    FD_CLR(clifd, &allset);
                    close(clifd);
                } else { /* process client's request */
                    handle_request(buf, nread, clifd, client[i].uid);
                }
            }
        }
    }
}
