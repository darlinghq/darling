project(CoreSimulator)

remove_sdk_framework(CoreSimulator
	PRIVATE
	PARENT_DIR "/Library/Developer/PrivateFrameworks"
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "885.2.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreSimulator
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/CoreSimulator"
	PRIVATE
	PARENT_DIR "/Library/Developer/PrivateFrameworks"
)

add_framework(CoreSimulator
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}
	PRIVATE
	PARENT_DIR "/Library/Developer/PrivateFrameworks"

	SOURCES
		src/SimNearbyPeer.m
		src/SimIdentityManager.m
		src/SimRuntimeMapData.m
		src/SimDeviceIOLoadedBundle.m
		src/SimIdentityDevice.m
		src/SimDiskImageIdentifier.m
		src/SimMachPortServer.m
		src/SimDeviceIOServer.m
		src/SimDevice.m
		src/SimDeviceType.m
		src/SimDisplayDefaultDescriptorState.m
		src/_SimDeallocSentinel.m
		src/SimDeviceIOServerLogger.m
		src/SimDeviceIOClient.m
		src/SimNotificationManager.m
		src/SimRuntimePairingRequirements.m
		src/SimDevicePair.m
		src/SimProfileBundle.m
		src/SimProfileModTime.m
		src/SimNearbyDeviceManager.m
		src/SimPasteboardItem.m
		src/SimSelectedRuntimePolicyResult.m
		src/SimLocalThrowable.m
		src/SimProfilesPathMonitor.m
		src/SimDiskImage.m
		src/SimNSPasteboardItem.m
		src/SimNotificationManagerToken.m
		src/SimDeviceNotificationManager.m
		src/SimLegacyHIDDefaultDescriptorState.m
		src/SimDiskImageManager.m
		src/SimAudioHostRouteDescriptorState.m
		src/SimLibLaunchInstance.m
		src/SimIdentityClient.m
		src/SimKnownSDKEntry.m
		src/SimDeviceBootInfo.m
		src/SimPasteboardSyncPool.m
		src/SimLaunchHostConnection.m
		src/SimNearbyDevice.m
		src/SimDeviceSet.m
		src/SimPendingOperation.m
		src/SimDeviceDataMigrationPluginResult.m
		src/SimVolumeManagerNotification.m
		src/SimPasteboardSyncPoolObject.m
		src/SimSDKInfo.m
		src/SimDeviceIO.m
		src/SimPasteboardItemData.m
		src/SimPasteboardPortMap.m
		src/SimLaunchHostClient.m
		src/SimRuntime.m
		src/SimDevicePasteboard.m
		src/SimPlatformInfo.m
		src/SimDevicePasteboardConnection.m
		src/SimDevicePasteboardItem.m
		src/SimDiskImageRuntimeInfo.m
		src/SimVolumeManager.m
		src/SimNSPasteboard.m
		src/SimPasteboardItemWrappedData.m
		src/SimServiceContext.m
		src/SimHostResourceChecker.m
		src/NSArray+SimPasteboardItem.m
		src/NSString+SimPasteboardItem.m
		src/NSPasteboardItem+SimPasteboardItem.m
		src/CoreSimulator.m

	DEPENDENCIES
		system
		objc
		Foundation
		AppKit
)
