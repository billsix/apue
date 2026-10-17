#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <shadow.h> /* for getspnam, struct spwd */

#include "apue.h"

int main(void) /* Linux/Solaris version */
{
    const struct spwd *const ptr = getspnam("sar");
    if (ptr == nullptr) {
        err_sys("getspnam error");
    }
    printf("sp_pwdp = %s\n", ptr->sp_pwdp == nullptr || ptr->sp_pwdp[0] == 0
                                 ? "(null)"
                                 : ptr->sp_pwdp);
    exit(0);
}
