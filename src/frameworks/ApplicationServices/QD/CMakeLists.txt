project(QD)

remove_sdk_framework(QD)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(QD
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/QD"
)

add_framework(QD
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}
    PARENT ApplicationServices

    SOURCES
    	src/QD.c

    DEPENDENCIES
	CoreFoundation
        system
)
