project(darling-libsystem_malloc)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -fblocks -ggdb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -include ${CMAKE_CURRENT_SOURCE_DIR}/../duct/include/commpage.h -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-flat_namespace -Wl,-undefined,suppress")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__ -DPRIVATE)
add_definitions("'-DMAGMALLOC_MALLOCERRORBREAK()='")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/malloc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/private)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/gen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel/libsyscall/wrappers/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel/libsyscall)

set(libmalloc_sources
	src/bitarray.c
	src/frozen_malloc.c
	src/legacy_malloc.c
	src/magazine_large.c
	src/magazine_lite.c
	src/magazine_malloc.c
	src/magazine_small.c
	src/magazine_tiny.c
	src/malloc.c
	src/nano_malloc.c
	src/purgeable_malloc.c
	src/stack_logging_disk.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_malloc.dylib")
add_darling_library(system_malloc SHARED ${libmalloc_sources})
target_link_libraries(system_malloc PRIVATE system_kernel)
make_fat(system_malloc)

install(TARGETS system_malloc DESTINATION libexec/darling/usr/lib/system)

