#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>

int main(int argc, char** argv) {
    int input_file = open(argv[1], O_RDWR);
    struct stat file_stats;
    fstat(input_file, &file_stats);
    char* content_ptr = mmap(NULL, file_stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, input_file, 0);

    for (size_t i = 0; i < file_stats.st_size; i++) {
        content_ptr[i] = '\0';
    }

    strcpy(content_ptr + 15, "Hello, world!");

    munmap(content_ptr, file_stats.st_size);
    close(input_file);
}
