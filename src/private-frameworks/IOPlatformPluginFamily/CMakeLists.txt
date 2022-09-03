project(IOPlatformPluginFamily)

remove_sdk_framework(IOPlatformPluginFamily
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

file(GLOB PPF_C src/*.c)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IOPlatformPluginFamily
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IOPlatformPluginFamily"
	PRIVATE
)

add_framework(IOPlatformPluginFamily
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${PPF_C}

	DEPENDENCIES
		system
)
