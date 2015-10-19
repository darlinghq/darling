project(mach_server_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -nostdinc")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../platform-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../emulation/linux/ext")

set(mach_server_client_sources
	mach_traps.c
	mach_time.c
)

add_library(mach_server_client OBJECT ${mach_server_client_sources})
