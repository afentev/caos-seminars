#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void* first(void* arg) {
  pthread_mutex_lock(&mutex1);
  pthread_mutex_lock(&mutex2);
  pthread_mutex_unlock(&mutex2);
  pthread_mutex_unlock(&mutex1);

  return NULL;
}

void* second(void* arg) {
  pthread_mutex_lock(&mutex2);
  pthread_mutex_lock(&mutex1);
  pthread_mutex_unlock(&mutex1);
  pthread_mutex_unlock(&mutex2);

  return NULL;
}

int main() {
  pthread_mutex_init(&mutex1, NULL);
  pthread_mutex_init(&mutex2, NULL);
  
  for (int i = 0; i < 1000; ++i) {
    printf("%d\n", i);
    fflush(stdout);

    pthread_t t1;
    pthread_create(&t1, /*attr=*/NULL, &first, /*arg=*/NULL);
    pthread_t t2;
    pthread_create(&t2, /*attr=*/NULL, &second, /*arg=*/NULL);

    pthread_join(t1, /*thread_return=*/NULL);
    pthread_join(t2, /*thread_return=*/NULL);
  }
}
