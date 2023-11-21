#include "string_operation.hpp"
#include <cstring>
#include <cctype>
#include <cstdio>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <loguru.hpp>

#define FILE_OPEN_ERROR -1
#define RECEIVE_ERROR -1
#define PERMISSION_ERROR -2
#define FOUND_BOUNDARY 1
#define RECV_ERROR_THRESHOLD 0
#define MEMORY_ALLOCATION_ERROR -1


#ifndef NDEBUG
    LOG_SCOPE_FUNCTION(INFO);
#endif
int parseHttpRequestLine(int sock, char *buf, int size)
{
    int i = 0;     
    char c = '\0'; 
    int n;         
    memset(buf, 0, size);

    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0); 
        if (n > 0)
        {
            buf[i] = c; 
            i++;        
        }
        else
        {
            c = '\n'; 
        }
    }
    buf[i] = '\0'; 
    return (i);    
}



#ifndef NDEBUG
    LOG_SCOPE_FUNCTION(INFO);
#endif
int handleFileHeader(char *buf, int info_size, char *info, char *flag_info)
{
    char *info_start = strstr(buf, flag_info);
    if (!info_start)
    {
        return false;
    }
    info_start += strlen(flag_info);               
    char *info_end = strchr(info_start + 1, '\"'); 
    if (!info_end)
    {
        return false;
    }
    int info_length = info_end - (info_start + 1);
    if (info_length < info_size)
    {
        strncpy(info, info_start + 1, info_length);
        info[info_length] = '\0';
        return true;
    }
    else
    {
        return false;
    }
}

int processFileBody(int client, const char *file_path, const char *flag_boundary, size_t boundary_size)
{
    char buf[boundary_size]; 
    ssize_t bytes_received;      
    int found = 0;               
    FILE *fp = NULL;             
    char c;                      
    memset(buf, 0, sizeof(buf)); 
    // 获取目录路径
    char *dirPath = strdup(file_path); 
    if (!dirPath)
    {
        LOG_F(ERROR, "Memory allocation error");
        exit(-1);
    }
    char *lastSlash = strrchr(dirPath, '/'); 
    if (lastSlash != NULL)
    {
        *lastSlash = '\0';         
        ensureFileExists(dirPath); 
    }
    free(dirPath);

    fp = fopen(file_path, "wb+"); 
    if (!fp)
    {
        LOG_F(ERROR, "Open file failed");
        exit(-1);
    }

    bytes_received = recv(client, buf, sizeof(buf), 0); 
    if (bytes_received <= RECV_ERROR_THRESHOLD )
    {
        LOG_F(ERROR, "Receive error or close connection");
        exit(-1);
    }
    
    while (1)
    {
        if (strncmp(buf, flag_boundary, boundary_size) == 0) 
        {
            found = FOUND_BOUNDARY; 
            break;
        }
        fwrite(&buf[0], 1, 1, fp);                
        memmove(buf, buf + 1, boundary_size - 1); 
        bytes_received = recv(client, &c, 1, 0);  
        if (bytes_received <= RECV_ERROR_THRESHOLD )
        {
            LOG_F(ERROR, "Receive error or close connection");
            exit(-1);
        }
        buf[boundary_size - 1] = c;
    }

    fclose(fp);                                       
    if (chmod(file_path, S_IRWXU | S_IRWXG | S_IRWXO) != 0) 
    {
        LOG_F(ERROR, "Error setting file permissions");
        exit(-1);
    }
    return found; 
}


// 路径不存在创建目录
void ensureFileExists(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        if (mkdir(path, 0777) != 0)
        {
            LOG_F(ERROR, "mkdir error");
            exit(-1);
        }
    }
    else if (!S_ISDIR(st.st_mode))
    {
        LOG_F(ERROR, "file_path is not a directory.");
        exit(-1);
    }
}
