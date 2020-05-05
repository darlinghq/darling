project(ImageIO)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ImageIO
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
	src/CGImageDestination.m
	src/CGImageProperties.m
	src/CGImageSource.m

    DEPENDENCIES
        system
        objc
        Foundation
	CoreFoundation
	Onyx2D # Should be independent?
)
