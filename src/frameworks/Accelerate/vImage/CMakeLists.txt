project(vImage)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "545.0.0")

add_framework(vImage
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT "Accelerate"

    SOURCES
        src/vImageConvertCore.c
        src/vImageConverter.c
        src/vImageUtilities.c
        src/vImageLegacy.c

    DEPENDENCIES
        system
        CoreFoundation
        CoreGraphics
)

target_include_directories(vImage PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

InstallSymlink(Accelerate.framework/Versions/A/Frameworks/vImage.framework ${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/vImage.framework)
