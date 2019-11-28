cmake_minimum_required(VERSION 3.12)
project(serverjs VERSION 0.1.0 LANGUAGES C CXX)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_GNU_SOURCE")

add_library(server_jsb_h INTERFACE)
target_include_directories(server_jsb_h INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(serve_jsb STATIC 
	jsB_arraybuffer.c
	jsB_arraybuffer.h
	jsB_dataview.c
	jsB_dataview.h
	jsB_libstd.c
	jsB_libstd.h
	jsB_typedarray.c
	jsB_typedarray.h
	jsB_eventloop.c
	jsB_eventloop.h
	jsB_eventemitter.h
	jsB_eventemitter.c
	jsB_http.c
	jsB_http.h
	jsB_tcp.c
	jsB_tcp.h
	jsB_vm.h
	jsB_vm.c
	jsB_module.c
	jsB_module.h
	jsB_utils.c
	jsB_utils.h
	jsB_timers.c
	jsB_timers.h
	jsB_readstream.h
	jsB_readstream.c
	tests/t_utils.c
	tests/t_utils.h
)
target_link_libraries(serve_jsb mujs qsortx server_jsb_h)

add_executable(serve main.c)
target_link_libraries(serve serve_jsb)

add_subdirectory(deps/mujs)
add_subdirectory(deps/qsortx)
add_subdirectory(tests)
add_subdirectory(examples/tcp-server)
add_subdirectory(examples/http-server)
