project(DiskManagement)

remove_sdk_framework(DiskManagement
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB DM_C src/*.c)
file(GLOB DM_OBJC src/*.m)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DiskManagement
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DiskManagement"
	PRIVATE
)

add_framework(DiskManagement
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${DM_C}
		${DM_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
