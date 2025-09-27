#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <dirent.h>

int main(int argc, char** argv) {
    assert(argc > 1);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(argv[1])) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            printf("%s\n", ent->d_name);
        }
        closedir(dir);
    } else {
        printf("Cannot open directory\n");
        return 1;
    }
}
