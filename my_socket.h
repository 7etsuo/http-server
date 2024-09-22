#ifndef MY_SOCKET_H
#define MY_SOCKET_H

#define BACKLOG 5
#define PORT 4221

#include <netinet/in.h>

int init_server(void);
int accept_client(int server_fd);
int set_socket_nonblocking(int socket_fd);

#endif // MY_SOCKET_H