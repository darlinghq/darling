project(Ink)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "227.0.0")

add_framework(Ink
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/Ink.c

	DEPENDENCIES
		system
)
