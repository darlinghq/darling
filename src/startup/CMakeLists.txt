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

add_executable(darling darling.c launcher-env.c launcher-ids.c launcher-mode.c launcher-prefix.c launcher-server.c)

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

if (DARLING_CROSS_BUILD)
	# rtsig cannot run when it is built for the target, so use the copy built
	# for the build machine. The RT signal range is set by the C library
	# (glibc reserves the first two for itself), which is assumed to be the
	# same kind on both machines; set LINUX_SIGRTMIN/LINUX_SIGRTMAX to the
	# target's values when it is not.
	if (DEFINED LINUX_SIGRTMIN AND DEFINED LINUX_SIGRTMAX)
		file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rtsig.h"
			"#define LINUX_SIGRTMIN ${LINUX_SIGRTMIN}\n#define LINUX_SIGRTMAX ${LINUX_SIGRTMAX}\n")
	else()
		execute_process(COMMAND "${DARLING_HOST_RTSIG}" "${CMAKE_CURRENT_BINARY_DIR}/rtsig.h"
			RESULT_VARIABLE rtsig_res)
		if (NOT rtsig_res EQUAL 0)
			message(FATAL_ERROR "Cannot determine the RT signal range; set LINUX_SIGRTMIN and LINUX_SIGRTMAX")
		endif()
	endif()
	add_custom_target(rtsig_h)
else()
	add_executable(rtsig rtsig.c)
	add_custom_command(OUTPUT rtsig.h DEPENDS rtsig COMMAND ${CMAKE_CURRENT_BINARY_DIR}/rtsig rtsig.h COMMENT "Determining available RT signals")
	add_custom_target(rtsig_h DEPENDS rtsig.h)
endif()

# shellspawn runs inside the prefix (src/shellspawn); its descriptor helpers
# are plain POSIX and are tested on the host
add_executable(shellspawn_fds_test ${CMAKE_SOURCE_DIR}/tests/shellspawn/fds.c)
target_include_directories(shellspawn_fds_test PRIVATE ${CMAKE_SOURCE_DIR}/src/shellspawn)
add_test(NAME shellspawn_fds COMMAND shellspawn_fds_test)

# the choice of the shell shellspawn starts for the user
add_executable(shellspawn_shell_test ${CMAKE_SOURCE_DIR}/tests/shellspawn/shell.c)
target_include_directories(shellspawn_shell_test PRIVATE ${CMAKE_SOURCE_DIR}/src/shellspawn)
add_test(NAME shellspawn_shell COMMAND shellspawn_shell_test)

# the local directory node (src/frameworks/OpenDirectory/CFOpenDirectory) stores
# account changes with this plain C helper
add_executable(od_passwd_file_test
	${CMAKE_SOURCE_DIR}/tests/opendirectory/passwd-file.c
	${CMAKE_SOURCE_DIR}/src/frameworks/OpenDirectory/CFOpenDirectory/src/od_passwd_file.c)
target_include_directories(od_passwd_file_test PRIVATE
	${CMAKE_SOURCE_DIR}/src/frameworks/OpenDirectory/CFOpenDirectory/src)
add_test(NAME od_passwd_file COMMAND od_passwd_file_test)

# the setuid launcher's id switching; the test supplies its own id system calls
add_executable(launcher_ids_test ${CMAKE_SOURCE_DIR}/tests/launcher/ids.c launcher-ids.c)
target_include_directories(launcher_ids_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_ids COMMAND launcher_ids_test)

# what the setuid launcher passes to darlingserver
add_executable(launcher_env_test ${CMAKE_SOURCE_DIR}/tests/launcher/env.c launcher-env.c)
target_include_directories(launcher_env_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_env COMMAND launcher_env_test)

# the launcher's mode selection, install prefix and user name rules, and the process tree walk
add_executable(launcher_mode_test ${CMAKE_SOURCE_DIR}/tests/launcher/mode.c launcher-mode.c launcher-server.c)
target_include_directories(launcher_mode_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_mode COMMAND launcher_mode_test)

# how the rootless launcher examines and joins the container's namespaces
add_executable(launcher_rootless_ns_test ${CMAKE_SOURCE_DIR}/tests/launcher/rootless-ns.c launcher-server.c)
target_include_directories(launcher_rootless_ns_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_rootless_ns COMMAND launcher_rootless_ns_test)

# the setuid launcher's prefix path and socket path checks
add_executable(launcher_prefix_test ${CMAKE_SOURCE_DIR}/tests/launcher/prefix.c launcher-prefix.c)
target_include_directories(launcher_prefix_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_prefix COMMAND launcher_prefix_test)

# the setuid launcher's checks before it joins the server's mount namespace
add_executable(launcher_server_test ${CMAKE_SOURCE_DIR}/tests/launcher/server.c launcher-server.c)
target_include_directories(launcher_server_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_server COMMAND launcher_server_test)

# existing prefixes must be initialized Darling trees; empty directories are safe to initialize
add_executable(launcher_prefix_state_test ${CMAKE_SOURCE_DIR}/tests/launcher/prefix-state.c)
target_include_directories(launcher_prefix_state_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_prefix_state COMMAND launcher_prefix_state_test)

# named profile selection is pure path construction and validation
add_executable(launcher_profile_prefix_test ${CMAKE_SOURCE_DIR}/tests/launcher/profile-prefix.c)
target_include_directories(launcher_profile_prefix_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME launcher_profile_prefix COMMAND launcher_profile_prefix_test)

# Refusing a second init must precede all prefix mutation when a server is live.
find_package(Python3 QUIET COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
	add_test(NAME launcher_missing_init
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/launcher/test-missing-init.py
			${CMAKE_SOURCE_DIR}/src/startup/darling.c --run)
endif()
# shellspawn's accept loop, with shellspawn.c built for the host as it is and
# accept, usleep, perror and fork replaced through the linker. shellspawn.c
# includes <sys/event.h> without using kqueue, so an empty one stands in.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/shellspawn-host-include/sys/event.h
	"/* empty: shellspawn.c makes no kqueue calls */\n")
add_executable(shellspawn_accept_test
	${CMAKE_SOURCE_DIR}/tests/shellspawn/accept-loop.c
	${CMAKE_SOURCE_DIR}/src/shellspawn/shellspawn.c
	${CMAKE_SOURCE_DIR}/src/shellspawn/duct_signals.c)
set_source_files_properties(${CMAKE_SOURCE_DIR}/src/shellspawn/shellspawn.c
	PROPERTIES COMPILE_DEFINITIONS main=shellspawn_main)
target_include_directories(shellspawn_accept_test PRIVATE
	${CMAKE_SOURCE_DIR}/src/shellspawn
	${CMAKE_CURRENT_BINARY_DIR}/shellspawn-host-include)
target_link_options(shellspawn_accept_test PRIVATE
	-Wl,--wrap=accept,--wrap=usleep,--wrap=perror,--wrap=fork)
add_test(NAME shellspawn_accept COMMAND shellspawn_accept_test)

# darling-prefsd runs inside the prefix (src/prefsd); its checks on the
# preferences files and values are plain C and are tested on the host
add_executable(prefsd_validate_test ${CMAKE_SOURCE_DIR}/tests/prefsd/validate.c ${CMAKE_SOURCE_DIR}/src/prefsd/prefsd-validate.c)
target_include_directories(prefsd_validate_test PRIVATE ${CMAKE_SOURCE_DIR}/src/prefsd)
add_test(NAME prefsd_validate COMMAND prefsd_validate_test)

# Builds shellspawn.c on the host with a test harness and checks where
# forwarded signals are delivered
find_package(Python3 QUIET COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
	add_test(NAME shellspawn_forwarded_signals
		COMMAND ${CMAKE_COMMAND} -E env CC=${CMAKE_C_COMPILER}
			${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/shellspawn/forwarded-signals.py ${CMAKE_SOURCE_DIR})
endif()

add_subdirectory(mldr)
