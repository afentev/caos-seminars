#include <pthread.h>
#include <stdio.h>

static int N = 10;

void* thread_func(void* arg) {
    int id = *(int*)arg;
    printf("  Thread %d func started\n", id);
    printf("  Thread %d func finished\n", id);
    return NULL;
}

int main() {
    printf("Main func started\n");
    pthread_t thread[N];
    int       counter[N];
    for (int i = 0; i < N; ++i) {
        counter[i] = i;
        pthread_create(&thread[i], NULL, thread_func, &counter[i]);
    }
    for (int i = 0; i < N; ++i) {
        pthread_join(thread[i], NULL);
    }
    printf("Main func finished\n");
    return 0;
}