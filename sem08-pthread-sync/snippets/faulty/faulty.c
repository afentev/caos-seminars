#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

int64_t x;
int64_t y;
int64_t r1;
int64_t r2;

void* write_x_read_y(void* arg) {
  (void)arg;

  x = 1;
  r1 = y;

  return NULL;
}

void* write_y_read_x(void* arg) {
  (void)arg;

  y = 1;
  r2 = x;

  return NULL;
}

int main() {
  for (size_t i = 0;; ++i) {
    x = 0;
    y = 0;

    r1 = 0;
    r2 = 0;

    pthread_t t1;
    pthread_create(&t1, /*attr=*/NULL, &write_x_read_y, /*arg=*/NULL);
    pthread_t t2;
    pthread_create(&t2, /*attr=*/NULL, &write_y_read_x, /*arg=*/NULL);

    pthread_join(t1, /*thread_return=*/NULL);
    pthread_join(t2, /*thread_return=*/NULL);

    if (r1 == 0 && r2 == 0) {
      printf("Iteration #%lu: CPU is broken\n", i);
      abort();
    }
  }
}
