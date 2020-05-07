project(GameController)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(GameController
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/GameController.m
        src/_GCCControllerHIDServiceInfo.m
        src/_GCControllerManager.m
        src/GCControllerElement.m
        src/GCControllerButtonInput.m
        src/_GCControllerButtonInput.m
        src/GCControllerAxisInput.m
        src/_GCControllerAxisInput.m
        src/_GCControllerAxisButtonInput.m
        src/GCControllerDirectionPad.m
        src/_GCControllerDirectionPad.m
        src/GCGamepad.m
        src/GCMicroGamepad.m
        src/_GCMicroGamepad.m
        src/_GCGamepad.m
        src/GCExtendedGamepad.m
        src/_GCExtendedGamepad.m
        src/GCControllerSFDelegate.m
        src/GCControllerMBDelegate.m
        src/GCControllerCBDelegate.m
        src/GCController.m
        src/GCEventViewController.m
        src/_GCController.m
        src/GCGamepadSnapshot.m
        src/_GCGamepadSnapshot.m
        src/GCMicroGamepadSnapshot.m
        src/_GCMicroGamepadSnapshot.m
        src/GCExtendedGamepadSnapshot.m
        src/_GCExtendedGamepadSnapshot.m
        src/GCMotion.m
        src/_GCACHomeButton.m
        src/_GCMFiGamepadControllerProfile.m
        src/_GCMFiExtendedGamepadControllerProfile.m
        src/_GCAppleTVRemoteControllerProfile.m

    DEPENDENCIES
        system
        objc
        Foundation
)
