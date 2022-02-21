project(AssertionServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "223.140.2")

add_framework(AssertionServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

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
