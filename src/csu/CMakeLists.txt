project(csu)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")

#include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include/FreeBSD)

set(csu_SRCS
	crt1.10.6.S
)

add_darling_static_library(csu SOURCES ${csu_SRCS})
make_fat(csu)

