set(FRAMEWORK_VERSION "A")

generate_sdk_framework(OnBoardingKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OnBoardingKit"
    PRIVATE
)

add_framework(OnBoardingKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/OnBoardingKit.m

    DEPENDENCIES
        system
        objc
        Cocoa
)
