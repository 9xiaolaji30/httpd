#ifndef HTTP_REQUEST
#define HTTP_REQUEST

#include <cstddef>

#define URL_TOO_LONG -2
#define SOCKET_ERROR -1
#define METHOD_SIZE 10
#define URL_SIZE 100

// 假设每个请求行不超过1024个字符
#define REQUEST_LINE_SIZE 1024


int startServer(const int port);
void handleClientRequest(const int  from_client);
void handlePostRequest(int client);
int parseHttpHeader(const int client_socket, char *boundary, int boundary_size);
void processFileHeader(int client, char *key_word, int file_name_size);

int retriveMethodAndUrl(int client_sock, char *method, char *url);


#endif