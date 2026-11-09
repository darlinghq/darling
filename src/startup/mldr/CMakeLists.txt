project(mldr)

cmake_minimum_required(VERSION 3.13)

include_directories(include)

set_source_files_properties(${CMAKE_BINARY_DIR}/src/external/darlingserver/src/rpc.c PROPERTIES
	GENERATED TRUE
)

add_library(mldr_dserver_rpc STATIC ${CMAKE_BINARY_DIR}/src/external/darlingserver/src/rpc.c)

add_dependencies(mldr_dserver_rpc generate_dserver_rpc_wrappers)

target_compile_options(mldr_dserver_rpc PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/resources/dserver-rpc-defs.h)
add_dependencies(mldr_dserver_rpc rtsig_h)
target_include_directories(mldr_dserver_rpc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set(mldr_sources
	mldr.c
	socket_bitmap.c
	image_protection.c
	interpreter_path.c
	commpage.c
	elfcalls/elfcalls.c
	elfcalls/native_fork.c
	elfcalls/threads.c
	elfcalls/thread_stack.c
)

if (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
	list(APPEND mldr_sources cpu_caps_x86.c)
endif()

add_executable(mldr ${mldr_sources})

target_compile_options(mldr PRIVATE -pthread)
target_link_options(mldr PRIVATE -pthread)

target_link_libraries(mldr PRIVATE -lrt -ldl mldr_dserver_rpc -lm)

install(TARGETS mldr DESTINATION libexec/darling/usr/libexec/darling)
install(PROGRAMS $<TARGET_FILE:mldr> DESTINATION libexec/darling/bin)

configure_file(darling.conf.in darling.conf @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/darling.conf" DESTINATION lib/binfmt.d)

if (BUILD_TARGET_32BIT)
	add_library(mldr32_dserver_rpc STATIC ${CMAKE_BINARY_DIR}/src/external/darlingserver/src/rpc.c)
	add_dependencies(mldr32_dserver_rpc generate_dserver_rpc_wrappers)
	target_compile_options(mldr32_dserver_rpc PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/resources/dserver-rpc-defs.h)
	target_compile_options(mldr32_dserver_rpc PRIVATE -m32)
	target_link_options(mldr32_dserver_rpc PRIVATE -m32)
	add_dependencies(mldr32_dserver_rpc rtsig_h)
	target_include_directories(mldr32_dserver_rpc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

	add_executable(mldr32 ${mldr_sources})
	target_compile_options(mldr32 PRIVATE -m32)
	target_link_options(mldr32 PRIVATE -m32)
	target_compile_options(mldr32 PRIVATE -pthread)
	target_link_options(mldr32 PRIVATE -pthread)
	target_link_libraries(mldr32 PRIVATE -lrt -ldl mldr32_dserver_rpc)

	install(TARGETS mldr32 DESTINATION libexec/darling/usr/libexec/darling)
endif()

add_executable(mldr_socket_bitmap_test ${CMAKE_SOURCE_DIR}/tests/mldr/socket_bitmap.c socket_bitmap.c)
target_include_directories(mldr_socket_bitmap_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(mldr_socket_bitmap_test PRIVATE -pthread)
target_link_options(mldr_socket_bitmap_test PRIVATE -pthread)
add_test(NAME mldr_socket_bitmap COMMAND mldr_socket_bitmap_test)

add_executable(mldr_native_fork_test ${CMAKE_SOURCE_DIR}/tests/mldr/native_fork.c elfcalls/native_fork.c socket_bitmap.c)
target_include_directories(mldr_native_fork_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(mldr_native_fork_test PRIVATE -pthread)
target_link_options(mldr_native_fork_test PRIVATE -pthread)
add_test(NAME mldr_native_fork COMMAND mldr_native_fork_test)

add_executable(mldr_image_protection_test ${CMAKE_SOURCE_DIR}/tests/mldr/image_protection.c image_protection.c)
target_include_directories(mldr_image_protection_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_image_protection COMMAND mldr_image_protection_test)

add_executable(mldr_interpreter_path_test ${CMAKE_SOURCE_DIR}/tests/mldr/interpreter_path.c interpreter_path.c)
target_include_directories(mldr_interpreter_path_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_interpreter_path COMMAND mldr_interpreter_path_test)

add_executable(mldr_thread_stack_test ${CMAKE_SOURCE_DIR}/tests/mldr/thread_stack.c elfcalls/thread_stack.c)
target_include_directories(mldr_thread_stack_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/elfcalls)
add_test(NAME mldr_thread_stack COMMAND mldr_thread_stack_test)

add_executable(mldr_segment_mapping_test ${CMAKE_SOURCE_DIR}/tests/mldr/segment_mapping.c)
target_include_directories(mldr_segment_mapping_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_segment_mapping COMMAND mldr_segment_mapping_test)

if (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
	add_executable(mldr_cpu_caps_x86_test ${CMAKE_SOURCE_DIR}/tests/mldr/cpu_caps_x86.c cpu_caps_x86.c)
	target_include_directories(mldr_cpu_caps_x86_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	add_test(NAME mldr_cpu_caps_x86 COMMAND mldr_cpu_caps_x86_test)
endif()
