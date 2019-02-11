project(ImageIO)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ImageIO
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/ImageIO.m

    DEPENDENCIES
        system
        objc
        Foundation
	CoreFoundation
)
