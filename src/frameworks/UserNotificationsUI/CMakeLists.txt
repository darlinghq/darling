project(UserNotificationsUI)

remove_sdk_framework(UserNotificationsUI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(UserNotificationsUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/UserNotificationsUI"
)

add_framework(UserNotificationsUI
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/UserNotificationsUI.c

    DEPENDENCIES
        system
)
