#include <stdio.h> /* for printf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for getuid, getgid */


int main(void) {
    printf("uid = %d, gid = %d\n", getuid(), getgid());
    exit(0);
}
