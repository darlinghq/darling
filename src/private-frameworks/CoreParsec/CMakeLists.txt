project(CoreParsec)

remove_sdk_framework(CoreParsec
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreParsec
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreParsec"
    PRIVATE
)

add_framework(CoreParsec
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreParsec.c

    DEPENDENCIES
        system
)
