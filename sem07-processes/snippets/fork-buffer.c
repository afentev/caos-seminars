#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    printf("Hello, world!");
    pid_t pid = fork();
    if (pid == 0) {
        printf("Hello from child with pid %d!\n", getpid());
        return 0;
    } else {
        int status;
        waitpid(pid, &status, 0);
        printf("Hello from parent!\n");
    }
}
