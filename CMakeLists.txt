cmake_minimum_required(VERSION 3.22.1)

include(build/arm_linux_4.8/usr/share/buildroot/toolchainfile.cmake)
# set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
# set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_STRIP arm_linux_4.8/usr/bin/arm-linux-strip)

project(Microcontroller)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# arm不支持
add_compile_definitions(LOGURU_STACKTRACES=0)

if( "${ENV}" STREQUAL "development" )
    message("building for development")
    add_compile_options(-O2 -ldl -lpthread)
else()
    message("building for production")
    add_compile_definitions(NDEBUG=1)
    add_compile_options(-O2)
endif()

add_subdirectory(3rdparty)
add_subdirectory(string_handling)
add_subdirectory(http)
add_subdirectory(script_executor_project)
add_subdirectory(socket)


set(APP_TARGETS
    httpd)

foreach(TARGET ${APP_TARGETS})
    add_executable(${TARGET} "${TARGET}.cpp")
    target_link_libraries(${TARGET}
                        PRIVATE socket
                        PRIVATE http
                        PRIVATE string_handling      
                        PRIVATE script_executor_project  
                        PRIVATE loguru
    )
    target_include_directories(${TARGET}
                        PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                        PRIVATE socket
                        PRIVATE http
                        PRIVATE string_handling      
                        PRIVATE script_executor_project
                        PRIVATE loguru
    )
    add_custom_command(
        TARGET "${TARGET}" POST_BUILD
        DEPENDS "${TARGET}"
        COMMAND ${CMAKE_STRIP} ARGS --strip-all $<TARGET_FILE:${TARGET}>
    )
endforeach(TARGET)
