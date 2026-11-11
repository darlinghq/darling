project(SoftwareUpdate)

remove_sdk_framework(SoftwareUpdate
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SoftwareUpdate
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SoftwareUpdate"
    PRIVATE
)

add_framework(SoftwareUpdate
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SoftwareUpdate.m

	DEPENDENCIES
		system
		objc
)
