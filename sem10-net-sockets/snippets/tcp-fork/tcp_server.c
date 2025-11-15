#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const int BUFF_SIZE = 4096;

int server_socket_init(uint16_t port) {
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        exit(1);
    }
    
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    int connection_status = bind(
        server_fd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connection_status) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    int listening_status = listen(server_fd, SOMAXCONN);
    if (listening_status) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    
    printf("Server listening on port %d\n", port);
    return server_fd;
}

ssize_t read_message(int client_fd, char* buff, size_t buff_size) {
    ssize_t bytes_read = read(client_fd, buff, buff_size - 1);
    if (bytes_read > 0) {
        buff[bytes_read] = '\0';
    } else if (bytes_read == 0) {
        printf("Client disconnected\n");
    } else {
        perror("Read error");
    }
    return bytes_read;
}

void write_message(int client_fd, char* buff, size_t len) {
    size_t total_written = 0;
    while (total_written < len) {
        ssize_t bytes_written = write(client_fd, buff + total_written, len - total_written);
        if (bytes_written <= 0) {
            perror("Write error");
            break;
        }
        total_written += bytes_written;
    }
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    int server_fd = server_socket_init(atoi(argv[1]));
    
    printf("Forking server started. Process PID: %d\n", getpid());
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Accept failed");
            continue;
        }
        
        printf("New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            
            printf("Child process %d handling client %s:%d\n", 
                   getpid(), inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            char msg[BUFF_SIZE];
            while (1) {
                ssize_t bytes_read = read_message(client_fd, msg, sizeof(msg));
                if (bytes_read <= 0) {
                    break;
                }
                
                printf("[Child %d] Received: %s\n", getpid(), msg);
                
                if (strcmp(msg, "q") == 0) {
                    printf("[Child %d] Quit command received\n", getpid());
                    break;
                }
                
                write_message(client_fd, msg, bytes_read);
                printf("[Child %d] Echoed back: %s\n", getpid(), msg);
            }
            
            shutdown(client_fd, SHUT_RDWR);
            close(client_fd);
            printf("Child process %d exiting\n", getpid());
            exit(0);
            
        } else if (pid > 0) {
            close(client_fd);
            printf("Spawned child process %d for client handling\n", pid);
        } else {
            perror("Fork failed");
            close(client_fd);
        }
    }
    
    close(server_fd);
    return 0;
}