project(QuickLookUI)

remove_sdk_framework(QuickLookUI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(QuickLookUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/QuickLookUI"
)

add_framework(QuickLookUI
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/QLPreviewPanel.m
        src/QLPreviewView.m

    DEPENDENCIES
        system
        objc
        Foundation
        AppKit
        QuickLook
)
