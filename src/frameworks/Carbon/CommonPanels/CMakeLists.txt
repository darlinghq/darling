project(CommonPanels)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "101.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(CommonPanels
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/CommonPanels"
)

add_framework(CommonPanels
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/CommonPanels.m

	DEPENDENCIES
		system
		objc
		Foundation
)
