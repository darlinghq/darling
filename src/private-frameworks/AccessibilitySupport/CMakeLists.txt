project(AccessibilitySupport)

remove_sdk_framework(AccessibilitySupport
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AccessibilitySupport
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AccessibilitySupport"
    PRIVATE
)

add_framework(AccessibilitySupport
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AccessibilitySupport.c

    DEPENDENCIES
        system
)
