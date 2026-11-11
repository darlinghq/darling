project(Marco)

remove_sdk_framework(Marco
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "800.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Marco
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Marco"
    PRIVATE
)

add_framework(Marco
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Marco.c

    DEPENDENCIES
        system
)
