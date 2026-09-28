set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppSupport
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppSupport"
    PRIVATE
)

add_framework(AppSupport
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AppSupport.m

    DEPENDENCIES
        system
        objc
        Foundation
)
