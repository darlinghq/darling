project(NavigationServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "226.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(NavigationServices
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/NavigationServices"
)

add_framework(NavigationServices
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/Navigation.m

	DEPENDENCIES
		system
		CoreFoundation
		CoreServices
		Foundation
		objc
		HIToolbox
)
