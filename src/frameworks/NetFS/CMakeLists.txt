project(NetFS)

remove_sdk_framework(NetFS)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NetFS
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NetFS"
)

add_framework(NetFS
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/NetFS.m

	DEPENDENCIES
		system
		objc
		Foundation
)
