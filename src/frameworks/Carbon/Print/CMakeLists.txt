project(Print)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "271.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(Print
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/Print"
)

add_framework(Print
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/Print.c

	DEPENDENCIES
		system
)
