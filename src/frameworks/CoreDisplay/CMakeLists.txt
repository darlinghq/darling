project(CoreDisplay)

remove_sdk_framework(CoreDisplay)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreDisplay
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/CoreDisplay"
)

add_framework(CoreDisplay
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/CoreDisplay.c

	DEPENDENCIES
		system
		CoreFoundation
)
