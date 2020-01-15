project(libinfo-gen)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lookup.subproj)

set(info-gen_sources
	configuration_profile.c
	ether_addr.c
	getifaddrs.c
	getifmaddrs.c
	if_indextoname.c
	if_nameindex.c
	if_nametoindex.c
	ip6opt.c
	map_v4v6.c
	rthdr.c
	vars.c
)

add_darling_object_library(info-gen ${info-gen_sources})
