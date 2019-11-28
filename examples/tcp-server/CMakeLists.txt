cmake_minimum_required(VERSION 3.12)

include("${CMAKE_SOURCE_DIR}/cmake/CMakeUtils.cmake")

file_intern("${CMAKE_CURRENT_SOURCE_DIR}/server.js" server_js server_js_file_c)

add_executable(tcp_server main.c ${server_js_file_c})
target_link_libraries(tcp_server serve_jsb)
