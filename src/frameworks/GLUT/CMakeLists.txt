project(GLUT)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(GLUT
)

generate_sdk_framework(GLUT
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/GLUT"
)

add_framework(GLUT
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/GLUT.m
        src/GLUTWindow.m
        src/GLUTView.m
        src/GLUTPreferencesController.m
        src/GLUTMenuItem.m
        src/GLUTMenu.m
        src/GLUTUnknownView.m
        src/GLUTClipboardController.m
        src/GLUTApplication.m

    DEPENDENCIES
        system
        objc
        Foundation
)
