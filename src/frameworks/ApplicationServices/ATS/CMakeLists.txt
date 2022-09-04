project(ATS)

remove_sdk_framework(ATS)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "236.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ATS
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ATS"
)

add_framework(ATS
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
    PARENT ApplicationServices

    SOURCES
    	src/ATS.c

    DEPENDENCIES
	CoreFoundation
        system
)
