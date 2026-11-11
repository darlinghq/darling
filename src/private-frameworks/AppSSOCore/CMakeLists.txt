project(AppSSOCore)

remove_sdk_framework(AppSSOCore
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "483.160.10")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppSSOCore
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppSSOCore"
    PRIVATE
)

add_framework(AppSSOCore
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AppSSOCore.c

    DEPENDENCIES
        system
)
