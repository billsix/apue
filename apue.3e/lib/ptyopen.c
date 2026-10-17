#include <stdio.h>  /* for snprintf */
#include <errno.h>  /* for errno */
#include <fcntl.h>  /* for open, posix_openpt, O_RDWR */
#include <stdlib.h> /* for grantpt, unlockpt, ptsname */
#include <unistd.h> /* for close */
#if defined(SOLARIS)
#include <stropts.h> /* for ioctl, I_FIND, I_PUSH (Solaris STREAMS) */
#endif

#include "apue.h"

int ptym_open(char *pts_name, int pts_namesz) {
    int fdm = posix_openpt(O_RDWR);
    if (fdm < 0) {
        return (-1);
    }
    if (grantpt(fdm) < 0) { /* grant access to slave */
        goto errout;
    }
    if (unlockpt(fdm) < 0) { /* clear slave's lock flag */
        goto errout;
    }
    const char *const ptr = ptsname(fdm); /* get slave's name */
    if (ptr == nullptr) {
        goto errout;
    }

    /*
     * Return name of slave.  Null terminate to handle
     * case where strlen(ptr) > pts_namesz.
     */
    snprintf(pts_name, pts_namesz, "%s", ptr);
    pts_name[pts_namesz - 1] = '\0';
    return (fdm); /* return fd of master */
errout:
    int err = errno;
    close(fdm);
    errno = err;
    return (-1);
}

int ptys_open(const char *pts_name) {
#if defined(SOLARIS)
    int err, setup;
#endif

    int fds = open(pts_name, O_RDWR);
    if (fds < 0) {
        return (-1);
    }

#if defined(SOLARIS)
    /*
     * Check if stream is already set up by autopush facility.
     */
    setup = ioctl(fds, I_FIND, "ldterm");
    if (setup < 0) {
        goto errout;
    }

    if (setup == 0) {
        if (ioctl(fds, I_PUSH, "ptem") < 0) {
            goto errout;
        }
        if (ioctl(fds, I_PUSH, "ldterm") < 0) {
            goto errout;
        }
        if (ioctl(fds, I_PUSH, "ttcompat") < 0) {
        errout:
            err = errno;
            close(fds);
            errno = err;
            return (-1);
        }
    }
#endif
    return (fds);
}
