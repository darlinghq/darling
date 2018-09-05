project(JavaVM)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(JavaVM
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/JavaVM.m
        src/NSJavaVirtualMachine.m
        src/NSJavaConfiguration.m

    DEPENDENCIES
        system
        objc
        Foundation
)
