
add_library(http
    http_parser.hpp
    http_parser.cpp
)


target_include_directories(http
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE loguru

)

target_link_libraries(http      
        PRIVATE loguru

)

