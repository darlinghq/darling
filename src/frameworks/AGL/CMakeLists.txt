project(AGL)

remove_sdk_framework(AGL)

set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "3.3.2")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AGL
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AGL"
)

add_framework(AGL
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AGL.c

    DEPENDENCIES
    	CoreFoundation
    	Carbon
        system
)
