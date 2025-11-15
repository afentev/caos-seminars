#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFF_SIZE 4096
#define MAX_EVENTS 64

typedef struct {
    int fd;
    char buffer[BUFF_SIZE];
    int buffer_begin;
    int buffer_end;
} fd_data;

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

int disable_io_block(int fd) {
    int current_descriptor_flags = fcntl(fd, F_GETFL);
    if (current_descriptor_flags == -1) {
        perror("fcntl F_GETFL failed");
        return -1;
    }
    
    if (fcntl(fd, F_SETFL, current_descriptor_flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
        return -1;
    }
    return 0;
}

int read_buff(fd_data* data) {
    if (data->buffer_end >= BUFF_SIZE) {
        return -1;
    }
    
    ssize_t bytes_read = read(data->fd, data->buffer + data->buffer_end, 
                             BUFF_SIZE - data->buffer_end);
    
    if (bytes_read > 0) {
        data->buffer_end += bytes_read;
        return bytes_read;
    } else if (bytes_read == 0) {
        return 0;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        perror("Read error");
        return -1;
    }
}

int write_buff(fd_data* data) {
    if (data->buffer_begin >= data->buffer_end) {
        return 0;
    }
    
    ssize_t bytes_written = write(data->fd, data->buffer + data->buffer_begin, 
                                 data->buffer_end - data->buffer_begin);
    
    if (bytes_written > 0) {
        data->buffer_begin += bytes_written;
        
        if (data->buffer_begin == data->buffer_end) {
            data->buffer_begin = 0;
            data->buffer_end = 0;
        }
        return bytes_written;
    } else if (bytes_written == 0) {
        return 0;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        perror("Write error");
        return -1;
    }
}

void close_connection(fd_data* data, int epoll_fd) {
    if (data && data->fd >= 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, data->fd, NULL);
        shutdown(data->fd, SHUT_RDWR);
        close(data->fd);
        free(data);
        printf("Connection closed\n");
    }
}

void process_epoll_event(struct epoll_event* event, int server_fd, int epoll_fd) {
    if (event->data.fd == server_fd) {
        // Handle new connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Accept failed");
            }
            return;
        }
        
        if (disable_io_block(client_fd)) {
            close(client_fd);
            return;
        }
        
        struct epoll_event client_event;
        fd_data* add_data = calloc(1, sizeof(*add_data));
        if (!add_data) {
            perror("Failed to allocate client data");
            close(client_fd);
            return;
        }
        
        add_data->fd = client_fd;
        add_data->buffer_begin = 0;
        add_data->buffer_end = 0;
        
        client_event.data.ptr = add_data;
        client_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) < 0) {
            perror("Add client to epoll failed");
            free(add_data);
            close(client_fd);
            return;
        }
        
        printf("New client connected: %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
    } else {
        // Handle client event
        fd_data* data = (fd_data*)event->data.ptr;
        if (!data) return;
        
        const uint32_t mask = event->events;
        
        if (mask & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
            printf("Client disconnected (error or hangup)\n");
            close_connection(data, epoll_fd);
            return;
        }
        
        if (mask & EPOLLIN) {
            while (1) {
                int read_result = read_buff(data);
                if (read_result > 0) {
                    printf("Received %d bytes from client: %s\n", read_result, data->buffer);
                    
                    struct epoll_event modify_event;
                    modify_event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP;
                    modify_event.data.ptr = data;
                    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, data->fd, &modify_event);
                } else if (read_result == 0) {
                    break;
                } else {
                    close_connection(data, epoll_fd);
                    return;
                }
            }
        }
        
        if (mask & EPOLLOUT) {
            int write_result = write_buff(data);
            if (write_result < 0) {
                close_connection(data, epoll_fd);
                return;
            }

            if (data->buffer_begin == data->buffer_end) {
                struct epoll_event modify_event;
                modify_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                modify_event.data.ptr = data;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, data->fd, &modify_event);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    
    int server_fd = server_socket_init(atoi(argv[1]));
    
    if (disable_io_block(server_fd)) {
        close(server_fd);
        return 1;
    }
    
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1 failed");
        close(server_fd);
        return 1;
    }
    
    struct epoll_event listen_event;
    listen_event.events = EPOLLIN;
    listen_event.data.fd = server_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &listen_event) < 0) {
        perror("Adding server socket to epoll failed");
        close(epoll_fd);
        close(server_fd);
        return 1;
    }
    
    struct epoll_event pending_events[MAX_EVENTS];
    
    printf("Epoll server started. Waiting for connections...\n");
    
    while (1) {
        int pending_events_count = epoll_wait(epoll_fd, pending_events, MAX_EVENTS, -1);
        if (pending_events_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait failed");
            break;
        }
        
        for (int i = 0; i < pending_events_count; i++) {
            process_epoll_event(&pending_events[i], server_fd, epoll_fd);
        }
    }
    
    close(epoll_fd);
    close(server_fd);
    return 0;
}