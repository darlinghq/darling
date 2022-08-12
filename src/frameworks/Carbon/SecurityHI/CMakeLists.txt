project(SecurityHI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "55008.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(SecurityHI
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/SecurityHI"
)

add_framework(SecurityHI
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/SecurityHI.c

	DEPENDENCIES
		system
)
