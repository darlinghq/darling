project(ClassroomKit)

remove_sdk_framework(ClassroomKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ClassroomKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ClassroomKit"
    PRIVATE
)

add_framework(ClassroomKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ClassroomKit.c

    DEPENDENCIES
        system
)
