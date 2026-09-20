project(KerberosHelper)

remove_sdk_framework(KerberosHelper
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(KerberosHelper
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/KerberosHelper"
    PRIVATE
)

add_framework(KerberosHelper
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/KerberosHelper.c

    DEPENDENCIES
        system
)
