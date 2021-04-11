project(libinfo-membership)

cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lookup.subproj)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(
	-DDS_AVAILABLE
)

set(info-membership_sources
	membership.c
)

add_darling_object_library(info-membership ${info-membership_sources})
