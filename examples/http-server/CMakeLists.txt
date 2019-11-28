cmake_minimum_required(VERSION 3.12)

include("${CMAKE_SOURCE_DIR}/cmake/CMakeUtils.cmake")

file_intern("${CMAKE_CURRENT_SOURCE_DIR}/server.js" server_js server_js_file_c)
file_intern( "${CMAKE_CURRENT_SOURCE_DIR}/http.js" http_js http_js_file_c)

add_executable(http_server main.c ${server_js_file_c} ${http_js_file_c})
target_link_libraries(http_server serve_jsb)
