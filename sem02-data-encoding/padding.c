#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    struct s {
        int i;    // 4 bytes
        char c;   // 1 byte
        double d; // 8 bytes
        char a;   // 1 byte
    };

    /* Output is compiler dependent */

    printf("offsets: i=%ld; c=%ld; d=%ld a=%ld\n",
        (long) offsetof(struct s, i),
        (long) offsetof(struct s, c),
        (long) offsetof(struct s, d),
        (long) offsetof(struct s, a));
    printf("sizeof(struct s)=%ld\n", (long) sizeof(struct s));
}