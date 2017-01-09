project(keymgr)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_SOURCE_DIR}/libc/fbsdcompat)
include_directories(${CMAKE_SOURCE_DIR}/kernel/libsyscall/wrappers)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

set(keymgr_sources
	keymgr.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libkeymgr.dylib")
add_darling_library(keymgr SHARED ${keymgr_sources})
target_link_libraries(keymgr PRIVATE system_c system_dyld)

install(TARGETS keymgr DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)
