project(SpotlightIndex)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1191.2.6")

add_framework(SpotlightIndex
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

    SOURCES
        src/SpotlightIndex.m

    DEPENDENCIES
        system
        objc
        Foundation
)
