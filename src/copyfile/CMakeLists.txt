project(copyfile)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fblocks")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/libinfo/membership.subproj/)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libdispatch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

# Hide warnings
add_definitions(
	-Wno-parentheses
	-Wno-int-conversion
)

set(copyfile_sources
	copyfile.c
	# xattr_flags.c # Doesn't build yet (XPC)
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libcopyfile.dylib")
add_darling_library(system_copyfile SHARED ${copyfile_sources})
set_target_properties(system_copyfile PROPERTIES OUTPUT_NAME "copyfile")
target_link_libraries(system_copyfile PRIVATE system_kernel system_c system_blocks system_info system_dyld system_malloc)

install(TARGETS system_copyfile DESTINATION libexec/darling/usr/lib/system)
