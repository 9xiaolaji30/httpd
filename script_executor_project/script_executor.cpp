#include "script_executor.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <loguru.hpp>

int executeScript(const char *scriptPath, char *errorMessage, size_t errorMessageSize) {
#ifndef NDEBUG
    LOG_SCOPE_FUNCTION(INFO);
#endif
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s 2>&1", scriptPath);
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        LOG_F(ERROR, "popen error");
        exit(-1);        
    }

    if (errorMessage != NULL) {
        errorMessage[0] = '\0';
    }

    char buf[1024];  
    size_t totalOutputSize = 0;  
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        DLOG_F(INFO,"%s",buf);
        if (errorMessage != NULL) {
            strncat(errorMessage, buf, errorMessageSize - totalOutputSize - 1);
            totalOutputSize += strlen(buf);
        }
    }

    const int return_code = pclose(fp);
    return  return_code;
}