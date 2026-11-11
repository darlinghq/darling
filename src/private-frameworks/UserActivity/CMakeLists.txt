project(UserActivity)

remove_sdk_framework(UserActivity
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "551.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(UserActivity
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/UserActivity"
    PRIVATE
)

add_framework(UserActivity
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/UserActivity.c

    DEPENDENCIES
        system
)
