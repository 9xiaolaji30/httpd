#ifndef HTTP_PARSER  // 防止头文件被多次包含
#define HTTP_PARSER



void writeResponseAndCloseSocket(const int client_socket, const int http_code, const char *response_body);
void write204Response(const int client_socket);
void write404Response(const int client_socket);
void write406Response(const int client_socket);
void write200JsonResponse(const int client_socket, const char *response);
void write500JsonResponse(const int client_socket, const char *response);

#endif 