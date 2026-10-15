#include <stdio.h>  /* for snprintf */
#include <string.h> /* for strlen, strncmp */

#define MAXSTRINGSZ 4096

static char envbuf[MAXSTRINGSZ];

extern char **environ;

char *getenv(const char *name) {
    const int len = strlen(name);
    for (int i = 0; environ[i] != nullptr; i++) {
        if ((strncmp(name, environ[i], len) == 0) && (environ[i][len] == '=')) {
            snprintf(envbuf, MAXSTRINGSZ, "%s", &environ[i][len + 1]);
            return (envbuf);
        }
    }
    return (nullptr);
}
