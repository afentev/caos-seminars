#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void impl(FILE* source) {
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, source)) != -1) {
#if defined(_USE_PRINTF)
        printf("%s", line);
#endif
#if defined(_USE_WRITE)
        write(STDOUT_FILENO, line, len);
#endif
    }

    if (line) {
        free(line);
    }
}

int main() {
    // в домашке по этой теме fopen нельзя будет использовать :)
    FILE* file = fopen("/Users/k.afentev/CAOS/caos-2025/sem03-lowlevel-io-fs/huge_file.txt", "r");

    impl(file);

    fclose(file);
    
    return 0;
}
