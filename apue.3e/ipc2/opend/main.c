#include <stdio.h>  /* for EOF */
#include <unistd.h> /* for getopt, opterr, optopt */
#include <syslog.h> /* for LOG_PID, LOG_USER */

#include "opend.h"

int debug, oflag, client_size, log_to_stderr;
char errmsg[MAXLINE];
char *pathname;
Client *client = nullptr;

int main(int argc, char *argv[]) {
    log_open("open.serv", LOG_PID, LOG_USER);

    opterr = 0; /* don't want getopt() writing to stderr */
    for (;;) {
        int c = getopt(argc, argv, "d");
        if (c == EOF) {
            break;
        }
        switch (c) {
            case 'd': /* debug */
                debug = log_to_stderr = 1;
                break;

            case '?':
                err_quit("unrecognized option: -%c", optopt);
        }
    }

    if (debug == 0) {
        daemonize("opend");
    }

    loop(); /* never returns */
}
