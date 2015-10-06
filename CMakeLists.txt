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
add_subdirectory(libunwind)
add_subdirectory(libsystem)
add_subdirectory(keymgr)
add_subdirectory(duct/src)
add_subdirectory(libstdcxx)
add_subdirectory(external/libobjc2)

######################
# libc++ & libc++abi #
######################
set(CMAKE_CXX_FLAGS_SAVED ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc -nostdinc++")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libcxx/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/include/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcxxabi/include)
set(LIBCXXABI_LIBCXX_PATH "{CMAKE_CURRENT_SOURCE_DIR}/libcxx")
set(LIBCXXABI_LIBCXX_INCLUDES "-I${CMAKE_CURRENT_SOURCE_DIR}/libcxx/include")

add_subdirectory(libcxxabi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED} -std=c++11 -msse -msse2 -msse3 -nostdinc -nostdinc++ -include ${CMAKE_CURRENT_SOURCE_DIR}/libc/include/xlocale.h")
set(LIBCXX_HAS_STDCXX11_FLAG TRUE)
add_definitions(-D_USE_EXTENDED_LOCALES_)
add_subdirectory(libcxx)
