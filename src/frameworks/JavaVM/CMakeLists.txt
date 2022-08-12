project(JavaVM)

remove_sdk_framework(JavaVM)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(JavaVM
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/JavaVM"
)

add_framework(JavaVM
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/JavaVM.m
        src/NSJavaVirtualMachine.m
        src/NSJavaConfiguration.m

    DEPENDENCIES
        system
        objc
        Foundation
)
