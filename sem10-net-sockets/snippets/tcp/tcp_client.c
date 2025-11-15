#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP> <port>\n", argv[0]);
        return 1;
    }
    
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Invalid address");
        close(fd);
        return 1;
    }
    
    int connection_status = connect(fd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connection_status) {
        printf("Connect failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    printf("Connected to server %s:%s\n", argv[1], argv[2]);
    printf("Enter messages (single 'q' to quit):\n");
    
    const int BUFF_SIZE = 4096;
    char buff[BUFF_SIZE];
    
    while (fgets(buff, sizeof(buff), stdin) != NULL) {
        size_t len = strlen(buff);
        if (len > 0 && buff[len-1] == '\n') {
            buff[len-1] = '\0';
            len--;
        }
        
        if (len == 0) {
            continue;
        }
        
        ssize_t bytes_sent = write(fd, buff, len);
        if (bytes_sent <= 0) {
            perror("Write failed");
            break;
        }
        
        if (strcmp(buff, "q") == 0) {
            printf("Quitting...\n");
            break;
        }
        
        char response[BUFF_SIZE];
        ssize_t bytes_received = read(fd, response, sizeof(response) - 1);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Server closed connection\n");
            } else {
                perror("Read failed");
            }
            break;
        }
        
        response[bytes_received] = '\0';
        printf("Server echo: %s\n", response);
    }
    
    shutdown(fd, SHUT_RDWR);
    close(fd);
    printf("Disconnected\n");
    return 0;
}