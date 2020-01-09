project(DiskManagement)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB DM_C src/*.c)
file(GLOB DM_OBJC src/*.m)

add_framework(DiskManagement
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${DM_C}
		${DM_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
