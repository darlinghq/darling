project(SpotlightServerKit)

remove_sdk_framework(SpotlightServerKit
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1191.2.6")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpotlightServerKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpotlightServerKit"
	PRIVATE
)

add_framework(SpotlightServerKit
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpotlightServerKit.m
        src/MDSNotificationTokenRecord.m
        src/MDSObject.m
        src/MDSMachPortNotificationRecord.m
        src/MDSMachPortObject.m
        src/MDSMachSubsystem.m
        src/MDSTwoFaceMachPortObject.m
        src/MDSInternalToken.m
        src/MDSToken.m
        src/MDSObjectToken.m
        src/MDSNotificationCenter.m
        src/MDSReadCopyUpdate.m

    DEPENDENCIES
        system
        objc
        Foundation
)
