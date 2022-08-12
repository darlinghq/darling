project(OpenScripting)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(OpenScripting
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/OpenScripting"
)

add_framework(OpenScripting
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/OpenScripting.m

	DEPENDENCIES
		system
		objc
		Foundation
)
