add_library(script_executor_project
    script_executor.cpp
    script_executor.hpp
)

target_include_directories(script_executor_project
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE loguru

)

target_link_libraries(script_executor_project      
        PRIVATE loguru

)

