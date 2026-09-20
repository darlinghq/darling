project(MediaExperience)

remove_sdk_framework(MediaExperience
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MediaExperience
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MediaExperience"
    PRIVATE
)

add_framework(MediaExperience
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MediaExperience.c

    DEPENDENCIES
        system
)
