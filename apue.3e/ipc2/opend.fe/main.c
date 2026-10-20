#include <stdlib.h> /* for exit */
#include <unistd.h> /* for read, STDIN_FILENO, STDOUT_FILENO */

#include "opend.h"

char errmsg[MAXLINE];
int oflag;
char *pathname;

int main(void) {
    char buf[MAXLINE];

    while (true) { /* read arg buffer from client, process request */
        int nread = read(STDIN_FILENO, buf, MAXLINE);
        if (nread < 0) {
            err_sys("read error on stream pipe");
        } else if (nread == 0) {
            break; /* client has closed the stream pipe */
        }
        handle_request(buf, nread, STDOUT_FILENO);
    }
    exit(0);
}
