#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <pwd.h>    /* for getpwnam, struct passwd */

#include "apue.h"

int main(void) /* FreeBSD/Mac OS X version */
{
    const struct passwd *ptr = getpwnam("sar");
    if (ptr == nullptr) {
        err_sys("getpwnam error");
    }
    printf("pw_passwd = %s\n",
           ptr->pw_passwd == nullptr || ptr->pw_passwd[0] == 0
               ? "(null)"
               : ptr->pw_passwd);
    exit(0);
}
