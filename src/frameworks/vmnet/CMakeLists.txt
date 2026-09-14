project(vmnet)

remove_sdk_framework(vmnet)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(vmnet
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/vmnet"
)

add_framework(vmnet
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/vmnet.c

	DEPENDENCIES
		system
		xpc
		libdispatch_shared
)
