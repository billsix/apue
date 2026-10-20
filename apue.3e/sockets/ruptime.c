#include <stdlib.h>     /* for exit */
#include <unistd.h>     /* for write, STDOUT_FILENO */
#include <errno.h>      /* for errno */
#include <sys/socket.h> /* for recv, struct sockaddr, socklen_t, SOCK_STREAM */
#include <netdb.h>      /* for getaddrinfo, gai_strerror, struct addrinfo */

#include "apue.h"

#define BUFLEN 128

extern int connect_retry(int, int, int, const struct sockaddr *, socklen_t);

void print_uptime(int sockfd) {
    int n;
    char buf[BUFLEN];

    while (true) {
        n = recv(sockfd, buf, BUFLEN, 0);
        if (n <= 0) {
            break;
        }
        write(STDOUT_FILENO, buf, n);
    }
    if (n < 0) {
        err_sys("recv error");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        err_quit("usage: ruptime hostname");
    }
    struct addrinfo hint = {.ai_socktype = SOCK_STREAM};
    struct addrinfo *ailist;
    int err = getaddrinfo(argv[1], "ruptime", &hint, &ailist);
    if (err != 0) {
        err_quit("getaddrinfo error: %s", gai_strerror(err));
    }
    for (struct addrinfo *aip = ailist; aip != nullptr; aip = aip->ai_next) {
        int sockfd = connect_retry(aip->ai_family, SOCK_STREAM, 0, aip->ai_addr,
                                   aip->ai_addrlen);
        if (sockfd < 0) {
            err = errno;
        } else {
            print_uptime(sockfd);
            exit(0);
        }
    }
    err_exit(err, "can't connect to %s", argv[1]);
}
