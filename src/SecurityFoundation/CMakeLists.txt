project(SecurityFoundation)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "55132.50.7")

file(GLOB SF_OBJC src/*.m)

add_framework(SecurityFoundation
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		${SF_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
