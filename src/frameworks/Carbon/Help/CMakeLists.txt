project(Help)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "71.0.0")

add_framework(Help
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/Help.c

	DEPENDENCIES
		system
)
