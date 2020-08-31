project(cache)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libcache.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "83.0.0")

include_directories(include)

add_circular(libcache FAT
	SOURCES
		src/cache.c
	SIBLINGS
		system_c
		system_dyld
		system_malloc
		platform
		system_kernel
)
set_target_properties(libcache PROPERTIES OUTPUT_NAME "cache")
install(TARGETS libcache DESTINATION libexec/darling/usr/lib/system)
