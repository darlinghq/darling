project(DeviceLink)

remove_sdk_framework(DeviceLink
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "100.0.0")
set(DYLIB_CURRENT_VERSION "264.0.0")

include_directories(include)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DeviceLink
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DeviceLink"
	PRIVATE
)

add_framework(DeviceLink
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/DeviceLink.c

	DEPENDENCIES
		system
)
