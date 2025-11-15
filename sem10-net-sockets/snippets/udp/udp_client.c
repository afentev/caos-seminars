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
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct in_addr input_address;
    input_address.s_addr = inet_addr(argv[1]);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr = input_address;
    server_address.sin_port = htons(atoi(argv[2]));

    const int BUFF_SIZE = 4096;
    char buff[BUFF_SIZE];
    
    printf("Client connected to %s:%s\n", argv[1], argv[2]);
    printf("Enter messages (Ctrl+D to exit):\n");
    
    while (scanf("%s", buff) > 0) {
        sendto(fd, buff, strlen(buff), 0, 
               (const struct sockaddr*)&server_address, sizeof(server_address));
        
        struct sockaddr_in response_addr;
        socklen_t response_addr_len = sizeof(response_addr);
        
        ssize_t received_bytes = recvfrom(fd, buff, sizeof(buff) - 1, 0,
                                         (struct sockaddr*)&response_addr, &response_addr_len);
        
        if (received_bytes <= 0) {
            break;
        }
        
        buff[received_bytes] = '\0';
        printf("Server response: %s\n", buff);
    }

    close(fd);
    return 0;
}