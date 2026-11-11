project(SystemUIPlugin)

remove_sdk_framework(SystemUIPlugin
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "44.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SystemUIPlugin
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SystemUIPlugin"
    PRIVATE
)

add_framework(SystemUIPlugin
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SystemUIPlugin.c

    DEPENDENCIES
        system
)
