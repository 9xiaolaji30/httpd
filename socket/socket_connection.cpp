#include <iostream>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h> // 包含 sockaddr_in 结构体的头文件
#include <loguru.hpp>

#include "string_operation.hpp"
#include "http_parser.hpp"
#include "script_executor.hpp"
#include "socket_connection.hpp"

// 执行脚本路径
#define SH "./test.sh"

#define BUF_SIZE 1024
#define BOUNDAR 2048

#define SCRIPT_SUCCESS 0
#define HEADER_PARSE_SUCCESS 0
#define RESET_FLAG_TO_ZERO 0
#define NOT_A_TARGET_FILE -1
#define FILE_READ_AND_WRITE_SUCCESS 1


#define FILEPATH "/tmp/accepted"

#define FILE_KEY "chuyu"

int startServer(const int port) {
#ifndef NDEBUG
    LOG_SCOPE_FUNCTION(INFO);
#endif
    const int httpd = socket(PF_INET, SOCK_STREAM, 0);
    if (httpd == -1) {
        LOG_F(ERROR, "failed to create socket");
        exit(-1);
    }
    const struct sockaddr_in name = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY)}};
    const int bind_result = bind(httpd, (struct sockaddr *)&name, sizeof(name));
    if (bind_result < 0) {
        LOG_F(ERROR, "failed to bind to port %d", port);
        exit(-1);
    }
    const int start_listening_result = listen(httpd, 5);
    if (start_listening_result < 0) {
        LOG_F(ERROR, "failed to start server on port %d", port);
        exit(-1);
    }
    LOG_F(INFO, "server started listening on port %d", port);
    return (httpd);
}


// 这个函数尝试从套接字中读取HTTP请求的方法和URL
int retriveMethodAndUrl(int client_sock, char *method, char *url) {
    char request_line[REQUEST_LINE_SIZE];
    int bytes_received = recv(client_sock, request_line, REQUEST_LINE_SIZE - 1, 0);
    if (bytes_received <= 0) {
        // 发生了读取错误或对方关闭了连接
        return SOCKET_ERROR;
    }

    // 确保字符串以null字符结束
    request_line[bytes_received] = '\0';

    // 使用sscanf来解析方法和URL
    // 注意：这里假设请求行的格式是 "方法 URL HTTP版本"
    int num_scanned = sscanf(request_line, "%s %s", method, url);

    // 检查是否成功扫描了两个参数
    if (num_scanned != 2) {
        // 解析失败
        return SOCKET_ERROR;
    }

    // 检查方法和URL是否过长
    if (strlen(method) >= METHOD_SIZE || strlen(url) >= URL_SIZE) {
        // 方法或URL过长
        return URL_TOO_LONG;
    }

    return 0; // 成功
}


void handleClientRequest(const int from_client) {
#ifndef NDEBUG
    LOG_SCOPE_FUNCTION(INFO);
#endif
    // int client = *from_client;
    char buf[BOUNDAR];
    char method[BUF_SIZE];
    // 获取一行HTTP报文数据
    memset(buf, 0, sizeof(buf));    // 清空数组
    memset(method, 0, sizeof(buf)); // 清空数组
    parseHttpRequestLine(from_client, buf, sizeof(buf));
    // 每个字段用空白字符相连
    size_t i = 0, j = 0;
    while (!isspace(buf[j]) && (i < sizeof(method) - 1)) {
        method[i] = buf[j];
        i++;
        j++;
    }
    method[i] = '\0';
    if (strcasecmp(method, "POST") == 0) {
        handlePostRequest(from_client);
    } else if (strcasecmp(method, "GET") == 0) {
        const char *scriptPath = SH; // 脚本文件的路径
        char recv_buffer[BUF_SIZE];  // 返回客户端缓冲区
        char errorMessage[BUF_SIZE]; // 存储标准错误输出的缓冲区

        const int return_code = executeScript(scriptPath, errorMessage, sizeof(errorMessage));
        snprintf(recv_buffer, sizeof(recv_buffer), "The return value of %s is: %d \nstdError: %s", SH, return_code, errorMessage);
        // 根据返回值发送不同的响应
        if (return_code == SCRIPT_SUCCESS) {
            send_200_response(from_client, recv_buffer);
        } else {
            send_500_response(from_client, recv_buffer);
        }
    }
    close(from_client); 
    DLOG_F(INFO, "Client has been closed");
    return;
}

// 实现上传文件
void handlePostRequest(int client) {
#ifndef NDEBUG
LOG_SCOPE_FUNCTION(INFO);
#endif
    char file_path[BUFSIZ] = FILEPATH;  // 请替换成实际的文件路径
    char buf[BUFSIZ];                   // 临时区
    char file_key[BUF_SIZE];                // key值
    char boundary[BUF_SIZE];            // http头部信息： boundary
    char star_boundary[BOUNDAR];        // 辅助界定符
    char end_boundary[BOUNDAR];         // 辅助界定符
    char flag_boundary[BOUNDAR];        // 辅助界定符
    int numchars = 0;                   // 一次读取一行http数据大小
    int flag = 0;                       // 处理请求标志位

    memset(buf, 0, sizeof(buf));           
    memset(boundary, 0, sizeof(boundary)); 
    flag = parseHttpHeader(client, boundary, sizeof(boundary));
    DLOG_F(INFO, "%d 处理头部信息结束 boundary:%s   \n", __LINE__, boundary);
    if (flag == HEADER_PARSE_SUCCESS) { 
        DLOG_F(INFO, "parseHttpHeader has failed");
        return;
    }

    snprintf(star_boundary, sizeof(star_boundary), "%s%s%s", "--", boundary, "\r\n");
    snprintf(end_boundary, sizeof(end_boundary), "%s%s%s", "--", boundary, "--\r\n");
    snprintf(flag_boundary, sizeof(flag_boundary), "%s%s", "\r\n--", boundary);
    flag = RESET_FLAG_TO_ZERO ; 
    while (1) {
        numchars = parseHttpRequestLine(client, buf, sizeof(buf)); // 获取一行字符串；
        if (strcmp(buf, star_boundary) == 0) {
            // 处理文件头部信息
            memset(file_key, 0, sizeof(file_key));
            processFileHeader(client, file_key, sizeof(file_key));
            if (strcmp(FILE_KEY, file_key) == 0) 
            {
                flag = processFileBody(client, file_path, flag_boundary, strlen(flag_boundary));
                if (flag == FILE_READ_AND_WRITE_SUCCESS) {
                    LOG_F(INFO, "Write successful and set permissions to %s\n", FILEPATH);
                    break;
                } else {
                    LOG_F(ERROR, "open file or connect  error  ");
                    exit(-1);
                }
            } else { 
                flag = NOT_A_TARGET_FILE;
                continue;
            }
        } else if (strcmp(buf, end_boundary) == 0) 
        {
            flag = NOT_A_TARGET_FILE;
            break;
        }
    }
    if (flag == FILE_READ_AND_WRITE_SUCCESS) {
        send_204_response(client);
    } else {
        send_406_response(client);
    }
}

#ifndef NDEBUG
LOG_SCOPE_FUNCTION(INFO);
#endif
int parseHttpHeader(const int client_socket, char *boundary, int boundary_size) {
    int numchars = 0;            
    int boundary_length;       
    char buf[BUFSIZ];            
    char *boundary_start = NULL;
    memset(buf, 0, sizeof(buf)); 
    numchars = parseHttpRequestLine(client_socket, buf, sizeof(buf));
    while ((numchars > 0) && strcmp("\n", buf)) {
        if ((*boundary) != '\0') { 
            return true;
        }
        if ((boundary_start = strstr(buf, "boundary=")) != NULL) {
            boundary_start += strlen("boundary=");
            char *boundary_end = strchr(boundary_start, '\r');
            if (!boundary_end) {
                return false;
            }
            boundary_length = boundary_end - boundary_start;
            if (boundary_length < boundary_size) {
                strncpy(boundary, boundary_start, boundary_length);
                boundary[boundary_length] = '\0';
            } else {
                return false;
            }
        }
        memset(buf, 0, sizeof(buf));                              
        numchars = parseHttpRequestLine(client_socket, buf, sizeof(buf)); 
    }
    return false;
}

// 处理文件头部信息
#ifndef NDEBUG
LOG_SCOPE_FUNCTION(INFO);
#endif
void processFileHeader(int client, char *file_key, int file_name_size) {
    int isFileInfoProcessed;                         
    char buf[BUFSIZ];                 
    char flag_info[BUFSIZ] = "name="; 
    while (1) {
        if ((*file_key) != '\0') { 
            break;
        }
        isFileInfoProcessed = handleFileHeader(buf, file_name_size, file_key, flag_info);
        parseHttpRequestLine(client, buf, sizeof(buf)); 
    }
    if (isFileInfoProcessed == 0) {
        DLOG_F(INFO, "解析文件头部信息失败：file_info   has   failed\n");
    }
    parseHttpRequestLine(client, buf, sizeof(buf)); 
}
