#ifndef SCRIPT_EXECUTOR
#define SCRIPT_EXECUTOR
#include <cstddef>

int executeScript(const char *scriptPath, char *errorMessage, size_t errorMessageSize);

#endif