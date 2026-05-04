#include <stdio.h>  /* for L_ctermid */
#include <string.h> /* for strcpy */

static char ctermid_name[L_ctermid];

char *ctermid(char *str) {
    if (str == NULL) {
        str = ctermid_name;
    }
    return (strcpy(str, "/dev/tty")); /* strcpy() returns str */
}
