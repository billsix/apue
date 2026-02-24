#include "apue2.h"
#include "apue.h"

int main(void) {
    printf("real uid = %d, effective uid = %d\n", getuid(), geteuid());
    exit(0);
}
