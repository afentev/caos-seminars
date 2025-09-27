#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    char buffer[4096];
    int bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
    printf("From stdin: '%.*s'\n", bytes_read, buffer); // buffer not zero-terminated string after `read`!
    
    return 0;
}