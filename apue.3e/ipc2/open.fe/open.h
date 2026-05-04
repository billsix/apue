#include <sys/types.h> /* needed for apue.h */
#include <errno.h>     /* for errno */

#include "apue.h"

#define CL_OPEN "open" /* client's request for server */

int csopen(char *, int);
