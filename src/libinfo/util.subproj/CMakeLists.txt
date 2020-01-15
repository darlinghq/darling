project(libinfo-util)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/stdtime/FreeBSD)

set(info-util_sources
	hton.c
	rcmd.c
)

add_darling_object_library(info-util ${info-util_sources})
