#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() {
    int input_fd  = open("/Users/k.afentev/CAOS/caos-2025/sem03-lowlevel-io-fs/input.txt",  O_RDONLY);
    int output_fd = open("/Users/k.afentev/CAOS/caos-2025/sem03-lowlevel-io-fs/output.txt", O_WRONLY | O_TRUNC);

    int buffer_size = 16;
    char* buffer = (char*)malloc(buffer_size);

    int read_bytes = read(input_fd, buffer, buffer_size);
    int written = write(output_fd, buffer, read_bytes);

    close(input_fd);
    close(output_fd);

    return 0;
}