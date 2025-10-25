#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>


int main() {
    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0664);
    dup2(fd, STDOUT_FILENO); // redirect stdout to file
    close(fd);
    printf("Redirectred 'Hello world!'");
    return 0;
}