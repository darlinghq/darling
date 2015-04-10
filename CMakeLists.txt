project(darling-libsystem)

cmake_minimum_required(VERSION 2.4.0)

add_definitions(-D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE -DDARLING)
if ((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") AND (${CMAKE_C_FLAGS} MATCHES ".*-m32.*"))
	set(CMAKE_SYSTEM_PROCESSOR "i686")
endif()

message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR}, ${CMAKE_C_FLAGS}")

if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1 -D__LITTLE_ENDIAN__)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1 -D__LITTLE_ENDIAN__)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

add_definitions(-D__APPLE__ -D__DYNAMIC__)

add_subdirectory(bootstrap_cmds)
add_subdirectory(kernel)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libm/Source")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/platform-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/platform-include/pthread")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/kernel-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/compiler-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/duct/include")

add_subdirectory(libc)
add_subdirectory(libm)
add_subdirectory(libinfo)
add_subdirectory(libmalloc)
add_subdirectory(libsystem)
add_subdirectory(keymgr)
add_subdirectory(duct/src)
add_subdirectory(libstdcxx)
