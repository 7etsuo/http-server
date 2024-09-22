#include "http_handler.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int build_and_send_response(int client_fd, const char *version, int status, const char *content,
                                   const char *content_type);
static int send_response(const char *response_str, int client_fd);

static int dispatch_method(http_request *request, int client_fd);
static int handle_http_get(http_request *request, int client_fd);
static int handle_http_unkown(http_request *request, int client_fd);
static int handle_http_post(http_request *request, int client_fd);
static int handle_http_put(http_request *request, int client_fd);
static int handle_http_delete(http_request *request, int client_fd);
static int handle_http_head(http_request *request, int client_fd);
static int handle_http_options(http_request *request, int client_fd);
static int handle_http_patch(http_request *request, int client_fd);
static int handle_http_trace(http_request *request, int client_fd);
static int handle_http_connect(http_request *request, int client_fd);

static int dispatch_uri(http_request *request, int client_fd);
static int handle_root(http_request *request, int client_fd);
static int handle_echo(http_request *request, int client_fd);
static int handle_user_agent(http_request *request, int client_fd);
static int handle_not_found(http_request *request, int client_fd);

int (*handle_http_method[])(http_request *request, int client_fd) = {
    handle_http_get,
    handle_http_post,
    handle_http_put,
    handle_http_delete,
    handle_http_head,
    handle_http_options,
    handle_http_patch,
    handle_http_trace,
    handle_http_connect,
    handle_http_unkown,
    NULL,
};

struct
{
    const char *uri;
    int (*handler)(http_request *request, int client_fd);
} uri_entry[] = {
    {"/", handle_root},
    {"/echo/", handle_echo},
    {"/user-agent", handle_user_agent},
    {NULL, NULL},
};

int handle_request(http_request *request, int client_fd)
{
    return dispatch_method(request, client_fd);
}

static int dispatch_method(http_request *request, int client_fd)
{
    return handle_http_method[request->request_line.method](request, client_fd);
}

static int build_and_send_response(int client_fd, const char *version, int status, const char *content,
                                   const char *content_type)
{
    const char *http_header_format = "Content-Type:%s\r\n";
    char http_header[HTTP_HEADER_VALUE_MAX_LEN];
    snprintf(http_header, sizeof(http_header), http_header_format, content_type);

    char *response_str = build_http_response(version, status, content, strlen(content), http_header);
    if (response_str == NULL)
        return -1;

    int retval = send_response(response_str, client_fd);
    free(response_str);

    return retval;
}

static int send_response(const char *response_str, int client_fd)
{
    ssize_t n = send(client_fd, response_str, strlen(response_str), 0);
    if (n == -1)
    {
        perror("Failed to send response to client");
        return -1;
    }

    return 0;
}

static int dispatch_uri(http_request *request, int client_fd)
{
    if (request->request_line.uri == NULL)
        return handle_not_found(request, client_fd);

    if (request->request_line.uri[0] == '/' && request->request_line.uri[1] == '\0')
        return uri_entry[0].handler(request, client_fd);

    for (int i = 1; uri_entry[i].uri != NULL; i++)
    {
        size_t uri_len = strlen(uri_entry[i].uri);

        if (strncmp(request->request_line.uri, uri_entry[i].uri, uri_len) == 0)
            return uri_entry[i].handler(request, client_fd);
    }

    return handle_not_found(request, client_fd);
}

static int handle_root(http_request *request, int client_fd)
{
    return build_and_send_response(client_fd, request->request_line.version, HTTP_OK, "OK", "text/html");
}

static int handle_echo(http_request *request, int client_fd)
{
    char *echo = request->request_line.uri + strlen("/echo/");
    return build_and_send_response(client_fd, request->request_line.version, HTTP_OK, echo, "text/plain");
}

static int handle_user_agent(http_request *request, int client_fd)
{
    for (int i = 0; i < request->header_count; i++)
    {
        if (strcmp(request->headers[i].name, "User-Agent") == 0)
            return build_and_send_response(client_fd, request->request_line.version, HTTP_OK, request->headers[i].value,
                                           "text/plain");
    }

    return build_and_send_response(client_fd, request->request_line.version, HTTP_NOT_FOUND, "Not Found", "text/plain");
}

static int handle_not_found(http_request *request, int client_fd)
{
    return build_and_send_response(client_fd, request->request_line.version, HTTP_NOT_FOUND, "Not Found", "text/plain");
}

static int handle_http_get(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_unkown(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_post(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_put(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_delete(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_head(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_options(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_patch(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_trace(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}

static int handle_http_connect(http_request *request, int client_fd)
{
    return dispatch_uri(request, client_fd);
}
