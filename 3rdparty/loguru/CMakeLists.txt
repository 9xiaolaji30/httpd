
add_library(loguru
    loguru.hpp
    loguru.cpp
)

target_include_directories(loguru
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)