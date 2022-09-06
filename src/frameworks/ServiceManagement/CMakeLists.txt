project(ServiceManagement)

remove_sdk_framework(ServiceManagement)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "972.70.1")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ServiceManagement
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ServiceManagement"
)

add_framework(ServiceManagement
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
	  ServiceManagement.c

	DEPENDENCIES
		system
		CoreFoundation
		Security
		CoreServices
)
