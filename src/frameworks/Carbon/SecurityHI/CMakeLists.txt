project(SecurityHI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "55008.0.0")

add_framework(SecurityHI
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/SecurityHI.c

	DEPENDENCIES
		system
)
