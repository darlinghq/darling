project(DiscRecordingUI)

remove_sdk_framework(DiscRecordingUI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DiscRecordingUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DiscRecordingUI"
)

add_framework(DiscRecordingUI
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DiscRecordingUI.m
        src/DRDeviceChooser.m
        src/DRDeviceChooserCell.m
        src/DRBurnProgressPanel.m
        src/DREraseProgressPanel.m
        src/DRSetupPanel.m
        src/DRSetupPanelDeviceFilter.m
        src/DRSetupPanelPrivate.m
        src/DRBurnSetupPanel.m
        src/DRBurnSetupPanelPrivate.m
        src/DREraseSetupPanel.m
        src/DREraseSetupPanelPrivate.m
        src/DRMoveHelper.m
        src/DRUIKitPanelController.m
        src/DRUIPrivateFrameworkClassForFindingBundle.m
        src/DRUIPrivateFrameworkInitializationClass.m
        src/DRLazyIconRefImageRep.m
        src/DRBurnSessionSetupPanelGlueObject.m
        src/DRBurnSessionProgressPanelGlueObject.m
        src/DREraseSessionSetupPanelGlueObject.m
        src/DREraseSessionProgressPanelGlueObject.m
        src/DRMediaEraseController.m
        src/DRMediaEraseControllerInternal.m
        src/DRDeviceObjectController.m
        src/DRDeviceArrayController.m
        src/DROpticalDiscEraseView.m

    DEPENDENCIES
        system
        objc
        Foundation
)
