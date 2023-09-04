project(ShazamKitUI)

remove_sdk_framework(ShazamKitUI
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ShazamKitUI
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/ShazamKitUI"
)


add_framework(ShazamKitUI
	PRIVATE
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SHPaletteLayer.m
		src/SHPalette.m
		src/SHMaths.m
		src/SHCustomBackgroundCompatiblePalette.m
		src/SHListeningLayer.m
		src/SHListeningViewHelperMethods.m
		src/SHListeningMediaTimingFunctionProvider.m
		src/SHListeningOuterCirclesLayer.m
		src/SHInnerCircleMaskLayer.m
		src/_TtCV11ShazamKitUI29ListeningLayerPlatformWrapper15ListeningNSView.m
		src/SHShazamKitUIServiceConnection.m
		src/SHShazamButtonLayer.m
		src/SHListeningAutoShadowLayer.m
		src/SHListeningInnerCircleLayer.m
		src/SHShazamKitUIServiceConnectionProvider.m
		src/SHListeningOuterCircleLayer.m
		src/SHListeningInnerCirclesLayer.m
		src/NSBundle+ResourcesBundle.m
		src/SHMediaItem+Presentation.m
		src/ShazamKitUI.m

	DEPENDENCIES
		system
		objc
		Foundation
		ShazamKit
)