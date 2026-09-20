project(QueryParser)

remove_sdk_framework(QueryParser
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "3525.3.5")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(QueryParser
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/QueryParser"
    PRIVATE
)

add_framework(QueryParser
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/QueryParser.c

    DEPENDENCIES
        system
)
