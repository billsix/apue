#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for STDIN_FILENO */
#include <termios.h>   /* for struct termios, tcgetattr, tcsetattr, TCSANOW, CSIZE, CS5, CS6, CS7, CS8 */

#include "apue.h"

int main(void) {
    struct termios term;

    if (tcgetattr(STDIN_FILENO, &term) < 0) {
        err_sys("tcgetattr error");
    }

    switch (term.c_cflag & CSIZE) {
        case CS5:
            printf("5 bits/byte\n");
            break;
        case CS6:
            printf("6 bits/byte\n");
            break;
        case CS7:
            printf("7 bits/byte\n");
            break;
        case CS8:
            printf("8 bits/byte\n");
            break;
        default:
            printf("unknown bits/byte\n");
    }

    term.c_cflag &= ~CSIZE; /* zero out the bits */
    term.c_cflag |= CS8;    /* set 8 bits/byte */
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0) {
        err_sys("tcsetattr error");
    }

    exit(0);
}
