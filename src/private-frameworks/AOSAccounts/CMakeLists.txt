project(AOSAccounts)

remove_sdk_framework(AOSAccounts
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.9.95")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AOSAccounts
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AOSAccounts"
    PRIVATE
)

add_framework(AOSAccounts
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AOSAccounts.c

    DEPENDENCIES
        system
)
