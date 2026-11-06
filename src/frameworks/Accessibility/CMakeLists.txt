project(Accessibility)

remove_sdk_framework(Accessibility)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Accessibility
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Accessibility"
)

add_framework(Accessibility
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AXCustomContent.m
        src/Accessibility.m

    DEPENDENCIES
        system
        objc
        Foundation
)
