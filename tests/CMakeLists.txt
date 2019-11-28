cmake_minimum_required(VERSION 3.12)

function(make_test test_name test_file)
	add_executable("${test_name}" t_run.c t_utils.c ${ARGN})
	target_link_libraries("${test_name}" mujs qsortx)
	target_compile_definitions("${test_name}" PRIVATE 
		-DTEST_PATH="${CMAKE_SOURCE_DIR}/tests/${test_file}"
		-DTEST_DIR="${CMAKE_SOURCE_DIR}/tests"
	)
	add_test(NAME "${test_name}" COMMAND "${test_name}")
endfunction()

make_test(test_arraybuffer test_arraybuffer.js test_arraybuffer.c ../jsB_arraybuffer.c)
make_test(test_dataview test_dataview.js test_dataview.c ../jsB_arraybuffer.c ../jsB_dataview.c)
make_test(test_typedarray test_typedarray.js test_typedarray.c ../jsB_arraybuffer.c ../jsB_typedarray.c)
target_link_libraries(test_typedarray qsortx)
make_test(test_libstd test_libstd.js test_libstd.c ../jsB_arraybuffer.c ../jsB_typedarray.c ../jsB_libstd.c)
target_link_libraries(test_libstd qsortx)
make_test(test_eventloop test_eventloop.js test_eventloop.c ../jsB_eventloop.c)
make_test(test_vm test_vm.js test_vm.c ../jsB_vm.c ../jsB_arraybuffer.c)
make_test(test_utils test_utils.js test_utils.c ../jsB_utils.c ../jsB_arraybuffer.c)
make_test(test_eventemitter test_eventemitter.js test_eventemitter.c ../jsB_eventemitter.c)
make_test(test_readstream test_readstream.js test_readstream.c 
	../jsB_readstream.c 
	../jsB_eventemitter.c
	../jsB_arraybuffer.c
	../jsB_typedarray.c
	../jsB_utils.c
	../jsB_eventemitter.c
	../jsB_eventloop.c
	../jsB_timers.c)
