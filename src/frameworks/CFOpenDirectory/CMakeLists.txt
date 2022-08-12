project(CFOpenDirectory)

remove_sdk_framework(CFOpenDirectory)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CFOpenDirectory
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CFOpenDirectory"
)

add_framework(CFOpenDirectory
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/CFOpenDirectory.c
		src/constants.c

	DEPENDENCIES
		system
		CoreFoundation
)
