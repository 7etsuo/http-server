#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "my_socket.h"

int create_server_socket(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        printf("Socket creation failed: %s...\n", strerror(errno));
        return -1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        printf("SO_REUSEADDR failed: %s \n", strerror(errno));
        return -1;
    }

    return server_fd;
}

void set_server_address(struct sockaddr_in *serv_addr, int port)
{
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(port);
    serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);
}

int bind_server_socket(int server_fd, struct sockaddr_in *serv_addr)
{
    if (bind(server_fd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) != 0)
    {
        printf("Bind failed: %s \n", strerror(errno));
        return -1;
    }

    return 0;
}

int listen_server_socket(int server_fd, int connection_backlog)
{
    if (listen(server_fd, connection_backlog) != 0)
    {
        printf("Listen failed: %s \n", strerror(errno));
        return -1;
    }

    return 0;
}

int accept_client(int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return -1;
        perror("Failed to accept client connection");
        return -1;
    }

    if (set_socket_nonblocking(client_fd) == -1)
    {
        perror("Failed to set client socket to non-blocking mode");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

int set_socket_nonblocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
    {
        return -1;
    }

    flags |= O_NONBLOCK;
    return fcntl(socket_fd, F_SETFL, flags);
}

int init_server(void)
{
    int server_fd = create_server_socket();
    if (server_fd == -1)
        return -1;

    struct sockaddr_in serv_addr;
    set_server_address(&serv_addr, PORT);

    if (bind_server_socket(server_fd, &serv_addr) != 0)
    {
        close(server_fd);
        return -1;
    }

    if (listen_server_socket(server_fd, BACKLOG) != 0)
    {
        close(server_fd);
        return -1;
    }

    if (set_socket_nonblocking(server_fd) == -1)
    {
        perror("Failed to set server socket to non-blocking mode");
        close(server_fd);
        return -1;
    }

    return server_fd;
}
