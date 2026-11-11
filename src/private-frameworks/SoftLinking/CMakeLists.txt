set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SoftLinking
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SoftLinking"
    PRIVATE
)

add_framework(SoftLinking
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SoftLinking.m

    DEPENDENCIES
        system
        objc
        Foundation
)
