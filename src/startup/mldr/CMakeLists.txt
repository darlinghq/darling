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
	fat_slice.c
	exec_policy.c
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

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
	target_sources(mldr PRIVATE
		fex/fex_elfcalls.c
		fex/fex_syscall.c
		fex/fex_engine.cpp
	)
	set_source_files_properties(fex/fex_engine.cpp PROPERTIES COMPILE_FLAGS "-std=c++17")
endif()

target_compile_options(mldr PRIVATE -pthread)
target_link_options(mldr PRIVATE -pthread)

if (DARLING_ANDROID_HOST)
	# Bionic has no separate librt; its POSIX semaphores come from a separate library where it is provided.
	find_library(ANDROID_POSIX_SEMAPHORE_LIB android-posix-semaphore)
	target_link_libraries(mldr PRIVATE -ldl mldr_dserver_rpc -lm)
	if (ANDROID_POSIX_SEMAPHORE_LIB)
		target_link_libraries(mldr PRIVATE ${ANDROID_POSIX_SEMAPHORE_LIB})
	endif()
else()
	target_link_libraries(mldr PRIVATE -lrt -ldl mldr_dserver_rpc -lm)
endif()

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

add_executable(mldr_fat_slice_test ${CMAKE_SOURCE_DIR}/tests/mldr/fat_slice.c fat_slice.c)
target_include_directories(mldr_fat_slice_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_fat_slice COMMAND mldr_fat_slice_test)

add_executable(mldr_thread_stack_test ${CMAKE_SOURCE_DIR}/tests/mldr/thread_stack.c elfcalls/thread_stack.c)
target_include_directories(mldr_thread_stack_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/elfcalls)
add_test(NAME mldr_thread_stack COMMAND mldr_thread_stack_test)
set(mldr_commpage_test_sources ${CMAKE_SOURCE_DIR}/tests/mldr/commpage.c commpage.c)
if (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
	list(APPEND mldr_commpage_test_sources cpu_caps_x86.c)
endif()
add_executable(mldr_commpage_test ${mldr_commpage_test_sources})
target_include_directories(mldr_commpage_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_commpage COMMAND mldr_commpage_test)
set_tests_properties(mldr_commpage PROPERTIES SKIP_RETURN_CODE 77)

if (BUILD_TARGET_32BIT)
	add_executable(mldr32_commpage_test ${mldr_commpage_test_sources})
	target_include_directories(mldr32_commpage_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_compile_options(mldr32_commpage_test PRIVATE -m32)
	target_link_options(mldr32_commpage_test PRIVATE -m32)
	add_test(NAME mldr32_commpage COMMAND mldr32_commpage_test)
	set_tests_properties(mldr32_commpage PROPERTIES SKIP_RETURN_CODE 77)
endif()

add_executable(mldr_tsd_rewrite_test ${CMAKE_SOURCE_DIR}/tests/mldr/tsd_rewrite.c)
target_include_directories(mldr_tsd_rewrite_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_tsd_rewrite COMMAND mldr_tsd_rewrite_test)

add_executable(mldr_segment_mapping_test ${CMAKE_SOURCE_DIR}/tests/mldr/segment_mapping.c exec_policy.c)
target_include_directories(mldr_segment_mapping_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_segment_mapping COMMAND mldr_segment_mapping_test)

if (NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
	add_executable(mldr_cpu_caps_x86_test ${CMAKE_SOURCE_DIR}/tests/mldr/cpu_caps_x86.c cpu_caps_x86.c)
	target_include_directories(mldr_cpu_caps_x86_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	add_test(NAME mldr_cpu_caps_x86 COMMAND mldr_cpu_caps_x86_test)
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
	add_executable(mldr_fex_bridge_test ${CMAKE_SOURCE_DIR}/tests/mldr/fex_bridge.c fex/fex_elfcalls.c fex/fex_syscall.c)
	target_include_directories(mldr_fex_bridge_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_compile_options(mldr_fex_bridge_test PRIVATE -pthread)
	target_link_options(mldr_fex_bridge_test PRIVATE -pthread)
	target_link_libraries(mldr_fex_bridge_test PRIVATE -ldl)
	add_test(NAME mldr_fex_bridge COMMAND mldr_fex_bridge_test)

	add_executable(mldr_fexcore_paths_test ${CMAKE_SOURCE_DIR}/tests/mldr/fexcore_paths.c
		fex/fex_elfcalls.c fex/fex_syscall.c fex/fex_engine.cpp)
	target_include_directories(mldr_fexcore_paths_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	set_target_properties(mldr_fexcore_paths_test PROPERTIES LINKER_LANGUAGE CXX)
	target_compile_options(mldr_fexcore_paths_test PRIVATE -pthread)
	target_link_options(mldr_fexcore_paths_test PRIVATE -pthread)
	target_link_libraries(mldr_fexcore_paths_test PRIVATE -ldl)
	add_test(NAME mldr_fexcore_paths COMMAND mldr_fexcore_paths_test)
endif()

# x86_64 emulation on aarch64 hosts. FEXCore is not vendored: the build needs a FEX
# source tree and an already-configured build tree of it, because the shim links
# FEXCore's static libraries and compiles with FEX's own define and include set.
option(WITH_FEXCORE "Build the darling-fexcore x86_64 emulation shim (aarch64 hosts only)" OFF)

if (WITH_FEXCORE)
	if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
		message(FATAL_ERROR "WITH_FEXCORE needs an aarch64 host: FEXCore emulates x86_64 on ARM")
	endif()
	if (NOT FEXCORE_SOURCE_DIR OR NOT FEXCORE_BUILD_DIR)
		message(FATAL_ERROR
			"WITH_FEXCORE needs -DFEXCORE_SOURCE_DIR=<FEX source tree> and -DFEXCORE_BUILD_DIR=<configured FEX build tree>")
	endif()
	if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		message(FATAL_ERROR "WITH_FEXCORE needs clang++: FEXCore headers use clang-specific attributes")
	endif()

	foreach (fexcore_archive
		${FEXCORE_BUILD_DIR}/FEXCore/Source/libFEXCore.a
		${FEXCORE_BUILD_DIR}/FEXCore/Source/libFEXCore_Base.a
		${FEXCORE_BUILD_DIR}/External/cephes/libcephes_128bit.a
		${FEXCORE_BUILD_DIR}/External/SoftFloat-3e/libsoftfloat_3e.a)
		if (NOT EXISTS ${fexcore_archive})
			message(FATAL_ERROR
				"${fexcore_archive} is missing: FEXCORE_BUILD_DIR must point at a FEX build tree that was fully built")
		endif()
	endforeach()

	add_library(darling-fexcore SHARED
		fex/darling_fexcore_adapter.cpp
		${FEXCORE_SOURCE_DIR}/FEXCore/Source/Utils/AllocatorHooks.cpp
		${FEXCORE_SOURCE_DIR}/Source/Common/HostFeatures.cpp
		${FEXCORE_SOURCE_DIR}/Source/Common/CPUInfo.cpp
		${FEXCORE_SOURCE_DIR}/Source/Common/Linux/LinuxVersion.cpp)

	target_include_directories(darling-fexcore PRIVATE
		${FEXCORE_BUILD_DIR}/FEXCore/Source
		${FEXCORE_SOURCE_DIR}/FEXCore/Source
		${FEXCORE_SOURCE_DIR}/Source
		${FEXCORE_BUILD_DIR}/Source
		${FEXCORE_BUILD_DIR}/generated
		${FEXCORE_SOURCE_DIR}/FEXCore/Source/IncludePrivate
		${FEXCORE_SOURCE_DIR}/FEXCore/include
		${FEXCORE_BUILD_DIR}/include
		${FEXCORE_SOURCE_DIR}/FEXHeaderUtils
		${FEXCORE_SOURCE_DIR}/CodeEmitter
		${FEXCORE_SOURCE_DIR}/External/cephes/include
		${FEXCORE_SOURCE_DIR}/External/SoftFloat-3e/include
		${FEXCORE_SOURCE_DIR}/External/SoftFloat-3e/include/SoftFloat-3e
		${FEXCORE_SOURCE_DIR}/External/unordered_dense/include
		${FEXCORE_SOURCE_DIR}/External/range-v3/include
		${FEXCORE_SOURCE_DIR}/External/zydis/include
		${FEXCORE_SOURCE_DIR}/External/fmt/include)

	target_compile_options(darling-fexcore PRIVATE
		-O2 -std=gnu++20 -fPIC -fno-strict-aliasing -fwrapv -ffunction-sections -fno-exceptions
		-DARCHITECTURE_arm64=1
		-DFEXCORE_HAS_PRESERVE_ALL_ATTR=1
		"-DFEXCORE_PRESERVE_ALL_ATTR=__attribute__((preserve_all))"
		-DFEX_DISABLE_TELEMETRY=1
		-DFEX_HAS_PRESERVE_ALL_ATTR=1
		"-DFEX_PRESERVE_ALL_ATTR=__attribute__((preserve_all))"
		-DFMT_SHARED
		"-DGLOBAL_DATA_DIRECTORY=\"/usr/local/share/fex-emu/\""
		-DHAS_PROGRAM_INVOCATION_NAME=1
		-DHAS_SYSCALL_GETCPU=1
		-DHAS_SYSCALL_GETTID=1
		-DHAS_SYSCALL_RENAMEAT2=1
		-DHAS_SYSCALL_STATX=1
		-DHAS_SYSCALL_TGKILL=1
		"-DINLINE=static inline"
		-DINLINE_LEVEL=4
		-DJIT_ARM64
		-DSOFTFLOAT_BUILTIN_CLZ=1
		-DSOFTFLOAT_FAST_DIV32TO16=1
		-DSOFTFLOAT_FAST_DIV64TO32=1
		-DSOFTFLOAT_FAST_INT64=1
		-D_LARGEFILE64_SOURCE)

	target_link_options(darling-fexcore PRIVATE -Wl,--no-undefined)
	target_link_libraries(darling-fexcore PRIVATE
		-Wl,--start-group
		${FEXCORE_BUILD_DIR}/FEXCore/Source/libFEXCore.a
		${FEXCORE_BUILD_DIR}/FEXCore/Source/libFEXCore_Base.a
		${FEXCORE_BUILD_DIR}/External/cephes/libcephes_128bit.a
		${FEXCORE_BUILD_DIR}/External/SoftFloat-3e/libsoftfloat_3e.a
		-Wl,--end-group
		-lfmt -lxxhash -lpthread -ldl -lm -lrt)

	install(TARGETS darling-fexcore DESTINATION libexec/darling/lib)
endif()

# mldr must refuse to start without a darlingserver socket path instead of falling back
# to an address nothing listens on; with a path it goes on to load the program.
add_test(NAME mldr_sockpath_missing COMMAND ${CMAKE_COMMAND} -E env --unset=__mldr_sockpath $<TARGET_FILE:mldr> /nonexistent)
add_test(NAME mldr_sockpath_empty COMMAND ${CMAKE_COMMAND} -E env --unset=__mldr_sockpath __mldr_sockpath= $<TARGET_FILE:mldr> /nonexistent)
set_tests_properties(mldr_sockpath_missing mldr_sockpath_empty PROPERTIES
	PASS_REGULAR_EXPRESSION "__mldr_sockpath is missing")
add_test(NAME mldr_sockpath_set COMMAND ${CMAKE_COMMAND} -E env --unset=__mldr_sockpath __mldr_sockpath=/nonexistent.sock $<TARGET_FILE:mldr> /nonexistent)
set_tests_properties(mldr_sockpath_set PROPERTIES
	PASS_REGULAR_EXPRESSION "Cannot open /nonexistent")

# Run the real i386 retry path with injected mapping/environment/exec failures.
if (BUILD_TARGET_32BIT)
	add_executable(mldr32_layout_retry_test ${CMAKE_SOURCE_DIR}/tests/mldr/layout_retry.c)
	add_dependencies(mldr32_layout_retry_test generate_dserver_rpc_wrappers rtsig_h)
	target_include_directories(mldr32_layout_retry_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_compile_options(mldr32_layout_retry_test PRIVATE -m32 -ffunction-sections -fdata-sections)
	target_link_options(mldr32_layout_retry_test PRIVATE -m32 -Wl,--gc-sections)
	add_test(NAME mldr32_layout_retry COMMAND mldr32_layout_retry_test)
	set_tests_properties(mldr32_layout_retry PROPERTIES TIMEOUT 15)
endif()

add_executable(mldr_exec_policy_test ${CMAKE_SOURCE_DIR}/tests/mldr/exec_policy.c exec_policy.c)
target_include_directories(mldr_exec_policy_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME mldr_exec_policy COMMAND mldr_exec_policy_test)
