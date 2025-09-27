#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int write_retry(int fd, char* data, int size) {
    char* cdata = data;
    while (1) {
        int written_bytes = write(fd, cdata, size);
        if (written_bytes == 0) { // если write вернул 0, значит файловый дескриптор закрыт с другого конца 
                                  // или конец файла
            return cdata - data;
        }
        if (written_bytes < 0) { // если возвращено значение < 0, то это ошибка
            if (errno == EAGAIN || errno == EINTR) { // она может быть retryable
                continue;
            } else { // а может быть критичной, и нет смысла пытаться повторить попытку чтения
                return -1;
            }
        }
        // если возвращенное значение > 0, значит успешно прочитано столько байт
        cdata += written_bytes;
        size -= written_bytes;
        if (size == 0) {
            return cdata - data;
        }
    }
}

struct BufferedWriter {
    int fd;
    size_t capacity;
    size_t size;
    char* buffer;
} s;

void init_buffer(struct BufferedWriter* writer, size_t capacity, int fd) {
    char* buffer = (char*)malloc(capacity);

    writer->capacity = capacity;
    writer->size = 0;
    writer->buffer = buffer;
    writer->fd = fd;
}

void close_buffer(struct BufferedWriter* writer) {
    if (writer->size > 0) {
        write_retry(writer->fd, writer->buffer, writer->size);
    }
    free(writer->buffer);
}

void buffered_write(struct BufferedWriter* writer, char* string) {
    size_t len = strlen(string);
    // это условие некорректно, если длина строки может быть очень большой
    // в нашем примере мы знаем, что длины маленькие, поэтому
    // для упрощения работаем с таким условием
    if (len + writer->size > writer->capacity) {
        size_t first_part = writer->capacity - writer->size;
        memcpy(writer->buffer + writer->size, string, first_part);
        write_retry(writer->fd, writer->buffer, writer->capacity);
        memcpy(writer->buffer, string + first_part, len - first_part);
        writer->size = len - first_part;
    } else {
        memcpy(writer->buffer + writer->size, string, len);
        writer->size += len;
    }
}

void write_impl(FILE* source) {
    struct BufferedWriter writer;
    size_t capacity = 1024 * 1024;  // 1MB
    init_buffer(&writer, capacity, STDOUT_FILENO);

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, source)) != -1) {
        buffered_write(&writer, line);
    }

    if (line) {
        free(line);
    }

    close_buffer(&writer);
}

int main() {
    FILE* file = fopen("/Users/k.afentev/CAOS/caos-2025/sem03-lowlevel-io-fs/huge_file.txt", "r");

    write_impl(file);

    fclose(file);
    
    return 0;
}
