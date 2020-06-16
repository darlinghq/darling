project(darling-libsystem_malloc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/malloc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/private)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/resolver)

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
	src/magazine_malloc.c
	src/magazine_small.c
	src/magazine_tiny.c
	src/malloc.c
	src/nano_malloc.c
	src/purgeable_malloc.c
	src/magazine_medium.c
	src/magazine_rack.c
	src/malloc_common.c
	src/malloc_printf.c
	src/msl_lite_support.c
	src/nano_malloc_common.c
	src/nanov2_malloc.c
	src/vm.c
)

set_source_files_properties(src/nanov2_malloc.c PROPERTIES
	COMPILE_FLAGS " -DOS_VARIANT_NOTRESOLVED=1 -DOS_VARIANT_RESOLVED=1"
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
