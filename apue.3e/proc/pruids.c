#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for getuid, geteuid */

int main(void) {
    printf("real uid = %d, effective uid = %d\n", getuid(), geteuid());
    exit(0);
}
