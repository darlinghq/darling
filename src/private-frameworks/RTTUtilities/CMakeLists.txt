project(RTTUtilities)

remove_sdk_framework(RTTUtilities
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(RTTUtilities
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/RTTUtilities"
	PRIVATE
)

add_framework(RTTUtilities
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/RTTUtilities.m

	DEPENDENCIES
		system
		objc
		Foundation
)
