project(AssertionServices)

remove_sdk_framework(AssertionServices
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "223.140.2")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AssertionServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AssertionServices"
    PRIVATE
)

add_framework(AssertionServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/AssertionServices.m
        src/BKSWorkspace.m
        src/BKSAssertion.m
        src/BKSProcessExitContext.m
        src/BKSProcess.m
        src/BKSTerminationContext.m
        src/BKSProcessAssertion.m
        src/BKSTerminationAssertionObserverManager.m
        src/BKSTerminationAssertion.m
        src/BKSApplicationStateMonitor.m
        src/BKSLaunchdJobSpecification.m

    DEPENDENCIES
        system
        objc
        Foundation
)
