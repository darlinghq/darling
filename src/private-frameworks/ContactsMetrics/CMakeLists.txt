project(ContactsMetrics)

remove_sdk_framework(ContactsMetrics
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ContactsMetrics
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ContactsMetrics"
    PRIVATE
)

add_framework(ContactsMetrics
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ContactsMetrics.c

    DEPENDENCIES
        system
)
