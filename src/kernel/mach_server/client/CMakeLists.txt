project(mach_server_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -nostdinc")
include_directories("${DARLING_TOP_DIRECTORY}/platform-include")
include_directories("${DARLING_TOP_DIRECTORY}/src/libc/include")
include_directories("${DARLING_TOP_DIRECTORY}/src/kernel/emulation/linux/ext")

set(mach_server_client_sources
	mach_traps.c
	mach_time.c
	lkm.c
)

add_library(mach_server_client OBJECT ${mach_server_client_sources})
