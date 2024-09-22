#ifndef RESPONSE_H
#define RESPONSE_H

#include "my_http.h"
#include <stddef.h> // size_t

typedef enum
{
    HTTP_OK = 200,
    HTTP_NOT_FOUND = 404,
    HTTP_INTERNAL_SERVER_ERROR = 500
} http_status_code;

typedef struct
{
    http_status_code code;
    const char *reason;
} http_status_entry;

typedef struct
{
    const char *name;
    const char *value;
} http_header;

typedef struct
{
    const char *version;
    http_status_entry status;
    http_header headers[HTTP_MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_length;
} http_response;

extern const http_status_entry http_statuses[HTTP_STATUS_COUNT];

/** 
 * Builds an HTTP response string.
 * 
 * @param version HTTP version string (e.g., "HTTP/1.1").
 * @param code HTTP status code.
 * @param body Pointer to the body data.
 * @param length Length of the body data.
 * @return Pointer to a newly allocated string containing the raw HTTP response.
 * The caller is responsible for freeing this memory.
 */
char *build_http_response(const char *version, http_status_code code, const char *body, size_t length, const char *headers);

/**
 * Initializes an http_response structure.
 *
 * @param response Pointer to the http_response structure to initialize.
 * @param version HTTP version string (e.g., "HTTP/1.1").
 */
void init_http_response(http_response *response, const char *version);

/**
 * Cleans up an http_response structure, freeing any allocated memory.
 *
 * @param response Pointer to the http_response structure to clean up.
 */
void cleanup_http_response(http_response *response);

/**
 * Sets the HTTP status for the response.
 *
 * @param response Pointer to the http_response structure.
 * @param code The HTTP status code to set.
 */
void set_http_status (http_response *response, http_status_code code);

/**
 * Finds an HTTP status entry based on the status code.
 *
 * @param code The HTTP status code to find.
 * @return Pointer to the corresponding http_status_entry, or NULL if not found.
 */
const http_status_entry *find_http_status(http_status_code code);

/**
 * Adds a header to the HTTP response.
 *
 * @param response Pointer to the http_response structure.
 * @param name Name of the header.
 * @param value Value of the header.
 * @return 0 on success, non-zero on failure (e.g., if maximum headers exceeded).
 */
int add_http_header (http_response *response, const char *name, const char *value);

/**
 * Sets the body of the HTTP response.
 *
 * @param response Pointer to the http_response structure.
 * @param body Pointer to the body data.
 * @param length Length of the body data.
 */
void set_http_body(http_response *response, const char *body, size_t length);

/**
 * Formats the http_response structure into a raw HTTP response string.
 *
 * @param response Pointer to the http_response structure.
 * @return Pointer to a newly allocated string containing the raw HTTP response.
 *         The caller is responsible for freeing this memory.
 */
char *format_http_response(const http_response *response);

/**
 * Finds an HTTP status code in the http_statuses array.
 *
 * @param code The HTTP status code to find.
 * @return Pointer to the corresponding http_status_entry, or NULL if not found.
 */
const http_status_entry *find_http_status(http_status_code code);

#endif // RESPONSE_H