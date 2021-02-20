project(mach_server_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_definitions(-nostdinc)

include_directories(
	${CMAKE_SOURCE_DIR}/src/kernel/emulation/linux/ext
	${CMAKE_SOURCE_DIR}/src/startup
	${CMAKE_BINARY_DIR}/src/startup
)

set(mach_server_client_sources
	mach_traps.c
	mach_time.c
	lkm.c
	darling_mach_syscall.S
	mach_table.c
)

add_darling_object_library(mach_server_client ${mach_server_client_sources})
add_darling_object_library(mach_server_client_dyld ${mach_server_client_sources})
set_target_properties(mach_server_client_dyld PROPERTIES COMPILE_FLAGS "-DVARIANT_DYLD")
make_fat(mach_server_client)
make_fat(mach_server_client_dyld)
