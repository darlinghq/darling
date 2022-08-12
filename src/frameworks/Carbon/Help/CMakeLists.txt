project(Help)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "71.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(Help
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/Help"
)

add_framework(Help
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/Help.c

	DEPENDENCIES
		system
)
