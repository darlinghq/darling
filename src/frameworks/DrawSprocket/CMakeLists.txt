project(DrawSprocket)

remove_sdk_framework(DrawSprocket)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DrawSprocket
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DrawSprocket"
)

add_framework(DrawSprocket
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DrawSprocket.c

    DEPENDENCIES
    	CoreFoundation
    	Carbon
        system
)
