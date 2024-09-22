#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "request.h"
#include "response.h"

int handle_request(http_request *request, int client_fd);

#endif // HTTP_HANDLER_H