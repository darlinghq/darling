project(PlugInKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB PK_C src/*.c)
file(GLOB PK_OBJC src/*.m)

add_framework(PlugInKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${PK_C}
		${PK_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
