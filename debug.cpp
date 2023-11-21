#include <iostream>
//打印所有的字符包括   \r\n\0
void printStringCharacters(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\r') {
            printf("\\r"); // 打印 \r
        } else if (str[i] == '\n') {
            printf("\\n"); // 打印 \n
        } else if (str[i] == '\0') {
            printf("\\0"); // 打印 \0
        } else {
            printf("%c", str[i]); // 打印普通字符
        }
    }
    printf("\n"); // 最后换行
}