project(CoreSymbolication)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "62046.0.0")

file(GLOB CS_C src/*.c)

add_framework(CoreSymbolication
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${CS_C}

	DEPENDENCIES
		system
)
