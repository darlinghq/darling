project(LinkPresentation)

remove_sdk_framework(LinkPresentation)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(LinkPresentation
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/LinkPresentation"
)

# The framework exists only for 64-bit processes on macOS, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

add_framework(LinkPresentation
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/LPLinkMetadata.m
		src/LPMetadataProvider.m

	DEPENDENCIES
		system
		objc
		Foundation
)
