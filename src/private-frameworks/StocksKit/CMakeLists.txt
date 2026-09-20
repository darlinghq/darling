project(StocksKit)

remove_sdk_framework(StocksKit
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(StocksKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/StocksKit"
    PRIVATE
)

add_framework(StocksKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/StocksKit.c

    DEPENDENCIES
        system
)
