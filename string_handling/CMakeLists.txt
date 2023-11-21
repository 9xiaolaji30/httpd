add_library(string_handling
    string_operation.hpp
    string_operation.cpp
)

target_include_directories(string_handling
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE loguru

)

target_link_libraries(string_handling      
        PRIVATE loguru
)

