project(mach_server_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -nostdinc")
include_directories("${CMAKE_SOURCE_DIR}/platform-include")
include_directories("${CMAKE_SOURCE_DIR}/src/libc/include")
include_directories("${CMAKE_SOURCE_DIR}/src/kernel/emulation/linux/ext")
include_directories("${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers")

set(mach_server_client_sources
	mach_traps.c
	mach_time.c
	lkm.c
)

add_library(mach_server_client OBJECT ${mach_server_client_sources})
add_library(mach_server_client_dyld OBJECT ${mach_server_client_sources})
set_target_properties(mach_server_client_dyld PROPERTIES COMPILE_FLAGS "-DVARIANT_DYLD")
make_fat(mach_server_client)
make_fat(mach_server_client_dyld)
