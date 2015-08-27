project(darling-libsystem_init)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--warn-unresolved-symbols -Wl,--unresolved-symbols=report-all")

include_directories(${CMAKE_SOURCE_DIR}/libc/fbsdcompat)
include_directories(${CMAKE_SOURCE_DIR}/kernel/libsyscall/wrappers)

if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-Ddlsym=__darwin_dlsym)

set(libsystem_sources
	init.c
	dummy.c
)

add_library(system SHARED ${libsystem_sources})
target_link_libraries(system system_malloc system_c system_kernel keymgr system_m)
