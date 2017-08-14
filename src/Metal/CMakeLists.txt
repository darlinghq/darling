project(Metal)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB MTL_C src/*.c)
file(GLOB MTL_OBJC src/*.m)

add_framework(Metal
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		${MTL_C}
		${MTL_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
