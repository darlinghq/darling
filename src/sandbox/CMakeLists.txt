project(libsandbox)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_sandbox.dylib")
add_circular(system_sandbox FAT
	SOURCES
	sandbox.c

	SIBLINGS
		system_dyld
	DEPENDENCIES
		system_c
		system_kernel
		system_malloc
)

install(TARGETS system_sandbox DESTINATION libexec/darling/usr/lib/system)
