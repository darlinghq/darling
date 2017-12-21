project(StreamingZip)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB SZ_C src/*.c)
file(GLOB SZ_OBJC src/*.m)

add_framework(StreamingZip
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${SZ_C}
		${SZ_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
