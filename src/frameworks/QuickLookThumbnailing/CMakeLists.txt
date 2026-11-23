project(QuickLookThumbnailing)

remove_sdk_framework(QuickLookThumbnailing)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(QuickLookThumbnailing
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/QuickLookThumbnailing"
)

add_framework(QuickLookThumbnailing
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/QLThumbnailGenerator.m
        src/QLThumbnailGenerationRequest.m
        src/QLThumbnailRepresentation.m
        src/QLThumbnailError.m

    DEPENDENCIES
        system
        objc
        Foundation
        CoreFoundation
        UniformTypeIdentifiers
)
