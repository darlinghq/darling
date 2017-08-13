project(NetworkStatistics)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB NW_C src/*.c)
file(GLOB NW_OBJC src/*.m)

add_framework(NetworkStatistics
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${NW_C}
		${NW_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
