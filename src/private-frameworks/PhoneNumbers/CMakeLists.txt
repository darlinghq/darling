project(PhoneNumbers)

remove_sdk_framework(PhoneNumbers
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PhoneNumbers
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PhoneNumbers"
    PRIVATE
)

add_framework(PhoneNumbers
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/PhoneNumbers.c

    DEPENDENCIES
        system
)
