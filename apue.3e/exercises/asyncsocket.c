#include <unistd.h>     /* for getpid */
#include <fcntl.h>      /* for fcntl, F_SETOWN */
#include <sys/socket.h> /* for socket-related constants */
#include <sys/ioctl.h>  /* for ioctl, FIOASYNC */
#if defined(BSD) || defined(MACOS) || defined(SOLARIS)
#include <sys/filio.h>  /* for FIOASYNC on BSD/MacOS/Solaris */
#endif

int setasync(int sockfd) {
    int n;

    if (fcntl(sockfd, F_SETOWN, getpid()) < 0) {
        return (-1);
    }
    n = 1;
    if (ioctl(sockfd, FIOASYNC, &n) < 0) {
        return (-1);
    }
    return (0);
}

int clrasync(int sockfd) {
    int n;

    n = 0;
    if (ioctl(sockfd, FIOASYNC, &n) < 0) {
        return (-1);
    }
    return (0);
}
