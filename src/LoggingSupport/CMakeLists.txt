project(LoggingSupport)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "518.70.1")

file(GLOB OS_C src/*.c)
file(GLOB OS_OBJC src/*.m)

add_framework(LoggingSupport
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${OS_C}
		${OS_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
