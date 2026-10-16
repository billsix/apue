#include <stdio.h> /* for snprintf */

#ifndef LINUX
extern char *sys_errlist[];
extern int sys_nerr;
#endif

char *strerror(int error) {
    static char mesg[30];

    if (error >= 0 && error <= sys_nerr) {
        return ((char *)sys_errlist[error]);
    }

    snprintf(mesg, sizeof(mesg), "Unknown error (%d)", error);
    return (mesg);
}
