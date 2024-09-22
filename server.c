#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include "http_handler.h"
#include "my_socket.h"
#include "request.h"
#include "response.h"

#define MAX_REQUEST_SIZE 2048
#define MAX_CLIENTS 10
#define POLL_TIMEOUT 5000 // 5 seconds

char *wait_for_client_request(int client_fd, size_t *nrecv);
int handle_new_connection(int server_fd, struct pollfd *fds, int *nfds);
void handle_client_request(struct pollfd *pfd);

static void print_http_request(http_request *request)
{
    printf("Method: %s\n", http_methods[request->request_line.method].name);
    printf("URI: %s\n", request->request_line.uri);
    printf("Version: %s\n", request->request_line.version);
    printf("Header count: %d\n", request->header_count);
    for (int i = 0; i < request->header_count; i++)
    {
        printf("Header %d: %s: %s\n", i, request->headers[i].name, request->headers[i].value);
    }
    printf("Body: %s\n", request->body);
}

static void initialize_server(void);
static int setup_server(void);
static void run_server_loop(int server_fd, struct pollfd *fds, int *nfds);
static void cleanup_server(int server_fd);

int main(int argc, char *argv[])
{
    initialize_server();

    int server_fd = setup_server();
    if (server_fd == -1)
        return 1;

    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    run_server_loop(server_fd, fds, &nfds);

    cleanup_server(server_fd);
    return 0;
}

static void initialize_server(void)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
}

static int setup_server(void)
{
    return init_server();
}

static void run_server_loop(int server_fd, struct pollfd *fds, int *nfds)
{
    while (1)
    {
        int poll_count = poll(fds, *nfds, POLL_TIMEOUT);

        if (poll_count == -1)
        {
            perror("poll");
            break;
        }

        if (poll_count == 0)
            continue;

        for (int i = 0; i < *nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == server_fd)
                {
                    if (handle_new_connection(server_fd, fds, nfds) == -1)
                        return;
                }
                else
                {
                    handle_client_request(&fds[i]);
                }
            }
        }
    }
}

static void cleanup_server(int server_fd)
{
    close(server_fd);
}

int handle_new_connection(int server_fd, struct pollfd *fds, int *nfds)
{
    int client_fd = accept_client(server_fd);
    if (client_fd == -1)
        return -1;

    if (*nfds < MAX_CLIENTS + 1)
    {
        fds[*nfds].fd = client_fd;
        fds[*nfds].events = POLLIN;
        (*nfds)++;
    }
    else
    {
        fprintf(stderr, "Too many clients. Connection rejected.\n");
        close(client_fd);
    }

    return 0;
}

void handle_client_request(struct pollfd *pfd)
{
    http_request request;
    size_t nrecv = 0;
    char *client_request = wait_for_client_request(pfd->fd, &nrecv);

    if (client_request == NULL)
    {
        close(pfd->fd);
        pfd->fd = -1;
        return;
    }

    if (parse_http_request(client_request, nrecv, &request) == -1)
    {
        free(client_request);
        close(pfd->fd);
        pfd->fd = -1;
        return;
    }

#ifdef DEBUG
    print_http_request(&request);
#endif // DEBUG

    handle_request(&request, pfd->fd);
    free(client_request);
}

char *wait_for_client_request(int client_fd, size_t *nrecv)
{
    char *buffer = (char *)malloc(MAX_REQUEST_SIZE);
    if (!buffer)
    {
        fprintf(stderr, "Failed to allocate memory for client request buffer.\n");
        return NULL;
    }

    ssize_t n = recv(client_fd, buffer, MAX_REQUEST_SIZE, 0);
    if (n == -1)
    {
        if (errno == ECONNRESET)
        {
            fprintf(stderr, "Connection reset by peer.\n");
        }
        else
        {
            perror("Failed to receive data from client");
        }
        free(buffer);
        return NULL;
    }

    if (n == 0)
    {
        fprintf(stderr, "Client closed the connection.\n");
        free(buffer);
        return NULL;
    }

    *nrecv = n;
    return buffer;
}
