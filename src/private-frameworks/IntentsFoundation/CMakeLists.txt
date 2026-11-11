project(IntentsFoundation)

remove_sdk_framework(IntentsFoundation
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IntentsFoundation
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IntentsFoundation"
    PRIVATE
)

add_framework(IntentsFoundation
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/IntentsFoundation.c

    DEPENDENCIES
        system
)
