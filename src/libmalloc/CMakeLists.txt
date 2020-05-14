project(darling-libsystem_malloc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/malloc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/private)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_compile_options(
	-nostdinc
)

add_definitions(
	-DPRIVATE
	-DOS_UNFAIR_LOCK_INLINE=1
	-D__DARWIN_UNIX03
)

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
add_circular(system_malloc FAT
	SOURCES
		${libmalloc_sources}
	SIBLINGS
		system_kernel
		system_c
		platform
		system_dyld
		compiler_rt
)

install(TARGETS system_malloc DESTINATION libexec/darling/usr/lib/system)
