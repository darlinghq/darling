project(ForceFeedback)

remove_sdk_framework(ForceFeedback)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.2")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ForceFeedback
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ForceFeedback"
)

add_framework(ForceFeedback
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ForceFeedback.c

    DEPENDENCIES
        system
)
