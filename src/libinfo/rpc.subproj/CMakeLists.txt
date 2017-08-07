project(libinfo-rpc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")

set(info-rpc_sources
	bindresvport.c
	xdr.c
)

add_library(info-rpc OBJECT ${info-rpc_sources})
