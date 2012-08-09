project(System.dylib)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM_NASM)

configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fblocks")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu/mach)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu/bsd)

set(libc_SRCS
	libc/mac.c
	libc/none.c
	libc/popcountdi2.c
	libc/runetable.c
	libc/stack_protector-obsd.c
	libc/err.c
	libc/errno.cpp
	libc/OSAtomic.cpp
	libc/strmode.c
	libc/directmap.asm
	libc/sysctl.cpp
	libc/dir.cpp
	libc/string.cpp
	libc/exec.cpp
	common/path.cpp
)

set(bsdkern_SRCS
	kernel-bsd/stat.cpp
	kernel-bsd/fopsmisc.cpp
	kernel-bsd/io.cpp
)

set(machkern_SRCS
	kernel-mach/host.cpp
	kernel-mach/lockset.cpp
	kernel-mach/semaphore.cpp
	kernel-mach/task.cpp
	kernel-mach/time.cpp
	kernel-mach/vm.cpp
)

add_library(System.dylib SHARED ${libc_SRCS} ${bsdkern_SRCS} ${machkern_SRCS})
# -luuid to make uuid_ functions available for Darwin apps
target_link_libraries(System.dylib -ldl -lpthread -luuid)

install(TARGETS System.dylib DESTINATION lib)

