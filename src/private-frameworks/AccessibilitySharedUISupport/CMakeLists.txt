project(AccessibilitySharedUISupport)

remove_sdk_framework(AccessibilitySharedUISupport
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AccessibilitySharedUISupport
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AccessibilitySharedUISupport"
    PRIVATE
)

add_framework(AccessibilitySharedUISupport
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AccessibilitySharedUISupport.c

    DEPENDENCIES
        system
)
