project(CFOpenDirectory)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(CFOpenDirectory
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		src/CFOpenDirectory.c
		src/constants.c

	DEPENDENCIES
		system
		CoreFoundation
)
