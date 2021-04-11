project(libquarantine)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

set(DYLIB_INSTALL_NAME "/usr/lib/system/libquarantine.dylib")
add_darling_library(system_quarantine SHARED
	quarantine.c
)
set_target_properties(system_quarantine PROPERTIES OUTPUT_NAME "quarantine")
target_link_libraries(system_quarantine PRIVATE system_c system_dyld system_malloc)
make_fat(system_quarantine)

install(TARGETS system_quarantine DESTINATION libexec/darling/usr/lib/system)

