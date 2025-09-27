#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() {
    int input_fd  = open("/Users/k.afentev/CAOS/caos-2025/sem03-lowlevel-io-fs/input.txt",  O_RDONLY);
    int output_fd = open("/Users/k.afentev/CAOS/caos-2025/sem03-lowlevel-io-fs/output.txt", O_WRONLY | O_TRUNC);

    int buffer_size = 16;
    char* buffer = (char*)malloc(buffer_size);
    
    ssize_t read_during_iteration;
    size_t read_bytes = 0;
    do {
        read_during_iteration = read(input_fd, buffer + read_bytes, buffer_size - read_bytes);
        if (read_during_iteration > 0) {
            read_bytes += read_during_iteration;
        }
    } while (read_during_iteration != 0);

    ssize_t written_during_iteration;
    size_t written_bytes = 0;
    do {
        written_during_iteration = write(output_fd, buffer + written_bytes, read_bytes - written_bytes);
        if (written_during_iteration > 0) {
            written_bytes += written_during_iteration;
        }
    } while (written_during_iteration != 0);

    close(input_fd);
    close(output_fd);

    return 0;
}