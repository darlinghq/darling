project(libinfo-nis)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/private)
include_directories(${DARLING_TOP_DIRECTORY}/src/libresolv)
include_directories(${DARLING_TOP_DIRECTORY}/src/libresolv/dnsinfo)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

set(info-nis_sources
	getdomainname.c
	setdomainname.c
)

add_darling_object_library(info-nis ${info-nis_sources})
