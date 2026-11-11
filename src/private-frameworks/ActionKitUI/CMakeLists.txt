project(ActionKitUI)

remove_sdk_framework(ActionKitUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ActionKitUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ActionKitUI"
    PRIVATE
)

add_framework(ActionKitUI
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ActionKitUI.c

    DEPENDENCIES
        system
)
