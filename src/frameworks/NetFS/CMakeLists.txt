project(NetFS)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(NetFS
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		src/NetFS.m

	DEPENDENCIES
		system
		objc
		Foundation
)
