cmake_minimum_required(VERSION 3.12)

include("${CMAKE_SOURCE_DIR}/cmake/CMakeUtils.cmake")

set(server_js_file "${CMAKE_CURRENT_SOURCE_DIR}/server.js")
set(server_js_file_c "${CMAKE_CURRENT_BINARY_DIR}/server.js.c")

file_was_changed(${server_js_file} wasChanged)
if ("${wasChanged}" STREQUAL "1")
	file_generate_rc(${server_js_file} ${server_js_file_c} bootstrap_js 1)
endif()

add_executable(tcp_server main.c ${server_js_file_c})
target_link_libraries(tcp_server serve_jsb)
