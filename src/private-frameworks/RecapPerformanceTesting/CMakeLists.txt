project(RecapPerformanceTesting)

remove_sdk_framework(RecapPerformanceTesting
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(RecapPerformanceTesting
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/RecapPerformanceTesting"
	PRIVATE
)

add_framework(RecapPerformanceTesting
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/RecapPerformanceTesting.m

	DEPENDENCIES
		system
		objc
		Foundation
)
