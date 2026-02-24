#include <stdio.h> /* for printf */
#include <unistd.h> /* for getpid */
#include <stdlib.h> /* for exit */

int main(void) {
    printf("hello world from process ID %ld\n", (long)getpid());
    exit(0);
}
