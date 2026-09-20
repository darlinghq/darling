project(LocalAuthenticationRecoveryUI)

remove_sdk_framework(LocalAuthenticationRecoveryUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "2005.160.7")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(LocalAuthenticationRecoveryUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/LocalAuthenticationRecoveryUI"
    PRIVATE
)

add_framework(LocalAuthenticationRecoveryUI
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/LocalAuthenticationRecoveryUI.c

    DEPENDENCIES
        system
)
