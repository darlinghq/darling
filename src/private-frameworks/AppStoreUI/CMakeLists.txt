project(AppStoreUI)

remove_sdk_framework(AppStoreUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppStoreUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppStoreUI"
    PRIVATE
)

add_framework(AppStoreUI
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AppStoreUI.c

    DEPENDENCIES
        system
)
