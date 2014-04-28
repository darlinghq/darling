project(libSystem)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if (NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm.*")
enable_language(ASM_NASM)
endif (NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm.*")

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(bsd/wchar.h HAS_BSD_WCHAR_H)

if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
	message(FATAL_ERROR "Clang is the only supported compiler (${CMAKE_CXX_COMPILER} used).")
endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

configure_file(config.h.in libsystem-config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fblocks -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -D_GNU_SOURCE")

#find_path(LIBCXX_INCLUDE_DIR c++/v1/vector)
#if (DEFINED LIBCXX_INCLUDE_DIR)
#    message(STATUS "Found libcxx in ${LIBCXX_INCLUDE_DIR}")
#else (DEFINED LIBCXX_INCLUDE_DIR)
#    message(FATAL_ERROR "libcxx not found")
#endif (DEFINED LIBCXX_INCLUDE_DIR)

#include_directories(${LIBCXX_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmach-o)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu/mach)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu/bsd)

#ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/directmap.nasm"
#	COMMAND "${CMAKE_CURRENT_BINARY_DIR}/../../genfuncmap" ARGS "${CMAKE_CURRENT_SOURCE_DIR}/libc/directmap.lst" "${CMAKE_CURRENT_BINARY_DIR}/directmap.nasm")


set(libc_SRCS
	libc/mac.c
	libc/rune-none.c
	libc/popcount.c
	libc/runetable.c
	libc/stack_protector-obsd.c
	libc/err.c
	libc/errno.cpp
	libc/OSAtomic.cpp
	libc/strmode.c
	libc/sysctl.cpp
	libc/dir.cpp
	libc/string.cpp
	libc/exec.cpp
	libc/uname.cpp
	libc/stdio.cpp
	libc/fopsmisc.cpp
	libc/aio.cpp
	libc/mount.cpp
	libc/nextstep.cpp
	libc/arch.cpp
	libc/qsort.cpp
	libc/misc.cpp
	libc/pthread.cpp
	libc/sysconf.cpp
	libc/net.cpp
	libc/getpwent.cpp
	libc/signals.cpp
	libc/termios.cpp
	libc/attrlist.cpp
	libc/spawn.cpp
	libc/xlocale.cpp
	libc/malloc_zone.cpp
	libc/namemap.cpp
	libc/vis.c
	libc/unvis.c
	libc/math.cpp
	libc/sigsetjmp.cpp
	libc/setjmp.nasm
	libc/confstr.cpp
	libc/progname.cpp
	common/auto.cpp
	common/path.cpp
)

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86.*")
	set(libc_SRCS ${libc_SRCS} libc/setjmp.nasm)
elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm.*")
	set(libc_SRCS ${libc_SRCS} libc/setjmp-arm.s)
endif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86.*")

set(asl_SRCS
	asl/asl.cpp
	asl/ASLClient.cpp
	asl/ASLSyslogClient.cpp
	asl/ASLMsg.cpp
	asl/ASLResponse.cpp
)

set(bsdkern_SRCS
	kernel-bsd/stat.cpp
	kernel-bsd/fopsmisc.cpp
	kernel-bsd/io.cpp
	kernel-bsd/wait.cpp
	kernel-bsd/sockets.cpp
	kernel-bsd/fs.cpp
	kernel-bsd/fcntl.cpp
	kernel-bsd/proc.cpp
	kernel-bsd/mman.cpp
)

set(machkern_SRCS
	kernel-mach/host.cpp
	kernel-mach/lockset.cpp
	kernel-mach/semaphore.cpp
	kernel-mach/task.cpp
	kernel-mach/time.cpp
	kernel-mach/vm.cpp
	kernel-mach/error.cpp
	kernel-mach/Futex.cpp
	kernel-mach/FutexSemaphore.cpp
	kernel-mach/compat.cpp
	kernel-mach/thread.cpp
)

add_library(System SHARED ${libc_SRCS} ${bsdkern_SRCS}
	${machkern_SRCS} ${asl_SRCS} keymgr/keymgr.c)
# -luuid to make uuid_ functions available for Darwin apps
target_link_libraries(System -ldl -lpthread -luuid mach-o -lrt -lssl -lbsd -lutil ${GNUSTEP_OBJC_CFLAGS})
add_dependencies(System mach-o)

install(TARGETS System DESTINATION "lib${SUFFIX}/darling")

