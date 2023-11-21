

#ifndef ROUTER_H
#define ROUTER_H

#include <cstdint>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>




enum class HttpMehthod {
    GET,
    POST
};

enum class ResponseHandler {
    NO_CONTENT_204,
    JSON_200,
    FILE_200
};

typedef bool (*url_recognizer)(const HttpMehthod, const char *url);

typedef ResponseHandler (*text_request_handler)(const char *url,
                                                const char *const http_headers[],
                                                const int length,
                                                char const *const request_body,
                                                uint8_t *response_body);

typedef ResponseHandler (*multipart_request_handler)(const char *url,
                                                     const char *const http_headers[],
                                                     const int n_pairs,
                                                     char const *keys[],
                                                     const uint8_t *const values[],
                                                     const int values_sizes[],
                                                     uint8_t *response_body);

void addRoute(const url_recognizer recognizer,
              const text_request_handler handler);

void addRoute(const url_recognizer recognizer,
              const multipart_request_handler handler);

void startRouting(const int http_socket_descriptor);

#endif

