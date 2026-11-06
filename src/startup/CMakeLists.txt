project(dyld-bin)

cmake_minimum_required(VERSION 3.13)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -ggdb -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Ttext-segment,0x400000 -Wl,-Tbss,0x410000 -Wl,-Tdata,0x420000")
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" -D_GNU_SOURCE -DMLDR_BUILD)

add_executable(darling darling.c)

target_link_libraries(darling -lutil)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/src/external/darlingserver/include ${CMAKE_SOURCE_DIR}/src/external/darlingserver/include)

install(TARGETS darling DESTINATION bin
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
    SETUID)

# host build of darpath; the Darling build is in src/tools
add_executable(darpath_host ${CMAKE_SOURCE_DIR}/src/tools/darpath.c)
set_target_properties(darpath_host PROPERTIES OUTPUT_NAME darpath)
install(TARGETS darpath_host DESTINATION bin)

add_executable(rtsig rtsig.c)
add_custom_command(OUTPUT rtsig.h DEPENDS rtsig COMMAND ${CMAKE_CURRENT_BINARY_DIR}/rtsig rtsig.h COMMENT "Determining available RT signals")
add_custom_target(rtsig_h DEPENDS rtsig.h)

# shellspawn runs inside the prefix (src/shellspawn); its descriptor helpers
# are plain POSIX and are tested on the host
add_executable(shellspawn_fds_test ${CMAKE_SOURCE_DIR}/tests/shellspawn/fds.c)
target_include_directories(shellspawn_fds_test PRIVATE ${CMAKE_SOURCE_DIR}/src/shellspawn)
add_test(NAME shellspawn_fds COMMAND shellspawn_fds_test)

# Builds shellspawn.c on the host with a test harness and checks where
# forwarded signals are delivered
find_package(Python3 QUIET COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
	add_test(NAME shellspawn_forwarded_signals
		COMMAND ${CMAKE_COMMAND} -E env CC=${CMAKE_C_COMPILER}
			${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/shellspawn/forwarded-signals.py ${CMAKE_SOURCE_DIR})
endif()

add_subdirectory(mldr)
