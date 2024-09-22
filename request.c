#include "request.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


const http_method_entry http_methods[HTTP_METHOD_COUNT] = {
    {HTTP_METHOD_GET, "GET"},       {HTTP_METHOD_POST, "POST"},   {HTTP_METHOD_PUT, "PUT"},
    {HTTP_METHOD_DELETE, "DELETE"}, {HTTP_METHOD_HEAD, "HEAD"},   {HTTP_METHOD_OPTIONS, "OPTIONS"},
    {HTTP_METHOD_PATCH, "PATCH"},   {HTTP_METHOD_TRACE, "TRACE"}, {HTTP_METHOD_CONNECT, "CONNECT"}};

static char *duplicate_string_or_exit(const char *source)
{
    char *dup = strdup(source);
    if (!dup)
    {
        fprintf(stderr, "Failed to allocate memory for string duplication.\n");
        return NULL;
    }

    return dup;
}

void init_http_request(http_request *request)
{
    if (request)
    {
        request->request_line.method = HTTP_METHOD_UNKNOWN;
        request->request_line.uri = NULL;
        request->request_line.version = NULL;

        // initialize headers
        request->header_count = 0;
        for (int i = 0; i < HTTP_MAX_HEADERS; i++)
        {
            request->headers[i].name = NULL;
            request->headers[i].value = NULL;
        }

        // initialize body
        request->body = NULL;
        request->body_length = 0;
    }
}

void cleanup_http_request(http_request *request)
{
    if (request)
    {
        if (request->request_line.uri)
        {
            free(request->request_line.uri);
            request->request_line.uri = NULL;
        }
        if (request->request_line.version)
        {
            free(request->request_line.version);
            request->request_line.version = NULL;
        }

        // free headers
        for (int i = 0; i < request->header_count; i++)
        {
            if (request->headers[i].name)
            {
                free((void *)request->headers[i].name);
                request->headers[i].name = NULL;
            }
            if (request->headers[i].value)
            {
                free((void *)request->headers[i].value);
                request->headers[i].value = NULL;
            }
        }
        request->header_count = 0;
        if (request->body)
        {
            free(request->body);
            request->body = NULL;
            request->body_length = 0;
        }
    }
}

http_method find_http_method(const char *method_str)
{
    if (!method_str)
        return HTTP_METHOD_UNKNOWN;

    for (size_t i = 0; i < HTTP_METHOD_COUNT; i++)
        if (strcasecmp(method_str, http_methods[i].name) == 0)
            return http_methods[i].method;

    return HTTP_METHOD_UNKNOWN;
}

int store_http_header(http_request *request, const char *name, const char *value)
{
    if (request && name && value && request->header_count < HTTP_MAX_HEADERS)
    {
        request->headers[request->header_count].name = duplicate_string_or_exit(name);
        request->headers[request->header_count].value = duplicate_string_or_exit(value);
        request->header_count++;
        return 0;
    }
    fprintf(stderr, "Failed to store header exceeded maximum headers or invalid parameters : %s: %s\n", name, value);
    return -1; //
}

void store_http_body(http_request *request, const char *body, size_t length)
{
    if (request)
    {
        // Free existing body if present
        if (request->body)
        {
            free(request->body);
        }
        // Duplicate the body content
        request->body = body ? strndup(body, length) : NULL;
        if (body)
        {
            request->body_length = length;
        }
        else
        {
            request->body_length = 0;
        }
        // Automatically set Content-Length header if body is present
        if (body)
        {
            char content_length[20];
            snprintf(content_length, sizeof(content_length), "%zu", length);
            // Check if Content-Length header already exists
            int found = -1;
            for (int i = 0; i < request->header_count; i++)
            {
                if (strcasecmp(request->headers[i].name, "Content-Length") == 0)
                {
                    found = i;
                    break;
                }
            }
            if (found != -1)
            {
                // Update existing Content-Length header
                free(request->headers[found].value);
                request->headers[found].value = duplicate_string_or_exit(content_length);
            }
            else
            {
                // Add new Content-Length header
                if (store_http_header(request, "Content-Length", content_length) != 0)
                {
                    fprintf(stderr, "Error: Unable to add Content-Length header.\n");
                }
            }
        }
    }
}

int parse_http_request(const char *raw_request, size_t request_length, http_request *request)
{
    if (!raw_request || !request)
        return -1;

    // Initialize the request structure
    init_http_request(request);

    // Make a copy of the raw request to tokenize
    char *request_copy = strndup(raw_request, request_length);
    if (!request_copy)
    {
        fprintf(stderr, "Error: Memory allocation failed while copying raw request.\n");
        cleanup_http_request(request);
        return -1;
    }

    char *saveptr;

    // Parse the request line
    char *line = strtok_r(request_copy, "\r\n", &saveptr);
    if (!line)
    {
        fprintf(stderr, "Error: Invalid HTTP request. Missing request line.\n");
        free(request_copy);
        cleanup_http_request(request);
        return -1;
    }

    // Tokenize the request line into method, URI, and version
    char *method_str = strtok(line, " ");
    char *uri = strtok(NULL, " ");
    char *version = strtok(NULL, " ");

    if (!method_str || !uri || !version)
    {
        fprintf(stderr, "Error: Malformed request line.\n");
        free(request_copy);
        cleanup_http_request(request);
        return -1;
    }

    // Set the method
    request->request_line.method = find_http_method(method_str);

    // Duplicate and set the URI
    request->request_line.uri = duplicate_string_or_exit(uri);

    // Duplicate and set the HTTP version
    request->request_line.version = duplicate_string_or_exit(version);

    // Parse headers
    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL && strlen(line) > 0)
    {
        char *colon = strchr(line, ':');
        if (!colon)
        {
            fprintf(stderr, "Error: Malformed header line (missing colon).\n");
            free(request_copy);
            cleanup_http_request(request);
            return -1;
        }
        *colon = '\0'; // Split the line into name and value
        char *name = line;
        char *value = colon + 1;

        // Trim leading and trailing whitespace from name
        while (isspace((unsigned char)*name))
            name++;
        char *end = name + strlen(name) - 1;
        while (end > name && isspace((unsigned char)*end))
        {
            *end = '\0';
            end--;
        }

        // Trim leading and trailing whitespace from value
        while (isspace((unsigned char)*value))
            value++;
        end = value + strlen(value) - 1;
        while (end > value && isspace((unsigned char)*end))
        {
            *end = '\0';
            end--;
        }

        // Store the header
        if (store_http_header(request, name, value) != 0)
        {
            fprintf(stderr, "Error: Failed to store header: %s: %s\n", name, value);
            free(request_copy);
            cleanup_http_request(request);
            return -1;
        }
    }

    // If there's a body, parse it based on Content-Length
    const char *body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start)
    {
        body_start += 4; // Move past "\r\n\r\n"
        size_t body_len = request_length - (body_start - raw_request);
        if (body_len > 0)
        {
            store_http_body(request, body_start, body_len);
        }
    }

    free(request_copy);
    return 0;
}
