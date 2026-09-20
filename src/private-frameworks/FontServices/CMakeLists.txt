project(FontServices)

remove_sdk_framework(FontServices
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(FontServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/FontServices"
    PRIVATE
)

add_framework(FontServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/FontServices.c

    DEPENDENCIES
        system
)
