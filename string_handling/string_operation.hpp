#ifndef STRING_LINE
#define STRING_LINE



#include <cstddef>

// 解析一行http报文  就是原http报文；  包含\r\n
int parseHttpRequestLine(int sock, char *buf, int size);
// 该函数用于处理文件头部信息
int handleFileHeader(char *buf, int info_size, char *info, char *flag_info);
// 处理文件体：client是客户端套接字，file为文件路径，flag_boundary结束标志符，结束标志符大小
int processFileBody(int client, const char *file, const char *flag_boundary, size_t boundary_size);
// 如果文件不存在，创建一个新文件
void ensureFileExists(const char *path);

#endif