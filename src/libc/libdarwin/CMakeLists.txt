project(libc-libdarwin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

add_compile_definitions(
	_LIBC_NO_FEATURE_VERIFICATION=1
	DARWIN_BUILDING_LIBSYSTEM_DARWIN=1
	OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/h")

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_darwin.dylib")
add_circular(system_darwin FAT
	SOURCES
		bsd.c
		ctl.c
		dirstat_collection.c
		dirstat.c
		err.c
		exception.c
		init.c
		mach.c
		stdio.c
		stdlib.c
		string.c
		variant.c
	SIBLINGS
		macho
		system_dyld
		compiler_rt
		system_dyld
		system_kernel
		system_m
		system_malloc
		platform
		system_pthread
		system_c
		libdispatch_shared
		xpc
		system_trace

		# not listed in the Xcode files, but apparently required for `mach.c` (for `bootstrap_strerror`)
		launch
)

install(TARGETS system_darwin DESTINATION libexec/darling/usr/lib/system)
