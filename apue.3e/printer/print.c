/*
 * The client command for printing documents.  Opens the file
 * and sends it to the printer spooling daemon.  Usage:
 * 	print [-t] filename
 */
#include <stdio.h>      /* for printf */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for strcpy, strncpy, strncat, strlen */
#include <unistd.h>     /* for getopt, optind, read, geteuid */
#include <fcntl.h>      /* for open, O_RDONLY */
#include <errno.h>      /* for errno */
#include <pwd.h>        /* for getpwuid, struct passwd */
#include <sys/stat.h>   /* for fstat, struct stat, S_ISREG */
#include <sys/socket.h> /* for AF_INET, SOCK_STREAM */
#include <arpa/inet.h>  /* for htonl, ntohl */
#include <netdb.h>      /* for struct addrinfo, gai_strerror */

#include "apue.h"
#include "print.h"

/*
 * Needed for logging funtions.
 */
int log_to_stderr = 1;

void submit_file(int, int, const char *, size_t, bool);

int main(int argc, char *argv[]) {
    int err = 0;
    bool text = false;
    while (true) {
        int c = getopt(argc, argv, "t");
        if (c == -1) {
            break;
        }
        switch (c) {
            case 't':
                text = true;
                break;

            case '?':
                err = 1;
                break;
        }
    }
    if (err || (optind != argc - 1)) {
        err_quit("usage: print [-t] filename");
    }
    const int fd = open(argv[optind], O_RDONLY);
    if (fd < 0) {
        err_sys("print: can't open %s", argv[optind]);
    }
    struct stat sbuf;
    if (fstat(fd, &sbuf) < 0) {
        err_sys("print: can't stat %s", argv[optind]);
    }
    if (!S_ISREG(sbuf.st_mode)) {
        err_quit("print: %s must be a regular file", argv[optind]);
    }

    /*
     * Get the hostname of the host acting as the print server.
     */
    const char *const host = get_printserver();
    if (host == nullptr) {
        err_quit("print: no print server defined");
    }
    struct addrinfo *ailist;
    err = getaddrlist(host, "print", &ailist);
    if (err != 0) {
        err_quit("print: getaddrinfo error: %s", gai_strerror(err));
    }

    for (struct addrinfo *aip = ailist; aip != nullptr; aip = aip->ai_next) {
        int sfd = connect_retry(AF_INET, SOCK_STREAM, 0, aip->ai_addr,
                                aip->ai_addrlen);
        if (sfd < 0) {
            err = errno;
        } else {
            submit_file(fd, sfd, argv[optind], sbuf.st_size, text);
            exit(0);
        }
    }
    err_exit(err, "print: can't contact %s", host);
}

/*
 * Send a file to the printer daemon.
 */
void submit_file(int fd, int sockfd, const char *fname, size_t nbytes,
                 bool text) {
    struct printreq req;

    /*
     * First build the header.
     */
    const struct passwd *const pwd = getpwuid(geteuid());
    if (pwd == nullptr) {
        strcpy(req.usernm, "unknown");
    } else {
        strncpy(req.usernm, pwd->pw_name, USERNM_MAX - 1);
        req.usernm[USERNM_MAX - 1] = '\0';
    }
    req.size = htonl(nbytes);

    if (text) {
        req.flags = htonl(PR_TEXT);
    } else {
        req.flags = 0;
    }

    const int len = strlen(fname);
    if (len >= JOBNM_MAX) {
        /*
         * Truncate the filename (+-5 accounts for the leading
         * four characters and the terminating null).
         */
        snprintf(req.jobnm, sizeof(req.jobnm), "... %s",
                 &fname[len - JOBNM_MAX + 5]);
    } else {
        snprintf(req.jobnm, sizeof(req.jobnm), "%s", fname);
    }

    /*
     * Send the header to the server.
     */
    int nw = writen(sockfd, &req, sizeof(struct printreq));
    if (nw != sizeof(struct printreq)) {
        if (nw < 0) {
            err_sys("can't write to print server");
        } else {
            err_quit("short write (%d/%d) to print server", nw,
                     sizeof(struct printreq));
        }
    }

    /*
     * Now send the file.
     */
    char buf[IOBUFSZ];
    int nr;
    while (true) {
        nr = read(fd, buf, IOBUFSZ);
        if (nr == 0) {
            break;
        }
        nw = writen(sockfd, buf, nr);
        if (nw != nr) {
            if (nw < 0) {
                err_sys("can't write to print server");
            } else {
                err_quit("short write (%d/%d) to print server", nw, nr);
            }
        }
    }

    /*
     * Read the response.
     */
    struct printresp res;
    nr = readn(sockfd, &res, sizeof(struct printresp));
    if (nr != sizeof(struct printresp)) {
        err_sys("can't read response from server");
    }
    if (res.retcode != 0) {
        printf("rejected: %s\n", res.msg);
        exit(1);
    } else {
        printf("job ID %ld\n", (long)ntohl(res.jobid));
    }
}
