#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_http.h"
#include "response.h"

char *status_line_error = "HTTP/1.1 500 Internal Server Error\r\n\r\n";

const http_status_entry http_statuses[HTTP_STATUS_COUNT] = {
    {HTTP_OK, "OK"}, {HTTP_NOT_FOUND, "Not Found"}, {HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error"}};

static void set_http_headers(http_response *response, const char *headers);

void print_hex(const char *str, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x ", str[i]);
    }
    printf("\n");
}

void debug_response(http_response *response)
{
    printf("Version: %s\t", response->version);
    print_hex(response->version, strlen(response->version));
    printf("Status: %d %s\t", response->status.code, response->status.reason);
    print_hex(response->status.reason, strlen(response->status.reason));
    printf("Headers:\t");
    for (int i = 0; i < response->header_count; i++)
    {
        printf("%s: %s\t", response->headers[i].name, response->headers[i].value);
        print_hex(response->headers[i].name, strlen(response->headers[i].name));
    }
    printf("Body: %s\t", response->body);
    print_hex(response->body, response->body_length);
}

char *build_http_response(const char *version, http_status_code code, const char *body, size_t length,
                          const char *headers)
{
    http_response *response = malloc(sizeof(http_response));
    if (!response)
    {
        fprintf(stderr, "Error: Unable to allocate memory for response\n");
        return NULL;
    }

    init_http_response(response, version);
    set_http_status(response, code);

    if (headers)
    {
        set_http_headers(response, headers);
    }

    if (body)
    {
        set_http_body(response, body, length);
    }

    char *response_str = format_http_response(response);

#ifdef DEBUG
    printf("=== Response ===\n");
    debug_response(response);
    printf("=== Response ===\n");
#endif

    cleanup_http_response(response);

    return response_str;
}

void init_http_response(http_response *response, const char *version)
{
    if (response)
    {
        response->version = version ? strdup(version) : strdup("HTTP/1.1");
        response->status = http_statuses[0]; // Default to HTTP_OK
        response->header_count = 0;
        response->body = NULL;
        response->body_length = 0;
    }
}

void cleanup_http_response(http_response *response)
{
    if (response)
    {
        if (response->version)
        {
            free((void *)response->version);
            response->version = NULL;
        }
        for (int i = 0; i < response->header_count; i++)
        {
            if (response->headers[i].name)
            {
                free((void *)response->headers[i].name);
                response->headers[i].name = NULL;
            }
            if (response->headers[i].value)
            {
                free((void *)response->headers[i].value);
                response->headers[i].value = NULL;
            }
        }
        response->header_count = 0;
        if (response->body)
        {
            free(response->body);
            response->body = NULL;
            response->body_length = 0;
        }
    }
}

void set_http_status(http_response *response, http_status_code code)
{
    assert(response);
    response->status.code = code;
    response->status.reason = find_http_status(code)->reason;
}

const http_status_entry *find_http_status(http_status_code code)
{
    for (size_t i = 0; i < HTTP_STATUS_COUNT; i++)
        if (http_statuses[i].code == code)
            return &http_statuses[i];
    return NULL;
}

int add_http_header(http_response *response, const char *name, const char *value)
{
    if (response && response->header_count < HTTP_MAX_HEADERS)
    {
        response->headers[response->header_count].name = strdup(name);
        response->headers[response->header_count].value = strdup(value);
        if (response->headers[response->header_count].name && response->headers[response->header_count].value)
        {
            response->header_count++;
            return 0;
        }
        else
        {
            fprintf(stderr, "Error: Unable to allocate memory for header\n");
            return -1;
        }
    }
    fprintf(stderr, "Error: Exceeded maximum headers or null response\n");
    return -1;
}

void set_http_body(http_response *response, const char *body, size_t length)
{
    if (response)
    {
        if (response->body)
        {
            free(response->body);
            response->body = NULL;
            response->body_length = 0;
        }
        response->body = body ? strndup(body, length) : NULL;
        if (body)
        {
            response->body_length = length;
        }
        else
        {
            response->body_length = 0;
        }

        // Automatically set Content-Length header
        char content_length[20];

        snprintf(content_length, sizeof(content_length), "%zu", length);
        // Remove the old Content-Length header if it exists
        // assumes that the Content-Length header is unique and only appears once
        int found = -1;
        for (int i = 0; i < response->header_count; i++)
        {
            if (strcasecmp(response->headers[i].name, "Content-Length") == 0)
            {
                found = i;
                break;
            }
        }
        if (found != -1)
        {
            free((void *)response->headers[found].value);
            response->headers[found].value = strdup(content_length);
            if (!response->headers[found].value)
                fprintf(stderr, "Error: Unable to allocate memory for header value\n");
        }
        else
        {
            add_http_header(response, "Content-Length", content_length);
        }
    }
}

char *format_http_response(const http_response *response)
{
    if (!response)
        return NULL;

    // estimate the size needed
    size_t size = 0;
    size += strlen(response->version) + 1; // version + space + null terminator
    size += MAX_STATUS_LEN;                // status code and reason
    size += 2;                             // CRLF

    for (int i = 0; i < response->header_count; i++)
    {
        size += strlen(response->headers[i].name) + 2;  // header name + colon + space
        size += strlen(response->headers[i].value) + 2; // header value + CRLF
    }

    size += 2; // CRLF after headers

    size += response->body_length;

    if (size > HTTP_MAX_RESPONSE_SIZE)
    {
        fprintf(stderr, "Error: Response size exceeds maximum size\n");
        return NULL;
    }

    char *response_str = malloc(size + 1); // +1 for null terminator
    if (!response_str)
    {
        fprintf(stderr, "Error: Unable to allocate memory for response\n");
        return NULL;
    }

    // initialize the response string
    response_str[0] = '\0';

    // start formatting
    char status_line[MAX_STATUS_LEN];
    snprintf(status_line, sizeof(status_line), "%s %d %s\r\n", response->version, response->status.code,
             response->status.reason);
    strncat(response_str, status_line, size + 1 - strlen(response_str)); // +1 for null terminator

    for (int i = 0; i < response->header_count; i++)
    {
        char header_line[HTTP_HEADER_NAME_MAX_LEN + HTTP_HEADER_VALUE_MAX_LEN + 4]; // 2 for colon and space, 2 for CRLF
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n", response->headers[i].name, response->headers[i].value);
        strncat(response_str, header_line, size + 1 - strlen(response_str));
    }

    strncat(response_str, "\r\n", size + 1 - strlen(response_str)); // end of headers

    if (response->body && response->body_length > 0)
        strncat(response_str, response->body, size + 1 - strlen(response_str));

    return response_str;
}

static void set_http_headers(http_response *response, const char *headers)
{
    if (response && headers)
    {
        char *header = strdup(headers);
        char *header_line = strtok(header, "\r\n");
        while (header_line)
        {
            char *name = strtok(header_line, ":");
            char *value = strtok(NULL, ":");
            if (name && value)
            {
                add_http_header(response, name, value);
            }
            header_line = strtok(NULL, "\r\n");
        }
        free(header);
    }
}