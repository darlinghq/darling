project(AdPlatforms)

remove_sdk_framework(AdPlatforms
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AdPlatforms
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AdPlatforms"
    PRIVATE
)

add_framework(AdPlatforms
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AdPlatforms.c

    DEPENDENCIES
        system
)
