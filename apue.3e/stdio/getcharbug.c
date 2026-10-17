#include <stdio.h> /* for getchar, putchar, EOF */

int main(void) {
    char c;

    /*
     * Kept deliberately nested and char-typed: the folded "assignment in the
     * test" with a char IS the bug this program demonstrates — c truncates
     * getchar()'s int return, so the `c != EOF` test misbehaves (an infinite
     * loop on a plain-unsigned-char system, a premature stop on a 0xFF byte
     * where char is signed). Do NOT de-nest this loop or widen c to int; either
     * "fix" erases the lesson. Exempt from the flatten-nested-expressions
     * convention (tasks/reference/teaching-flatten-expressions.md).
     */
    while ((c = getchar()) != EOF) {
        putchar(c);
    }
}
