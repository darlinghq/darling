project(CoreHaptics)

remove_sdk_framework(CoreHaptics)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreHaptics
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreHaptics"
)

add_framework(CoreHaptics
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CHHapticEvent.m
        src/CHHapticEventParameter.m
        src/CHHapticDynamicParameter.m
        src/CHHapticPattern.m
        src/CoreHaptics.m

    DEPENDENCIES
        system
        objc
        Foundation
)
