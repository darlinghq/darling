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

# The framework exists only for 64-bit processes on macOS, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

add_framework(RecapPerformanceTesting
	IOSSUPPORT_ALIAS
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/RecapPerformanceTesting.m

	DEPENDENCIES
		system
		objc
		Foundation
		CoreGraphics
)
