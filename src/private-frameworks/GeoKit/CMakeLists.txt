project(GeoKit)

remove_sdk_framework(GeoKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(GeoKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/GeoKit"
    PRIVATE
)

add_framework(GeoKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/GeoKit.m

	DEPENDENCIES
		system
		objc
		CoreFoundation
		CoreGraphics
)
