#include <arpa/inet.h>  
#include <cstdlib>
#include <iostream>
#include <loguru.hpp>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

#include "http_parser.hpp"
#include "socket_connection.hpp"
#include "string_operation.hpp"
#include "router.hpp"

constexpr int HTTP_PORT = 8080;

int main(int argc, char const *argv[]) {
    int server_sock = -1;
    int client_sock = -1;                            
    struct sockaddr_in client_name;                  
    socklen_t client_name_len = sizeof(client_name); 
    server_sock = startServer(HTTP_PORT);   

    startRouting(server_sock);
    
    close(server_sock);
    return 0;
}
