set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Calculate
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Calculate"
    PRIVATE
)

add_framework(Calculate
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Calculate.c

    DEPENDENCIES
        system
)
