project(darling-src)

cmake_minimum_required(VERSION 2.4.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(mig)

add_definitions(-D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE -DDARLING)
set(DARLING TRUE)

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

if (CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -m32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")
	SET(CMAKE_C_SIZEOF_DATA_PTR 4)
	SET(SUFFIX "32")
endif(CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
if (CMAKE_INSTALL_LIBDIR STREQUAL "lib64")
	set(SUFFIX "64")
endif (CMAKE_INSTALL_LIBDIR STREQUAL "lib64")

add_definitions(-D__APPLE__ -D__DYNAMIC__)
configure_file(darling-config.h.in darling-config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(bootstrap_cmds)
add_subdirectory(kernel)
add_subdirectory(util)
add_subdirectory(libmach-o)
add_subdirectory(libdyld)
add_subdirectory(dyld)
add_subdirectory(motool)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libm/Source")
include_directories("${DARLING_TOP_DIRECTORY}/platform-include")
include_directories("${DARLING_TOP_DIRECTORY}/platform-include/pthread")
include_directories("${DARLING_TOP_DIRECTORY}/kernel-include")
include_directories("${DARLING_TOP_DIRECTORY}/compiler-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/duct/include")

add_subdirectory(libc)
add_subdirectory(libm)
add_subdirectory(libinfo)
add_subdirectory(libmalloc)
add_subdirectory(libunwind)
add_subdirectory(libsystem)
add_subdirectory(libutil)
add_subdirectory(libnotify)
add_subdirectory(launchd/liblaunch)
add_subdirectory(keymgr)
add_subdirectory(libiconv)
add_subdirectory(duct/src)
add_subdirectory(libresolv)
add_subdirectory(libstdcxx)
add_subdirectory(external/libobjc2)
add_subdirectory(external/libkqueue)
add_subdirectory(external/libdispatch)
add_subdirectory(external/zlib)
add_subdirectory(external/bzip2)

######################
# libc++ & libc++abi #
######################
set(CMAKE_CXX_FLAGS_SAVED ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc -nostdinc++")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/include/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libcxxabi/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include)
set(LIBCXXABI_LIBCXX_PATH "{CMAKE_CURRENT_SOURCE_DIR}/external/libcxx")
set(LIBCXXABI_LIBCXX_INCLUDES "-I${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx/include")

add_subdirectory(external/libcxxabi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED} -std=c++11 -msse -msse2 -msse3 -nostdinc -nostdinc++ -include ${CMAKE_CURRENT_SOURCE_DIR}/libc/include/xlocale.h")
set(LIBCXX_HAS_STDCXX11_FLAG TRUE)
add_definitions(-D_USE_EXTENDED_LOCALES_)
add_subdirectory(external/libcxx)

#######################
# Linux kernel module #
#######################

add_custom_target(lkm
	COMMAND gmake
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lkm")

