#include <stdio.h>    /* for snprintf */
#include <stdlib.h>   /* for malloc, free */
#include <string.h>   /* for strlen, strcmp, strdup */
#include <unistd.h>   /* for isatty */
#include <limits.h>   /* for _POSIX_PATH_MAX */
#include <dirent.h>   /* for DIR, struct dirent, opendir, readdir, closedir */
#include <sys/stat.h> /* for struct stat, stat, fstat, S_ISDIR, S_ISCHR */

struct devdir {
    struct devdir *d_next;
    char *d_name;
};

static struct devdir *head;
static struct devdir *tail;
static char pathname[_POSIX_PATH_MAX + 1];

static void add(const char *dirname) {
    const int len = strlen(dirname);

    /*
     * Skip ., .., and /dev/fd.
     */
    if ((dirname[len - 1] == '.') &&
        (dirname[len - 2] == '/' ||
         (dirname[len - 2] == '.' && dirname[len - 3] == '/'))) {
        return;
    }
    if (strcmp(dirname, "/dev/fd") == 0) {
        return;
    }
    struct devdir *const ddp = malloc(sizeof(struct devdir));
    if (ddp == nullptr) {
        return;
    }

    ddp->d_name = strdup(dirname);
    if (ddp->d_name == nullptr) {
        free(ddp);
        return;
    }

    ddp->d_next = nullptr;
    if (tail == nullptr) {
        head = ddp;
        tail = ddp;
    } else {
        tail->d_next = ddp;
        tail = ddp;
    }
}

static void cleanup(void) {
    struct devdir *ddp = head;
    while (ddp != nullptr) {
        struct devdir *const nddp = ddp->d_next;
        free(ddp->d_name);
        free(ddp);
        ddp = nddp;
    }
    head = nullptr;
    tail = nullptr;
}

static char *searchdir(const char *dirname, const struct stat *fdstatp) {
    snprintf(pathname, sizeof(pathname), "%s/", dirname);
    DIR *const dp = opendir(dirname);
    if (dp == nullptr) {
        return (nullptr);
    }
    const int devlen = strlen(pathname);
    for (;;) {
        const struct dirent *const dirp = readdir(dp);
        if (dirp == nullptr) {
            break;
        }
        snprintf(pathname + devlen, sizeof(pathname) - devlen, "%s",
                 dirp->d_name);

        /*
         * Skip aliases.
         */
        if (strcmp(pathname, "/dev/stdin") == 0 ||
            strcmp(pathname, "/dev/stdout") == 0 ||
            strcmp(pathname, "/dev/stderr") == 0) {
            continue;
        }
        struct stat devstat;
        if (stat(pathname, &devstat) < 0) {
            continue;
        }
        if (S_ISDIR(devstat.st_mode)) {
            add(pathname);
            continue;
        }
        if (devstat.st_ino == fdstatp->st_ino &&
            devstat.st_dev == fdstatp->st_dev) { /* found a match */
            closedir(dp);
            return (pathname);
        }
    }

    closedir(dp);
    return (nullptr);
}

char *ttyname(int fd) {
    if (isatty(fd) == 0) {
        return (nullptr);
    }
    struct stat fdstat;
    if (fstat(fd, &fdstat) < 0) {
        return (nullptr);
    }
    if (S_ISCHR(fdstat.st_mode) == 0) {
        return (nullptr);
    }

    char *rval = searchdir("/dev", &fdstat);
    if (rval == nullptr) {
        for (struct devdir *ddp = head; ddp != nullptr; ddp = ddp->d_next) {
            rval = searchdir(ddp->d_name, &fdstat);
            if (rval != nullptr) {
                break;
            }
        }
    }

    cleanup();
    return (rval);
}
