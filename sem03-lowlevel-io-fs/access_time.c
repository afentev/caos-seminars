#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

int main(int argc, char *argv[]) {
    struct stat s;
    fstat(0, &s); 
    
    printf("update time: %s", asctime(gmtime(&s.st_mtimespec.tv_sec))); // '\n' есть в строке генерируемой asctime
    printf("access time: %s", asctime(gmtime(&s.st_atimespec.tv_sec)));

    return 0;
}