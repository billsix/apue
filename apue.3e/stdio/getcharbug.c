#include <stdio.h> /* for getchar, putchar, EOF */

int main(void) {
    char c;

    while ((c = getchar()) != EOF) {
        putchar(c);
    }
}
