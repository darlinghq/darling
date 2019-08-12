project(libinfo-rpc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

set(info-rpc_sources
	bindresvport.c
	xdr.c
)

add_darling_object_library(info-rpc ${info-rpc_sources})
