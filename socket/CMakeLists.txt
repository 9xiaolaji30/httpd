add_library(socket
    socket_connection.cpp
    socket_connection.hpp
)

target_link_libraries(socket      
        PRIVATE http
        PRIVATE string_handling      
        PRIVATE script_executor_project   
        PRIVATE loguru
)


target_include_directories(socket
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE string_handling
        PRIVATE http
        PRIVATE script_executor_project
        PRIVATE loguru
)

