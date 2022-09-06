project(Ink)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "227.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(Ink
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/Ink"
)

add_framework(Ink
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/Ink.c

	DEPENDENCIES
		system
)
