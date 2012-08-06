project(System.dylib)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM-ATT)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xnu/mach)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xnu/bsd)

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
)

set(bsdkern_SRCS
)

set(machkern_SRCS
	kernel-mach/host.cpp
	kernel-mach/lockset.cpp
	kernel-mach/semaphore.cpp
	kernel-mach/task.cpp
	kernel-mach/vm.cpp
)

add_library(System.dylib SHARED ${libc_SRCS} ${bsdkern_SRCS} ${machkern_SRCS})
# -luuid to make uuid_ functions available for Darwin apps
target_link_libraries(System.dylib -ldl -lpthread -luuid)

install(TARGETS System.dylib DESTINATION lib)

