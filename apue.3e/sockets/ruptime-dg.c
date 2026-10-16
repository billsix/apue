#include <stdio.h>  /* for fprintf, stderr */
#include <stdlib.h> /* for exit */
#include <string.h> /* for strerror */
#include <unistd.h> /* for write, alarm, STDOUT_FILENO */
#include <signal.h> /* for sigaction, sigemptyset, struct sigaction, SIGALRM */
#include <errno.h>  /* for errno, EINTR */
#include <sys/socket.h> /* for socket, sendto, recvfrom, SOCK_DGRAM */
#include <netdb.h>      /* for getaddrinfo, gai_strerror, struct addrinfo */

#include "apue.h"

#define BUFLEN 128
#define TIMEOUT 20

void sigalrm([[maybe_unused]] int signo) {}

void print_uptime(int sockfd, const struct addrinfo *aip) {
    char buf[BUFLEN];
    buf[0] = 0;
    if (sendto(sockfd, buf, 1, 0, aip->ai_addr, aip->ai_addrlen) < 0) {
        err_sys("sendto error");
    }
    alarm(TIMEOUT);
    int n = recvfrom(sockfd, buf, BUFLEN, 0, nullptr, nullptr);
    if (n < 0) {
        if (errno != EINTR) {
            alarm(0);
        }
        err_sys("recv error");
    }
    alarm(0);
    write(STDOUT_FILENO, buf, n);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        err_quit("usage: ruptime hostname");
    }
    struct sigaction sa;
    sa.sa_handler = sigalrm;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, nullptr) < 0) {
        err_sys("sigaction error");
    }
    struct addrinfo hint = {.ai_socktype = SOCK_DGRAM};
    struct addrinfo *ailist;
    int err = getaddrinfo(argv[1], "ruptime", &hint, &ailist);
    if (err != 0) {
        err_quit("getaddrinfo error: %s", gai_strerror(err));
    }

    for (struct addrinfo *aip = ailist; aip != nullptr; aip = aip->ai_next) {
        int sockfd = socket(aip->ai_family, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            err = errno;
        } else {
            print_uptime(sockfd, aip);
            exit(0);
        }
    }

    fprintf(stderr, "can't contact %s: %s\n", argv[1], strerror(err));
    exit(1);
}
