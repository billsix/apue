/*
 * Print server daemon.
 */
#include <stdio.h>  /* for printf, sprintf, snprintf */
#include <stdlib.h> /* for malloc, free, realloc, atol, atoi, exit */
#include <stddef.h> /* for offsetof */
#include <stdint.h> /* for int16_t, int32_t */
#include <string.h> /* for memcpy, strcpy, strncpy, strncmp, strlen, strerror, strsignal */
#include <strings.h> /* for strncasecmp */
#include <unistd.h> /* for read, write, close, unlink, lseek, sysconf, gethostname, setgid, setuid */
#include <fcntl.h>  /* for open, creat, O_CREAT, O_RDWR, O_RDONLY */
#include <ctype.h>  /* for isspace, isdigit */
#include <errno.h>  /* for errno, EIO */
#include <signal.h> /* for sigaction, sigemptyset, sigaddset, sigwait, sigset_t, SIG_IGN, SIGPIPE, SIGHUP, SIGTERM */
#include <time.h>   /* for nanosleep, struct timespec */
#include <pthread.h> /* for pthread_create, pthread_mutex_*, pthread_cond_*, pthread_sigmask, pthread_self, pthread_exit, pthread_cleanup_*, pthread_cancel */
#include <pwd.h>     /* for getpwnam, struct passwd */
#include <dirent.h>  /* for opendir, readdir, closedir, DIR, struct dirent */
#include <sys/stat.h>   /* for fstat, struct stat, S_IRUSR, S_IWUSR */
#include <sys/select.h> /* for select, fd_set, FD_ZERO, FD_SET, FD_ISSET */
#include <sys/uio.h>    /* for writev, struct iovec */
#include <sys/socket.h> /* for accept, SOCK_STREAM */
#include <arpa/inet.h>  /* for htonl, htons, ntohs, ntohl */

#include "apue.h"
#include "print.h"
#include "ipp.h"

/*
 * These are for the HTTP response from the printer.
 */
#define HTTP_INFO(x) ((x) >= 100 && (x) <= 199)
#define HTTP_SUCCESS(x) ((x) >= 200 && (x) <= 299)

/*
 * Describes a print job.
 */
struct job {
    struct job *next;    /* next in list */
    struct job *prev;    /* previous in list */
    int32_t jobid;       /* job ID */
    struct printreq req; /* copy of print request */
};

/*
 * Describes a thread processing a client request.
 */
struct worker_thread {
    struct worker_thread *next; /* next in list */
    struct worker_thread *prev; /* previous in list */
    pthread_t tid;              /* thread ID */
    int sockfd;                 /* socket */
};

/*
 * Needed for logging.
 */
int log_to_stderr = 0;

/*
 * Printer-related stuff.
 */
struct addrinfo *printer;
char *printer_name;
pthread_mutex_t configlock = PTHREAD_MUTEX_INITIALIZER;
int reread;

/*
 * Thread-related stuff.
 */
struct worker_thread *workers;
pthread_mutex_t workerlock = PTHREAD_MUTEX_INITIALIZER;
sigset_t mask;

/*
 * Job-related stuff.
 */
struct job *jobhead, *jobtail;
int jobfd;
int32_t nextjob;
pthread_mutex_t joblock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t jobwait = PTHREAD_COND_INITIALIZER;

/*
 * Function prototypes.
 */
void init_request(void);
void init_printer(void);
void update_jobno(void);
int32_t get_newjobno(void);
void add_job(const struct printreq *, int32_t);
void replace_job(struct job *);
void remove_job(struct job *);
void build_qonstart(void);
void *client_thread(void *);
void *printer_thread(void *);
void *signal_thread(void *);
ssize_t readmore(int, char **, int, int *);
int printer_status(int, const struct job *);
void add_worker(pthread_t, int);
void kill_workers(void);
void client_cleanup(void *);

/*
 * Main print server thread.  Accepts connect requests from
 * clients and spawns additional threads to service requests.
 *
 * LOCKING: none.
 */
int main(int argc, [[maybe_unused]] char *argv[]) {
    if (argc != 1) {
        err_quit("usage: printd");
    }
    daemonize("printd");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, nullptr) < 0) {
        log_sys("sigaction failed");
    }
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGTERM);
    int err = pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    if (err != 0) {
        log_sys("pthread_sigmask failed");
    }

    int n = sysconf(_SC_HOST_NAME_MAX);
    if (n < 0) { /* best guess */
        n = HOST_NAME_MAX;
    }
    char *host = malloc(n);
    if (host == nullptr) {
        log_sys("malloc error");
    }
    if (gethostname(host, n) < 0) {
        log_sys("gethostname error");
    }

    struct addrinfo *ailist;
    err = getaddrlist(host, "print", &ailist);
    if (err != 0) {
        log_quit("getaddrinfo error: %s", gai_strerror(err));
        exit(1);
    }
    fd_set rendezvous;
    FD_ZERO(&rendezvous);
    int maxfd = -1;
    for (struct addrinfo *aip = ailist; aip != nullptr; aip = aip->ai_next) {
        int sockfd =
            initserver(SOCK_STREAM, aip->ai_addr, aip->ai_addrlen, QLEN);
        if (sockfd >= 0) {
            FD_SET(sockfd, &rendezvous);
            if (sockfd > maxfd) {
                maxfd = sockfd;
            }
        }
    }
    if (maxfd == -1) {
        log_quit("service not enabled");
    }

    const struct passwd *pwdp = getpwnam(LPNAME);
    if (pwdp == nullptr) {
        log_sys("can't find user %s", LPNAME);
    }
    if (pwdp->pw_uid == 0) {
        log_quit("user %s is privileged", LPNAME);
    }
    if (setgid(pwdp->pw_gid) < 0 || setuid(pwdp->pw_uid) < 0) {
        log_sys("can't change IDs to user %s", LPNAME);
    }

    init_request();
    init_printer();

    pthread_t tid;
    err = pthread_create(&tid, nullptr, printer_thread, nullptr);
    if (err == 0) {
        err = pthread_create(&tid, nullptr, signal_thread, nullptr);
    }
    if (err != 0) {
        log_exit(err, "can't create thread");
    }
    build_qonstart();

    log_msg("daemon initialized");

    for (;;) {
        fd_set rset = rendezvous;
        if (select(maxfd + 1, &rset, nullptr, nullptr, nullptr) < 0) {
            log_sys("select failed");
        }
        for (int i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {
                /*
                 * Accept the connection and handle the request.
                 */
                int sockfd = accept(i, nullptr, nullptr);
                if (sockfd < 0) {
                    log_ret("accept failed");
                }
                pthread_create(&tid, nullptr, client_thread,
                               (void *)((long)sockfd));
            }
        }
    }
    exit(1);
}

/*
 * Initialize the job ID file.  Use a record lock to prevent
 * more than one printer daemon from running at a time.
 *
 * LOCKING: none, except for record-lock on job ID file.
 */
void init_request(void) {
    char name[FILENMSZ];
    snprintf(name, sizeof(name), "%s/%s", SPOOLDIR, JOBFILE);
    jobfd = open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (write_lock(jobfd, 0, SEEK_SET, 0) < 0) {
        log_quit("daemon already running");
    }

    /*
     * Reuse the name buffer for the job counter.
     */
    const int n = read(jobfd, name, FILENMSZ);
    if (n < 0) {
        log_sys("can't read job file");
    }
    if (n == 0) {
        nextjob = 1;
    } else {
        nextjob = atol(name);
    }
}

/*
 * Initialize printer information from configuration file.
 *
 * LOCKING: none.
 */
void init_printer(void) {
    printer = get_printaddr();
    if (printer == nullptr) {
        exit(1); /* message already logged */
    }
    printer_name = printer->ai_canonname;
    if (printer_name == nullptr) {
        printer_name = "printer";
    }
    log_msg("printer is %s", printer_name);
}

/*
 * Update the job ID file with the next job number.
 * Doesn't handle wrap-around of job number.
 *
 * LOCKING: none.
 */
void update_jobno(void) {
    char buf[32];

    if (lseek(jobfd, 0, SEEK_SET) == -1) {
        log_sys("can't seek in job file");
    }
    snprintf(buf, sizeof(buf), "%d", nextjob);
    if (write(jobfd, buf, strlen(buf)) < 0) {
        log_sys("can't update job file");
    }
}

/*
 * Get the next job number.
 *
 * LOCKING: acquires and releases joblock.
 */
int32_t get_newjobno(void) {
    pthread_mutex_lock(&joblock);
    const int32_t jobid = nextjob++;
    if (nextjob <= 0) {
        nextjob = 1;
    }
    pthread_mutex_unlock(&joblock);
    return (jobid);
}

/*
 * Add a new job to the list of pending jobs.  Then signal
 * the printer thread that a job is pending.
 *
 * LOCKING: acquires and releases joblock.
 */
void add_job(const struct printreq *reqp, int32_t jobid) {
    struct job *jp = malloc(sizeof(struct job));
    if (jp == nullptr) {
        log_sys("malloc failed");
    }
    memcpy(&jp->req, reqp, sizeof(struct printreq));
    jp->jobid = jobid;
    jp->next = nullptr;
    pthread_mutex_lock(&joblock);
    jp->prev = jobtail;
    if (jobtail == nullptr) {
        jobhead = jp;
    } else {
        jobtail->next = jp;
    }
    jobtail = jp;
    pthread_mutex_unlock(&joblock);
    pthread_cond_signal(&jobwait);
}

/*
 * Replace a job back on the head of the list.
 *
 * LOCKING: acquires and releases joblock.
 */
void replace_job(struct job *jp) {
    pthread_mutex_lock(&joblock);
    jp->prev = nullptr;
    jp->next = jobhead;
    if (jobhead == nullptr) {
        jobtail = jp;
    } else {
        jobhead->prev = jp;
    }
    jobhead = jp;
    pthread_mutex_unlock(&joblock);
}

/*
 * Remove a job from the list of pending jobs.
 *
 * LOCKING: caller must hold joblock.
 */
void remove_job(struct job *target) {
    if (target->next != nullptr) {
        target->next->prev = target->prev;
    } else {
        jobtail = target->prev;
    }
    if (target->prev != nullptr) {
        target->prev->next = target->next;
    } else {
        jobhead = target->next;
    }
}

/*
 * Check the spool directory for pending jobs on start-up.
 *
 * LOCKING: none.
 */
void build_qonstart(void) {
    char dname[FILENMSZ];
    snprintf(dname, sizeof(dname), "%s/%s", SPOOLDIR, REQDIR);
    DIR *dirp = opendir(dname);
    if (dirp == nullptr) {
        return;
    }
    struct dirent *entp;
    while ((entp = readdir(dirp)) != nullptr) {
        /*
         * Skip "." and ".."
         */
        if (strcmp(entp->d_name, ".") == 0 || strcmp(entp->d_name, "..") == 0) {
            continue;
        }

        /*
         * Read the request structure.
         */
        char fname[FILENMSZ];
        snprintf(fname, sizeof(fname), "%s/%s/%s", SPOOLDIR, REQDIR,
                 entp->d_name);
        int fd = open(fname, O_RDONLY);
        if (fd < 0) {
            continue;
        }
        struct printreq req;
        int nr = read(fd, &req, sizeof(struct printreq));
        if (nr != sizeof(struct printreq)) {
            int err;
            if (nr < 0) {
                err = errno;
            } else {
                err = EIO;
            }
            close(fd);
            log_msg("build_qonstart: can't read %s: %s", fname, strerror(err));
            unlink(fname);
            snprintf(fname, sizeof(fname), "%s/%s/%s", SPOOLDIR, DATADIR,
                     entp->d_name);
            unlink(fname);
            continue;
        }
        const int32_t jobid = atol(entp->d_name);
        log_msg("adding job %d to queue", jobid);
        add_job(&req, jobid);
    }
    closedir(dirp);
}

/*
 * Accept a print job from a client.
 *
 * LOCKING: none.
 */
void *client_thread(void *arg) {
    struct printreq req;
    struct printresp res;

    const pthread_t tid = pthread_self();
    pthread_cleanup_push(client_cleanup, (void *)((long)tid));
    const int sockfd = (long)arg;
    add_worker(tid, sockfd);

    /*
     * Read the request header.
     */
    const int n = treadn(sockfd, &req, sizeof(struct printreq), 10);
    if (n != sizeof(struct printreq)) {
        res.jobid = 0;
        if (n < 0) {
            res.retcode = htonl(errno);
        } else {
            res.retcode = htonl(EIO);
        }
        snprintf(res.msg, sizeof(res.msg), "%s", strerror(res.retcode));
        writen(sockfd, &res, sizeof(struct printresp));
        pthread_exit((void *)1);
    }
    req.size = ntohl(req.size);
    req.flags = ntohl(req.flags);

    /*
     * Create the data file.
     */
    const int32_t jobid = get_newjobno();
    char name[FILENMSZ];
    snprintf(name, sizeof(name), "%s/%s/%d", SPOOLDIR, DATADIR, jobid);
    int fd = creat(name, FILEPERM);
    if (fd < 0) {
        res.jobid = 0;
        res.retcode = htonl(errno);
        log_msg("client_thread: can't create %s: %s", name,
                strerror(res.retcode));
        snprintf(res.msg, sizeof(res.msg), "%s", strerror(res.retcode));
        writen(sockfd, &res, sizeof(struct printresp));
        pthread_exit((void *)1);
    }

    /*
     * Read the file and store it in the spool directory.
     * Try to figure out if the file is a PostScript file
     * or a plain text file.
     */
    bool first = true;
    char buf[IOBUFSZ];
    int nr;
    while ((nr = tread(sockfd, buf, IOBUFSZ, 20)) > 0) {
        if (first) {
            first = false;
            if (strncmp(buf, "%!PS", 4) != 0) {
                req.flags |= PR_TEXT;
            }
        }
        int nw = write(fd, buf, nr);
        if (nw != nr) {
            res.jobid = 0;
            if (nw < 0) {
                res.retcode = htonl(errno);
            } else {
                res.retcode = htonl(EIO);
            }
            log_msg("client_thread: can't write %s: %s", name,
                    strerror(res.retcode));
            close(fd);
            snprintf(res.msg, sizeof(res.msg), "%s", strerror(res.retcode));
            writen(sockfd, &res, sizeof(struct printresp));
            unlink(name);
            pthread_exit((void *)1);
        }
    }
    close(fd);

    /*
     * Create the control file.  Then write the
     * print request information to the control
     * file.
     */
    snprintf(name, sizeof(name), "%s/%s/%d", SPOOLDIR, REQDIR, jobid);
    fd = creat(name, FILEPERM);
    if (fd < 0) {
        res.jobid = 0;
        res.retcode = htonl(errno);
        log_msg("client_thread: can't create %s: %s", name,
                strerror(res.retcode));
        snprintf(res.msg, sizeof(res.msg), "%s", strerror(res.retcode));
        writen(sockfd, &res, sizeof(struct printresp));
        snprintf(name, sizeof(name), "%s/%s/%d", SPOOLDIR, DATADIR, jobid);
        unlink(name);
        pthread_exit((void *)1);
    }
    int nw = write(fd, &req, sizeof(struct printreq));
    if (nw != sizeof(struct printreq)) {
        res.jobid = 0;
        if (nw < 0) {
            res.retcode = htonl(errno);
        } else {
            res.retcode = htonl(EIO);
        }
        log_msg("client_thread: can't write %s: %s", name,
                strerror(res.retcode));
        close(fd);
        snprintf(res.msg, sizeof(res.msg), "%s", strerror(res.retcode));
        writen(sockfd, &res, sizeof(struct printresp));
        unlink(name);
        snprintf(name, sizeof(name), "%s/%s/%d", SPOOLDIR, DATADIR, jobid);
        unlink(name);
        pthread_exit((void *)1);
    }
    close(fd);

    /*
     * Send response to client.
     */
    res.retcode = 0;
    res.jobid = htonl(jobid);
    snprintf(res.msg, sizeof(res.msg), "request ID %d", jobid);
    writen(sockfd, &res, sizeof(struct printresp));

    /*
     * Notify the printer thread, clean up, and exit.
     */
    log_msg("adding job %d to queue", jobid);
    add_job(&req, jobid);
    pthread_cleanup_pop(1);
    return ((void *)0);
}

/*
 * Add a worker to the list of worker threads.
 *
 * LOCKING: acquires and releases workerlock.
 */
void add_worker(pthread_t tid, int sockfd) {
    struct worker_thread *wtp = malloc(sizeof(struct worker_thread));
    if (wtp == nullptr) {
        log_ret("add_worker: can't malloc");
        pthread_exit((void *)1);
    }
    wtp->tid = tid;
    wtp->sockfd = sockfd;
    pthread_mutex_lock(&workerlock);
    wtp->prev = nullptr;
    wtp->next = workers;
    if (workers == nullptr) {
        workers = wtp;
    } else {
        workers->prev = wtp;
    }
    pthread_mutex_unlock(&workerlock);
}

/*
 * Cancel (kill) all outstanding workers.
 *
 * LOCKING: acquires and releases workerlock.
 */
void kill_workers(void) {
    pthread_mutex_lock(&workerlock);
    for (struct worker_thread *wtp = workers; wtp != nullptr; wtp = wtp->next) {
        pthread_cancel(wtp->tid);
    }
    pthread_mutex_unlock(&workerlock);
}

/*
 * Cancellation routine for the worker thread.
 *
 * LOCKING: acquires and releases workerlock.
 */
void client_cleanup(void *arg) {
    const pthread_t tid = (pthread_t)((long)arg);
    pthread_mutex_lock(&workerlock);
    struct worker_thread *wtp;
    for (wtp = workers; wtp != nullptr; wtp = wtp->next) {
        if (wtp->tid == tid) {
            if (wtp->next != nullptr) {
                wtp->next->prev = wtp->prev;
            }
            if (wtp->prev != nullptr) {
                wtp->prev->next = wtp->next;
            } else {
                workers = wtp->next;
            }
            break;
        }
    }
    pthread_mutex_unlock(&workerlock);
    if (wtp != nullptr) {
        close(wtp->sockfd);
        free(wtp);
    }
}

/*
 * Deal with signals.
 *
 * LOCKING: acquires and releases configlock.
 */
void *signal_thread([[maybe_unused]] void *arg) {
    for (;;) {
        int signo;
        const int err = sigwait(&mask, &signo);
        if (err != 0) {
            log_quit("sigwait failed: %s", strerror(err));
        }
        switch (signo) {
            case SIGHUP:
                /*
                 * Schedule to re-read the configuration file.
                 */
                pthread_mutex_lock(&configlock);
                reread = 1;
                pthread_mutex_unlock(&configlock);
                break;

            case SIGTERM:
                kill_workers();
                log_msg("terminate with signal %s", strsignal(signo));
                exit(0);

            default:
                kill_workers();
                log_quit("unexpected signal %d", signo);
        }
    }
}

/*
 * Add an option to the IPP header.
 *
 * LOCKING: none.
 */
char *add_option(char *cp, int tag, const char *optname, const char *optval) {
    union {
        int16_t s;
        char c[2];
    } u;

    *cp++ = tag;
    int n = strlen(optname);
    u.s = htons(n);
    *cp++ = u.c[0];
    *cp++ = u.c[1];
    strcpy(cp, optname);
    cp += n;
    n = strlen(optval);
    u.s = htons(n);
    *cp++ = u.c[0];
    *cp++ = u.c[1];
    strcpy(cp, optval);
    return (cp + n);
}

/*
 * Single thread to communicate with the printer.
 *
 * LOCKING: acquires and releases joblock and configlock.
 */
void *printer_thread([[maybe_unused]] void *arg) {
    struct job *jp;
    int hlen;
    int ilen;
    int sockfd;
    int fd;
    int nr;
    int nw;
    int extra;
    char *icp;
    char *hcp;
    const char *p;
    struct ipp_hdr *hp;
    struct stat sbuf;
    struct iovec iov[2];
    char name[FILENMSZ];
    char hbuf[HBUFSZ];
    char ibuf[IBUFSZ];
    char buf[IOBUFSZ];
    char str[64];
    struct timespec ts = {60, 0}; /* 1 minute */

    for (;;) {
        /*
         * Get a job to print.
         */
        pthread_mutex_lock(&joblock);
        while (jobhead == nullptr) {
            log_msg("printer_thread: waiting...");
            pthread_cond_wait(&jobwait, &joblock);
        }
        remove_job(jp = jobhead);
        log_msg("printer_thread: picked up job %d", jp->jobid);
        pthread_mutex_unlock(&joblock);
        update_jobno();

        /*
         * Check for a change in the config file.
         */
        pthread_mutex_lock(&configlock);
        if (reread) {
            freeaddrinfo(printer);
            printer = nullptr;
            printer_name = nullptr;
            reread = 0;
            pthread_mutex_unlock(&configlock);
            init_printer();
        } else {
            pthread_mutex_unlock(&configlock);
        }

        /*
         * Send job to printer.
         */
        snprintf(name, sizeof(name), "%s/%s/%d", SPOOLDIR, DATADIR, jp->jobid);
        if ((fd = open(name, O_RDONLY)) < 0) {
            log_msg("job %d canceled - can't open %s: %s", jp->jobid, name,
                    strerror(errno));
            free(jp);
            continue;
        }
        if (fstat(fd, &sbuf) < 0) {
            log_msg("job %d canceled - can't fstat %s: %s", jp->jobid, name,
                    strerror(errno));
            free(jp);
            close(fd);
            continue;
        }
        if ((sockfd = connect_retry(AF_INET, SOCK_STREAM, 0, printer->ai_addr,
                                    printer->ai_addrlen)) < 0) {
            log_msg("job %d deferred - can't contact printer: %s", jp->jobid,
                    strerror(errno));
            goto defer;
        }

        /*
         * Set up the IPP header.
         */
        icp = ibuf;
        hp = (struct ipp_hdr *)icp;
        hp->major_version = 1;
        hp->minor_version = 1;
        hp->operation = htons(OP_PRINT_JOB);
        hp->request_id = htonl(jp->jobid);
        icp += offsetof(struct ipp_hdr, attr_group);
        *icp++ = TAG_OPERATION_ATTR;
        icp = add_option(icp, TAG_CHARSET, "attributes-charset", "utf-8");
        icp = add_option(icp, TAG_NATULANG, "attributes-natural-language",
                         "en-us");
        snprintf(str, sizeof(str), "http://%s/ipp", printer_name);
        icp = add_option(icp, TAG_URI, "printer-uri", str);
        icp = add_option(icp, TAG_NAMEWOLANG, "requesting-user-name",
                         jp->req.usernm);
        icp = add_option(icp, TAG_NAMEWOLANG, "job-name", jp->req.jobnm);
        if (jp->req.flags & PR_TEXT) {
            p = "text/plain";
            extra = 1;
        } else {
            p = "application/postscript";
            extra = 0;
        }
        icp = add_option(icp, TAG_MIMETYPE, "document-format", p);
        *icp++ = TAG_END_OF_ATTR;
        ilen = icp - ibuf;

        /*
         * Set up the HTTP header.
         */
        hcp = hbuf;
        sprintf(hcp, "POST /ipp HTTP/1.1\r\n");
        hcp += strlen(hcp);
        sprintf(hcp, "Content-Length: %ld\r\n",
                (long)sbuf.st_size + ilen + extra);
        hcp += strlen(hcp);
        strcpy(hcp, "Content-Type: application/ipp\r\n");
        hcp += strlen(hcp);
        sprintf(hcp, "Host: %s:%d\r\n", printer_name, IPP_PORT);
        hcp += strlen(hcp);
        *hcp++ = '\r';
        *hcp++ = '\n';
        hlen = hcp - hbuf;

        /*
         * Write the headers first.  Then send the file.
         */
        iov[0].iov_base = hbuf;
        iov[0].iov_len = hlen;
        iov[1].iov_base = ibuf;
        iov[1].iov_len = ilen;
        if (writev(sockfd, iov, 2) != hlen + ilen) {
            log_ret("can't write to printer");
            goto defer;
        }

        if (jp->req.flags & PR_TEXT) {
            /*
             * Hack: allow PostScript to be printed as plain text.
             */
            if (write(sockfd, "\b", 1) != 1) {
                log_ret("can't write to printer");
                goto defer;
            }
        }

        while ((nr = read(fd, buf, IOBUFSZ)) > 0) {
            if ((nw = writen(sockfd, buf, nr)) != nr) {
                if (nw < 0) {
                    log_ret("can't write to printer");
                } else {
                    log_msg("short write (%d/%d) to printer", nw, nr);
                }
                goto defer;
            }
        }
        if (nr < 0) {
            log_ret("can't read %s", name);
            goto defer;
        }

        /*
         * Read the response from the printer.
         */
        if (printer_status(sockfd, jp)) {
            unlink(name);
            snprintf(name, sizeof(name), "%s/%s/%d", SPOOLDIR, REQDIR,
                     jp->jobid);
            unlink(name);
            free(jp);
            jp = nullptr;
        }
    defer:
        close(fd);
        if (sockfd >= 0) {
            close(sockfd);
        }
        if (jp != nullptr) {
            replace_job(jp);
            nanosleep(&ts, nullptr);
        }
    }
}

/*
 * Read data from the printer, possibly increasing the buffer.
 * Returns offset of end of data in buffer or -1 on failure.
 *
 * LOCKING: none.
 */
ssize_t readmore(int sockfd, char **bpp, int off, int *bszp) {
    char *bp = *bpp;
    int bsz = *bszp;
    if (off >= bsz) {
        bsz += IOBUFSZ;
        bp = realloc(*bpp, bsz);
        if (bp == nullptr) {
            log_sys("readmore: can't allocate bigger read buffer");
        }
        *bszp = bsz;
        *bpp = bp;
    }
    const ssize_t nr = tread(sockfd, &bp[off], bsz - off, 1);
    if (nr > 0) {
        return (off + nr);
    } else {
        return (-1);
    }
}

/*
 * Read and parse the response from the printer.  Return 1
 * if the request was successful, and 0 otherwise.
 *
 * LOCKING: none.
 */
int printer_status(int sfd, const struct job *jp) {
    int i;
    int code;
    int len;
    bool found;
    int datsz;
    int32_t jobid;
    ssize_t nr;
    char *cp;
    char *statcode;
    char *reason;
    char *contentlen;
    struct ipp_hdr *hp;

    /*
     * Read the HTTP header followed by the IPP response header.
     * They can be returned in multiple read attempts.  Use the
     * Content-Length specifier to determine how much to read.
     */
    bool success = false;
    int bufsz = IOBUFSZ;
    char *bp = malloc(IOBUFSZ);
    if (bp == nullptr) {
        log_sys("printer_status: can't allocate read buffer");
    }

    while ((nr = tread(sfd, bp, bufsz, 5)) > 0) {
        /*
         * Find the status.  Response starts with "HTTP/x.y"
         * so we can skip the first 8 characters.
         */
        cp = bp + 8;
        datsz = nr;
        while (isspace((int)*cp)) {
            cp++;
        }
        statcode = cp;
        while (isdigit((int)*cp)) {
            cp++;
        }
        if (cp == statcode) { /* Bad format; log it and move on */
            log_msg(bp);
        } else {
            *cp++ = '\0';
            reason = cp;
            while (*cp != '\r' && *cp != '\n') {
                cp++;
            }
            *cp = '\0';
            code = atoi(statcode);
            if (HTTP_INFO(code)) {
                continue;
            }
            if (!HTTP_SUCCESS(code)) { /* probable error: log it */
                bp[datsz] = '\0';
                log_msg("error: %s", reason);
                break;
            }

            /*
             * HTTP request was okay, but still need to check
             * IPP status.  Search for the Content-Length.
             */
            i = cp - bp;
            for (;;) {
                while (*cp != 'C' && *cp != 'c' && i < datsz) {
                    cp++;
                    i++;
                }
                if (i >= datsz) { /* get more header */
                    if ((nr = readmore(sfd, &bp, i, &bufsz)) < 0) {
                        goto out;
                    } else {
                        cp = &bp[i];
                        datsz += nr;
                    }
                }

                if (strncasecmp(cp, "Content-Length:", 15) == 0) {
                    cp += 15;
                    while (isspace((int)*cp)) {
                        cp++;
                    }
                    contentlen = cp;
                    while (isdigit((int)*cp)) {
                        cp++;
                    }
                    *cp++ = '\0';
                    i = cp - bp;
                    len = atoi(contentlen);
                    break;
                } else {
                    cp++;
                    i++;
                }
            }
            if (i >= datsz) { /* get more header */
                if ((nr = readmore(sfd, &bp, i, &bufsz)) < 0) {
                    goto out;
                } else {
                    cp = &bp[i];
                    datsz += nr;
                }
            }

            found = false;
            while (!found) { /* look for end of HTTP header */
                while (i < datsz - 2) {
                    if (*cp == '\n' && *(cp + 1) == '\r' && *(cp + 2) == '\n') {
                        found = true;
                        cp += 3;
                        i += 3;
                        break;
                    }
                    cp++;
                    i++;
                }
                if (i >= datsz) { /* get more header */
                    if ((nr = readmore(sfd, &bp, i, &bufsz)) < 0) {
                        goto out;
                    } else {
                        cp = &bp[i];
                        datsz += nr;
                    }
                }
            }

            if (datsz - i < len) { /* get more header */
                if ((nr = readmore(sfd, &bp, i, &bufsz)) < 0) {
                    goto out;
                } else {
                    cp = &bp[i];
                    datsz += nr;
                }
            }

            hp = (struct ipp_hdr *)cp;
            i = ntohs(hp->status);
            jobid = ntohl(hp->request_id);

            if (jobid != jp->jobid) {
                /*
                 * Different jobs.  Ignore it.
                 */
                log_msg("jobid %d status code %d", jobid, i);
                break;
            }

            if (STATCLASS_OK(i)) {
                success = true;
            }
            break;
        }
    }

out:
    free(bp);
    if (nr < 0) {
        log_msg("jobid %d: error reading printer response: %s", jobid,
                strerror(errno));
    }
    return (success);
}
