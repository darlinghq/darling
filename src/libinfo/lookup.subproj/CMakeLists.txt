project(libinfo-lookup)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

set(info-lookup_sources
	cache_module.c
	ds_module.c
	file_module.c
	getnameinfo_link.c
	ils.c
	kvbuf.c
	libinfo.c
	mdns_module.c
	search_module.c
	si_data.c
	si_getaddrinfo.c
	si_module.c
	thread_data.c
)

add_library(info-lookup OBJECT ${info-lookup_sources})
