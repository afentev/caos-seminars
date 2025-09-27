#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    assert(argc > 1);

    struct stat s;
    int result = stat(argv[1], &s);
    if (result != 0) {
        printf("stat failed\n");
        return 1;
    }
    printf("is regular: %s    ", ((s.st_mode & S_IFMT) == S_IFREG) ? "yes" : "no "); // can use predefined mask
    printf("is directory: %s    ", S_ISDIR(s.st_mode) ? "yes" : "no "); // or predefined macro
    printf("is symbolic link: %s\n", S_ISLNK(s.st_mode) ? "yes" : "no "); 
    return 0;
}