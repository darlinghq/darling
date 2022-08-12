project(ImageIO)

remove_sdk_framework(ImageIO)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ImageIO
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ImageIO"
)

add_framework(ImageIO
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

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
