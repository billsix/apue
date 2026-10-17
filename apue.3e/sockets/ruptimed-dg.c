#include <stdio.h>  /* for FILE, sprintf, fgets, popen, pclose */
#include <stdlib.h> /* for exit, malloc */
#include <string.h> /* for strlen, strerror */
#include <unistd.h> /* for sysconf, gethostname, _SC_HOST_NAME_MAX */
#include <errno.h>  /* for errno */
#include <syslog.h> /* for syslog, LOG_ERR */
#include <sys/socket.h> /* for recvfrom, sendto, struct sockaddr, socklen_t, SOCK_DGRAM */
#include <netdb.h> /* for getaddrinfo, gai_strerror, struct addrinfo, AI_CANONNAME */

#include "apue.h"

#define BUFLEN 128
#define MAXADDRLEN 256

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

extern int initserver(int, const struct sockaddr *, socklen_t, int);

void serve(int sockfd) {
    char buf[BUFLEN];
    char abuf[MAXADDRLEN];
    struct sockaddr *const addr = (struct sockaddr *)abuf;

    set_cloexec(sockfd);
    for (;;) {
        socklen_t alen = MAXADDRLEN;
        int n = recvfrom(sockfd, buf, BUFLEN, 0, addr, &alen);
        if (n < 0) {
            syslog(LOG_ERR, "ruptimed: recvfrom error: %s", strerror(errno));
            exit(1);
        }
        FILE *const fp = popen("/usr/bin/uptime", "r");
        if (fp == nullptr) {
            snprintf(buf, sizeof(buf), "error: %s\n", strerror(errno));
            sendto(sockfd, buf, strlen(buf), 0, addr, alen);
        } else {
            if (fgets(buf, BUFLEN, fp) != nullptr) {
                sendto(sockfd, buf, strlen(buf), 0, addr, alen);
            }
            pclose(fp);
        }
    }
}

int main(int argc, [[maybe_unused]] char *argv[]) {
    if (argc != 1) {
        err_quit("usage: ruptimed");
    }
    int n = sysconf(_SC_HOST_NAME_MAX);
    if (n < 0) {
        n = HOST_NAME_MAX; /* best guess */
    }
    char *const host = malloc(n);
    if (host == nullptr) {
        err_sys("malloc error");
    }
    if (gethostname(host, n) < 0) {
        err_sys("gethostname error");
    }
    daemonize("ruptimed");
    struct addrinfo hint = {.ai_flags = AI_CANONNAME,
                            .ai_socktype = SOCK_DGRAM};
    struct addrinfo *ailist;
    const int err = getaddrinfo(host, "ruptime", &hint, &ailist);
    if (err != 0) {
        syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }
    for (struct addrinfo *aip = ailist; aip != nullptr; aip = aip->ai_next) {
        int sockfd = initserver(SOCK_DGRAM, aip->ai_addr, aip->ai_addrlen, 0);
        if (sockfd >= 0) {
            serve(sockfd);
            exit(0);
        }
    }
    exit(1);
}
