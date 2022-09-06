project(DiskArbitration)

remove_sdk_framework(DiskArbitration)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

set(da_SRCS
	DADisk.c
	DADissenter.c
	DASession.c
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DiskArbitration
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DiskArbitration"
)

add_framework(DiskArbitration
	FAT CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${da_SRCS}

	DEPENDENCIES
		system
		CoreFoundation
)
