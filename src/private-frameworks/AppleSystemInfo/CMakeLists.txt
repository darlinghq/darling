project(AppleSystemInfo)

remove_sdk_framework(AppleSystemInfo
    PRIVATE
)

add_compile_options(
	-nostdinc
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppleSystemInfo
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppleSystemInfo"
    PRIVATE
)

add_framework(AppleSystemInfo
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/AppleSystemInfo.c
		src/functions.c
	DEPENDENCIES
		system
		CoreFoundation
)
