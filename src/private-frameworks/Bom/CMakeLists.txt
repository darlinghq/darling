project(Bom)

set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "195.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(Bom
    PRIVATE
)

generate_sdk_framework(Bom
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Bom"
    PRIVATE
)

add_framework(Bom
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Bom.c

    DEPENDENCIES
        system
)
