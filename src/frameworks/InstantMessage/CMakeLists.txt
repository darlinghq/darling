project(InstantMessage)

remove_sdk_framework(InstantMessage)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "800.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(InstantMessage
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/InstantMessage"
)

add_framework(InstantMessage
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/InstantMessage.m
        src/IMAuxVideoProvider.m
        src/IMSharedPixelBufferPool.m
        src/_IMVideoDataSource.m
        src/IMAVManagerImpl.m
        src/IMAVManager.m
        src/IMAVControlBarImpl.m
        src/IMAVControlBar.m
        src/_IMAVControlAuxiliary.m
        src/_IMAVButtonAuxiliary.m
        src/_IMAVSliderAuxiliary.m
        src/IMAVControl.m
        src/IMAVButton.m
        src/IMAVSlider.m
        src/IMService.m
        src/iChatTheaterXPC.m

    DEPENDENCIES
        system
        objc
        Foundation
)
