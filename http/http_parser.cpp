#include "http_parser.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>

#include <loguru.hpp>

void writeResponseAndCloseSocket(const int client_socket, const int http_code, const char *response_body) {
#ifndef NDEBUG
LOG_SCOPE_FUNCTION(INFO);
#endif
    const char *status_text;
    char response[4096];

    switch (http_code) {
    case 200:
        status_text = "200 OK";
        break;
    case 204:
        status_text = "204 No Content";
        break;
    case 404:
        status_text = "404 Not Found";
        break;
    case 406:
        status_text = "406 Not Acceptable";
        break;
    case 500:
        status_text = "500 Internal Server Error";
        break;
    }
    const int length = snprintf(response, sizeof(response),
                          "HTTP/1.1 %s\r\n"
                          "Content-Length: %lu\r\n"
                          "Content-Type: text/plain\r\n\r\n"
                          "%s",
                          status_text, (unsigned long)strlen(response_body), response_body);

    if (length > 0 && length < sizeof(response)) {
        send(client_socket, response, length, 0);
    } else {
        LOG_F(ERROR, "response   overflow ");
        exit(-1);
    }

    sleep(1); 
}

void write204Response(int client_socket) {
    writeResponseAndCloseSocket(client_socket, 204, "");
}

void write404Response(int client_socket) {
    writeResponseAndCloseSocket(client_socket, 404, "");
}

void write406Response(int client_socket) {
    writeResponseAndCloseSocket(client_socket, 406, "");
}


void write200JsonResponse(int client_socket, const char *response_body) {
    writeResponseAndCloseSocket(client_socket, 200, response_body);
}

void write500JsonResponse(int client_socket, const char *response_body) {
    writeResponseAndCloseSocket(client_socket, 500, response_body);
}