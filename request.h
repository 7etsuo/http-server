#ifndef REQUEST_H
#define REQUEST_H

#include "my_http.h"
#include <stddef.h> // size_t

#define HTTP_METHOD_COUNT 9
typedef enum
{
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_PATCH,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_UNKNOWN
} http_method;

typedef struct
{
    http_method method;
    const char *name;
} http_method_entry;

extern const http_method_entry http_methods[HTTP_METHOD_COUNT];

typedef struct
{
    http_method method;
    char *uri;     // Mutable string for URI
    char *version; // Mutable string for HTTP version
} http_request_line;

typedef struct
{
    char *name;  // (e.g., "Content-Type")
    char *value; // (e.g., "application/json")
} http_request_header;

typedef struct
{
    http_request_line request_line;
    http_request_header headers[HTTP_MAX_HEADERS];
    int header_count;
    char *body; // Mutable string for body
    size_t body_length;
} http_request;

/**
 * Initializes an http_request structure.
 *
 * @param request Pointer to the http_request structure to initialize.
 */
void init_http_request(http_request *request);

/**
 * Cleans up an http_request structure, freeing any allocated memory.
 *
 * @param request Pointer to the http_request structure to clean up.
 */
void cleanup_http_request(http_request *request);

/**
 * Parses a raw HTTP request string into an http_request structure.
 *
 * @param raw_request Pointer to the raw HTTP request string.
 * @param request_length Length of the raw HTTP request string.
 * @param request Pointer to the http_request structure to populate.
 * @return 0 on success, non-zero on failure.
 */
int parse_http_request(const char *raw_request, size_t request_length, http_request *request);

/**
 * Finds an HTTP method enum based on its string representation.
 *
 * @param method_str String representation of the HTTP method.
 * @return Corresponding http_method enum, or HTTP_METHOD_UNKNOWN if not found.
 */
http_method find_http_method(const char *method_str);

/**
 * Stores a header in the HTTP request.
 *
 * @param request Pointer to the http_request structure.
 * @param name Name of the header.
 * @param value Value of the header.
 * @return 0 on success, non-zero on failure (e.g., if maximum headers exceeded).
 */
int store_http_header(http_request *request, const char *name, const char *value);

/**
 * Stores the body of the HTTP request.
 *
 * @param request Pointer to the http_request structure.
 * @param body Pointer to the body data.
 * @param length Length of the body data.
 */
void store_http_body(http_request *request, const char *body, size_t length);

#endif // REQUEST_H
