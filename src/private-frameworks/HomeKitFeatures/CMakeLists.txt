project(HomeKitFeatures)

remove_sdk_framework(HomeKitFeatures
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(HomeKitFeatures
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/HomeKitFeatures"
    PRIVATE
)

add_framework(HomeKitFeatures
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/HomeKitFeatures.c

    DEPENDENCIES
        system
)
