project(AuthKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB AK_C src/*.c)
file(GLOB AK_OBJC src/*.m)

add_framework(AuthKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${AK_C}
		${AK_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
