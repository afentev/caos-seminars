#include <stdio.h>
#include <stdlib.h>

int get(int *a, int i) {
    return a[i];
}

int main(void) {
    int a[10];
    printf("%d\n", get(a, 2));
    printf("%d\n", get(a, 27));
    printf("%d\n", get(a, 2007));
    printf("%d\n", get(a, 4000007));
}
