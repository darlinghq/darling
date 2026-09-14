project(OSD)

remove_sdk_framework(OSD PRIVATE)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(OSD
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/OSD"
	PRIVATE
)

add_framework(OSD
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/OSD.c

	DEPENDENCIES
		system
		CoreFoundation
)
