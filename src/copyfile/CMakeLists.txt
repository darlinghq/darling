project(copyfile)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_definitions(-DHAVE_STDINT_H=1 -DPRIVATE=1)

# Hide warnings
add_definitions(
	-w
	-nostdinc
)

set(copyfile_sources
	copyfile.c
	xattr_flags.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libcopyfile.dylib")
add_darling_library(system_copyfile SHARED ${copyfile_sources})
set_target_properties(system_copyfile PROPERTIES OUTPUT_NAME "copyfile")
target_link_libraries(system_copyfile PRIVATE system_kernel system_c system_blocks system_info system_dyld system_malloc xpc system_quarantine libdispatch_shared)
make_fat(system_copyfile)

install(TARGETS system_copyfile DESTINATION libexec/darling/usr/lib/system)
