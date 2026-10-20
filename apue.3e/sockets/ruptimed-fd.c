#include <stdlib.h> /* for exit, malloc */
#include <string.h> /* for strerror */
#include <unistd.h> /* for fork, dup2, close, execl, sysconf, gethostname, STDOUT_FILENO, STDERR_FILENO, _SC_HOST_NAME_MAX */
#include <errno.h>  /* for errno */
#include <syslog.h> /* for syslog, LOG_ERR */
#include <sys/types.h>  /* for pid_t */
#include <sys/wait.h>   /* for waitpid */
#include <sys/socket.h> /* for accept, struct sockaddr, socklen_t, SOCK_STREAM */
#include <netdb.h> /* for getaddrinfo, gai_strerror, struct addrinfo, AI_CANONNAME */

#include "apue.h"

#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

extern int initserver(int, const struct sockaddr *, socklen_t, int);

void serve(int sockfd) {
    set_cloexec(sockfd);
    while (true) {
        int clfd = accept(sockfd, nullptr, nullptr);
        if (clfd < 0) {
            syslog(LOG_ERR, "ruptimed: accept error: %s", strerror(errno));
            exit(1);
        }
        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "ruptimed: fork error: %s", strerror(errno));
            exit(1);
        } else if (pid == 0) { /* child */
            /*
             * The parent called daemonize ({Prog daemoninit}), so
             * STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO
             * are already open to /dev/null.  Thus, the call to
             * close doesn't need to be protected by checks that
             * clfd isn't already equal to one of these values.
             */
            if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
                dup2(clfd, STDERR_FILENO) != STDERR_FILENO) {
                syslog(LOG_ERR, "ruptimed: unexpected error");
                exit(1);
            }
            close(clfd);
            execl("/usr/bin/uptime", "uptime", (char *)0);
            syslog(LOG_ERR, "ruptimed: unexpected return from exec: %s",
                   strerror(errno));
        } else { /* parent */
            close(clfd);
            int status;
            waitpid(pid, &status, 0);
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
                            .ai_socktype = SOCK_STREAM};
    struct addrinfo *ailist;
    const int err = getaddrinfo(host, "ruptime", &hint, &ailist);
    if (err != 0) {
        syslog(LOG_ERR, "ruptimed: getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }
    for (struct addrinfo *aip = ailist; aip != nullptr; aip = aip->ai_next) {
        int sockfd =
            initserver(SOCK_STREAM, aip->ai_addr, aip->ai_addrlen, QLEN);
        if (sockfd >= 0) {
            serve(sockfd);
            exit(0);
        }
    }
    exit(1);
}
