project(UIKit)

remove_sdk_framework(UIKit)
generate_sdk_framework(UIKit VERSION A HEADER "include/UIKit")

add_framework(UIKit
    IOSSUPPORT
    CURRENT_VERSION
    FAT
    VERSION A
    SOURCES src/UICollectionViewCompositionalLayout.m
    DEPENDENCIES Foundation objc system
)
target_include_directories(UIKit PRIVATE include)
target_compile_options(UIKit PRIVATE -fobjc-arc)
