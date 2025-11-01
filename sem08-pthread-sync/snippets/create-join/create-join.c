#include <pthread.h>
#include <stdio.h>

void* thread_func(void* arg) {
    printf("  Thread func started\n");
    printf("  Thread func finished\n");
    return NULL;
}

int main() {
    printf("Main func started\n");
    pthread_t thread;
    printf("Thread creating\n");
    pthread_create(&thread, NULL, thread_func, 0); // В какой-то момент будет создан поток и в нем вызвана функция
    // Начиная отсюда неизвестно в каком порядке выполняются инструкции основного и дочернего потока
    pthread_join(thread, NULL); // -- аналог waitpid. Второй аргумент -- указатель в который запишется возвращаемое значение
    printf("Thread joined\n");
    printf("Main func finished\n");
    return 0;
}