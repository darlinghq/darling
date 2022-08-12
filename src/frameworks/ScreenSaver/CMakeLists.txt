project(ScreenSaver)

remove_sdk_framework(ScreenSaver)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ScreenSaver
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ScreenSaver"
)

add_framework(ScreenSaver
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/ScreenSaver.m
		src/ScreenSaverView.m
		src/ScreenSaverExtensionModule.m
		src/ScreenSaverModules.m
		src/ScreenSaverDefaults.m
		src/ScreenSaverDefaultsManager.m
		src/ScreenSaverWindow.m
		src/ScreenSaverController.m
		src/ScreenSaverEngine.m
		src/ScreenSaverClockView.m
		src/ScreenSaverClockWindow.m
		src/ScreenSaverViewController.m
		src/LegacyScreenSaverModule.m
		src/ScreenSaverExtensionContext.m
		src/ScreenSaverHostExtensionContext.m
		src/ScreenSaverMessageTracerLogger.m
		src/ScreenSaverExtensionView.m
		src/ScreenSaverExtension.m
		src/ScreenSaverConfigurationViewController.m
		src/ScreenSaverModule.m
		src/ScreenSaverMessage.m
		src/ScreenSaverExtensionManager.m
		src/LegacyScreenSaverExtensionModule.m
		src/ScreenSaverPhotoChooser.m

	DEPENDENCIES
		system
		objc
		Foundation
)
