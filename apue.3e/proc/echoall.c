#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */

int main(int argc, char *argv[]) {
    extern char **environ;

    for (int i = 0; i < argc; i++) { /* echo all command-line args */
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    for (char **ptr = environ; *ptr != nullptr;
         ptr++) { /* and all env strings */
        printf("%s\n", *ptr);
    }

    exit(0);
}
