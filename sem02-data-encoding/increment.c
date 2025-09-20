int check_increment(int x) {
    return x + 1 > x; // Всегда ли true?
}

int unsigned_check_increment(unsigned int x) {
    return x + 1 > x; // Всегда ли true?
}

#include <stdio.h>
#include <stdint.h>

int main() {
    printf("x =     %d\n", INT32_MAX);
    printf("x + 1 = %d\n", INT32_MAX + 1);
    printf("x + 1 > x, for int x  = INT32_MAX:  %d\n\n\n", check_increment(INT32_MAX));
    printf("x =     %u\n", UINT32_MAX);
    printf("x + 1 = %u\n", UINT32_MAX + 1);
    printf("x + 1 > x, for uint x = UINT32_MAX: %d\n", unsigned_check_increment(UINT32_MAX));
}
