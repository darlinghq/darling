set(FRAMEWORK_VERSION "A")
generate_sdk_framework(CPAnalytics
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CPAnalytics"
    PRIVATE
)
add_framework(CPAnalytics
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}
    SOURCES
        src/CPAnalytics.m
    DEPENDENCIES
        system
        objc
        Foundation
)