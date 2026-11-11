project(AggregateDictionary)

remove_sdk_framework(AggregateDictionary
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AggregateDictionary
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AggregateDictionary"
    PRIVATE
)

add_framework(AggregateDictionary
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AggregateDictionary.c

    DEPENDENCIES
        system
)
