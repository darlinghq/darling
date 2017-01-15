project(libremovefile)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/gen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

set(removefile_sources
	removefile.c
	removefile_random.c
	removefile_rename_unlink.c
	removefile_sunlink.c
	removefile_tree_walker.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libremovefile.dylib")
add_darling_library(removefile SHARED ${removefile_sources})
target_link_libraries(removefile system_c system_kernel system_dyld)

install(TARGETS removefile DESTINATION libexec/darling/usr/lib/system)

