#include "router.hpp"
#include "loguru.hpp"
#include "string_operation.hpp"
#include "http_parser.hpp"
#include "script_executor.hpp"
#include "socket_connection.hpp"

#define DEFAULT_TOUTER_NUMBER 20
#define ALLOWED_HTTP_HEADERS 10
#define HTTP_HEADER_LENGTH 20

#define HTTP_BODY_MAX_SIZE 1000000

static int n_routes = 0;

uint8_t request_body_buffer[HTTP_BODY_MAX_SIZE];
uint8_t response_body_buffer[HTTP_BODY_MAX_SIZE];

url_recognizer recognizers[DEFAULT_TOUTER_NUMBER];
text_request_handler text_request_handlers[DEFAULT_TOUTER_NUMBER];
multipart_request_handler multipart_request_handlers[DEFAULT_TOUTER_NUMBER];

void addRoute(const url_recognizer recognizer,
              const text_request_handler handler) {
    recognizers[n_routes] = recognizer;
    text_request_handlers[n_routes] = handler;
    multipart_request_handlers[n_routes] = nullptr;
    n_routes++;
}

void addRoute(const url_recognizer recognizer,
              const multipart_request_handler handler) {
    recognizers[n_routes] = recognizer;
    text_request_handlers[n_routes] = nullptr;
    multipart_request_handlers[n_routes] = handler;
    n_routes++;
}

static char http_headers[ALLOWED_HTTP_HEADERS][HTTP_HEADER_LENGTH];

static void processRequest(const int client_sock) {
    char method[10];
    char url[100];
    const int retrive_result = retriveMethodAndUrl(client_sock, method, url);
    if (retrive_result == URL_TOO_LONG) {
        write406Response(client_sock);
        close(client_sock);
        return;
    }
    if (retrive_result == SOCKET_ERROR) {
        close(client_sock);
        LOG_F(ERROR, "error reading url from client socket");
        exit(-1);
    }
    const HttpMehthod http_method = toHttpMethod(method);
    int ith_route_to_handle = -1;
    for (int i = 0; i < n_routes; i++) {
        if (recognizers[i](http_method, url)) {
            ith_route_to_handle = i;
            break;
        }
    }
    if (-1 == ith_route_to_handle) {
        write404Response(client_sock);
        close(client_sock);
        return;
    }
    const int n_headers = retriveHeaders(client_sock, http_headers);
    if (n_headers == SOCKET_ERROR) {
        close(client_sock);
        LOG_F(ERROR, "error reading url from client socket");
        exit(-1);
    }

    int request_body_length = 0;// 定义变量以存储请求体长度和键值对数组
    char keys[5][10];    // 假设最多有5个键，每个键的最大长度为10个字符
    int values_sizes[5]; // 存储相应值的大小的数组

    // 检查HTTP方法是否不是GET
    if (http_method != HttpMethod::GET) {
        // 如果Content-Type是multipart/form-data，解析多部分数据
        if (http_headers["Content-Type"].equals("multipart/form-data")) {
            request_body_length = retrieveAndParseMultipart(client_sock, keys, values_sizes, request_body_buffer);
        } else {
            // 否则，解析文本数据
            request_body_length = retrieveAndParseText(client_sock, request_body_buffer);
            // 检查解析后的文本长度是否与Content-Length头指定的长度一致
            CHECK_F(request_body_length == http_headers["Content-Length"], "parsed text length = header-indicated length");
        }
    }

    // 根据请求路由选择相应的响应处理器
    const ResponseHandler response_handler = text_request_handlers[ith_route_to_handle] != nullptr ?
                                                 text_request_handlers[ith_route_to_handle](url, http_headers, request_body_length, request_body_buffer, response_body_buffer) :
                                                 multipart_request_handlers[ith_route_to_handle](url, http_headers, request_body_length, keys, values_sizes, request_body_buffer, response_body_buffer);

    // 根据处理器的返回结果执行相应的操作
    if (response_handler == ResponseHandler::NO_CONTENT_204) {
        // 返回204响应
        write204Response(client_sock);
        close(client_sock);
        return;
    }
    if (response_handler == ResponseHandler::JSON_200) {
        // 返回200 JSON响应
        write200JsonResponse(client_sock, response_body_buffer);
        close(client_sock);
        return;
    }
}

void startRouting(const int http_socket_descriptor) {
    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof(client_name);
    while (1) {
        const int client_sock = accept(http_socket_descriptor, (struct sockaddr *)&client_name,
                                       &client_name_len);
        if (client_sock == -1) {
            LOG_F(ERROR, "fatal: failed to get client connection");
            exit(-1);
        }
        processRequest(client_sock);
    }
}

HttpMehthod toHttpMethod(const char *method) {
    if (strcmp(method, "GET") == 0) {
        return HttpMehthod::GET;
    } else if (strcmp(method, "POST") == 0) {
        return HttpMehthod::POST;
    }
}

// 读取一行数据，直到遇到换行符
int readLine(int sockfd, char *buffer, int maxlen) {
    int n, rc;
    char c, *ptr;

    ptr = buffer;
    for (n = 1; n < maxlen; n++) {
        if ((rc = recv(sockfd, &c, 1, 0)) == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break; // 检测到换行符，停止读取
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; // EOF，无数据读取
            } else {
                break; // EOF，已经读取了一些数据
            }
        } else {
            return SOCKET_ERROR; // 错误，返回SOCKET_ERROR
        }
    }

    *ptr = 0; // null terminate
    return n; // 返回读取的字节数
}

// retriveHeaders函数实现
int retriveHeaders(int client_sock, char headers[][HTTP_HEADER_LENGTH]) {
    int n_headers = 0;
    char line[REQUEST_LINE_SIZE];

    while (readLine(client_sock, line, REQUEST_LINE_SIZE) > 0) {
        // 检查是否是头部结束的空行
        if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) {
            break; // 头部读取完毕
        }

        if (n_headers >= ALLOWED_HTTP_HEADERS) {
            // 达到头部数量上限
            break;
        }

        // 截取line以适应HTTP_HEADER_LENGTH的大小限制
        strncpy(headers[n_headers], line, HTTP_HEADER_LENGTH - 1);
        headers[n_headers][HTTP_HEADER_LENGTH - 1] = '\0'; // 确保字符串以null结尾
        n_headers++;
    }

    return n_headers;
}
