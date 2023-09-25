/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _CORESIMULATOR_H_
#define _CORESIMULATOR_H_

#import <Foundation/Foundation.h>

#import <CoreSimulator/NSArray+SimPasteboardItem.h>
#import <CoreSimulator/NSObjectROCKDeserializable.h>
#import <CoreSimulator/NSObjectROCKSerializable.h>
#import <CoreSimulator/NSPasteboardItem+SimPasteboardItem.h>
#import <CoreSimulator/NSPasteboardItemDataProvider.h>
#import <CoreSimulator/NSString+SimPasteboardItem.h>
#import <CoreSimulator/OS_xpc_object.h>
#import <CoreSimulator/ROCKImpersonatable.h>
#import <CoreSimulator/ROCKImpersonateable.h>
#import <CoreSimulator/ROCKMemoizable.h>
#import <CoreSimulator/SimAudioDescriptorState.h>
#import <CoreSimulator/SimAudioHostRouteDescriptorState.h>
#import <CoreSimulator/SimDevice.h>
#import <CoreSimulator/SimDeviceBootInfo.h>
#import <CoreSimulator/SimDeviceDataMigrationPluginResult.h>
#import <CoreSimulator/SimDeviceIO.h>
#import <CoreSimulator/SimDeviceIOClient.h>
#import <CoreSimulator/SimDeviceIOClientProtocol.h>
#import <CoreSimulator/SimDeviceIODeviceTypeInterface.h>
#import <CoreSimulator/SimDeviceIOInterface.h>
#import <CoreSimulator/SimDeviceIOLoadedBundle.h>
#import <CoreSimulator/SimDeviceIOLogger.h>
#import <CoreSimulator/SimDeviceIOMachServiceProvider.h>
#import <CoreSimulator/SimDeviceIOPortDescriptorState.h>
#import <CoreSimulator/SimDeviceIOProtocol.h>
#import <CoreSimulator/SimDeviceIORuntimeInterface.h>
#import <CoreSimulator/SimDeviceIOServer.h>
#import <CoreSimulator/SimDeviceIOServerLogger.h>
#import <CoreSimulator/SimDeviceNotificationManager.h>
#import <CoreSimulator/SimDeviceNotifier.h>
#import <CoreSimulator/SimDevicePair.h>
#import <CoreSimulator/SimDevicePasteboard.h>
#import <CoreSimulator/SimDevicePasteboardConnection.h>
#import <CoreSimulator/SimDevicePasteboardItem.h>
#import <CoreSimulator/SimDeviceSet.h>
#import <CoreSimulator/SimDeviceType.h>
#import <CoreSimulator/SimDiskImage.h>
#import <CoreSimulator/SimDiskImageIdentifier.h>
#import <CoreSimulator/SimDiskImageManager.h>
#import <CoreSimulator/SimDiskImageRuntimeInfo.h>
#import <CoreSimulator/SimDisplayDefaultDescriptorState.h>
#import <CoreSimulator/SimDisplayDescriptorState.h>
#import <CoreSimulator/SimEnvironmentProvider.h>
#import <CoreSimulator/SimHostResourceChecker.h>
#import <CoreSimulator/SimIdentityClient.h>
#import <CoreSimulator/SimIdentityDevice.h>
#import <CoreSimulator/SimIdentityManager.h>
#import <CoreSimulator/SimIdentitySystem.h>
#import <CoreSimulator/SimKnownSDKEntry.h>
#import <CoreSimulator/SimLaunchHostClient.h>
#import <CoreSimulator/SimLaunchHostConnection.h>
#import <CoreSimulator/SimLegacyHIDDefaultDescriptorState.h>
#import <CoreSimulator/SimLegacyHIDDescriptorState.h>
#import <CoreSimulator/SimLibLaunchInstance.h>
#import <CoreSimulator/SimLocalThrowable.h>
#import <CoreSimulator/SimMachPortServer.h>
#import <CoreSimulator/SimNSPasteboard.h>
#import <CoreSimulator/SimNSPasteboardItem.h>
#import <CoreSimulator/SimNearbyDevice.h>
#import <CoreSimulator/SimNearbyDeviceManager.h>
#import <CoreSimulator/SimNearbyPeer.h>
#import <CoreSimulator/SimNotificationManager.h>
#import <CoreSimulator/SimNotificationManagerToken.h>
#import <CoreSimulator/SimPasteboard.h>
#import <CoreSimulator/SimPasteboardItem.h>
#import <CoreSimulator/SimPasteboardItemData.h>
#import <CoreSimulator/SimPasteboardItemDataProvider.h>
#import <CoreSimulator/SimPasteboardItemWrappedData.h>
#import <CoreSimulator/SimPasteboardPortMap.h>
#import <CoreSimulator/SimPasteboardSyncPool.h>
#import <CoreSimulator/SimPasteboardSyncPoolObject.h>
#import <CoreSimulator/SimPasteboardSyncPoolProtocol.h>
#import <CoreSimulator/SimPendingOperation.h>
#import <CoreSimulator/SimPlatformInfo.h>
#import <CoreSimulator/SimProfileBundle.h>
#import <CoreSimulator/SimProfileModTime.h>
#import <CoreSimulator/SimProfilesPathMonitor.h>
#import <CoreSimulator/SimRuntime.h>
#import <CoreSimulator/SimRuntimeMapData.h>
#import <CoreSimulator/SimRuntimePairingRequirements.h>
#import <CoreSimulator/SimSDKInfo.h>
#import <CoreSimulator/SimSelectedRuntimePolicyResult.h>
#import <CoreSimulator/SimServiceContext.h>
#import <CoreSimulator/SimVolumeManager.h>
#import <CoreSimulator/SimVolumeManagerNotification.h>
#import <CoreSimulator/_SimDeallocSentinel.h>


void *SimDiskImageStateDisplayString(void);
void *SimDiskImageStateIsTerminal(void);
void *SimLogFence(void);
void *SimLogSetHandler(void);
void *_DADiskGetVolumePath(void);
void *_GetDiskDescription(void);
void *_ShouldIgnoreDisk(void);
void *_SimLog(void);
void *_SimLogASL(void);
void *_SimLogCSS(void);
void *_SimLogSTDOUT(void);
void *pb_promise_support_mig_read_item_promised_data(void);


extern void* const SimDeviceBootKeyBinPref;
extern void* const SimDeviceBootKeyDeathPort;
extern void* const SimDeviceBootKeyDeviceType;
extern void* const SimDeviceBootKeyDisabledJobs;
extern void* const SimDeviceBootKeyEnvironment;
extern void* const SimDeviceBootKeyPersist;
extern void* const SimDeviceBootKeyRuntime;
extern void* const SimDeviceInstallOptionKeySkipWatchAppInstall;
extern void* const SimDeviceLaunchApplicationEventTypeKeyBackgroundFetch;
extern void* const SimDeviceLaunchApplicationKeyArguments;
extern void* const SimDeviceLaunchApplicationKeyBackgrounded;
extern void* const SimDeviceLaunchApplicationKeyEnvironment;
extern void* const SimDeviceLaunchApplicationKeyEventType;
extern void* const SimDeviceLaunchApplicationKeyStandardErrPath;
extern void* const SimDeviceLaunchApplicationKeyStandardOutPath;
extern void* const SimDeviceLaunchApplicationKeyTerminateRunningProcess;
extern void* const SimDeviceLaunchApplicationKeyWaitForDebugger;
extern void* const SimDeviceNotificationType_BootStatus;
extern void* const SimDeviceSetOptionHostPort;
extern void* const SimDeviceSpawnKeyArguments;
extern void* const SimDeviceSpawnKeyBinPref;
extern void* const SimDeviceSpawnKeyEnvironment;
extern void* const SimDeviceSpawnKeyStandalone;
extern void* const SimDeviceSpawnKeyStderr;
extern void* const SimDeviceSpawnKeyStdin;
extern void* const SimDeviceSpawnKeyStdout;
extern void* const SimDeviceSpawnKeyWaitForDebugger;
extern void* const SimNSPasteboardItemResolvedTypes;
extern void* const kSimDeviceNotification_NewBootStatus;
extern void* const kSimDeviceNotification_PreviousBootStatus;
extern void* const kSimDevicePropertyKeyDeviceType;
extern void* const kSimDevicePropertyKeyIsDeleted;
extern void* const kSimDevicePropertyKeyIsEphemeral;
extern void* const kSimDevicePropertyKeyLastBootedAt;
extern void* const kSimDevicePropertyKeyName;
extern void* const kSimDevicePropertyKeyRuntime;
extern void* const kSimDevicePropertyKeyRuntimePolicy;
extern void* const kSimDevicePropertyKeyRuntimeSpecifier;
extern void* const kSimDevicePropertyKeyState;
extern void* const kSimDevicePropertyKeyUDID;
extern void* const kSimDeviceRuntimePolicyKeyLatest;
extern void* const kSimDeviceRuntimePolicyKeyNewest;
extern void* const kSimDeviceRuntimePolicyKeyPinnedToBuild;
extern void* const kSimDeviceRuntimePolicyKeyPinnedToPath;
extern void* const kSimDeviceRuntimePolicyKeySystem;
extern void* const kSimPlatformIdentifierPhone;
extern void* const kSimPlatformIdentifierTV;
extern void* const kSimPlatformIdentifierWatch;
extern void* const kSimPlatformShortNamePhone;
extern void* const kSimPlatformShortNameTV;
extern void* const kSimPlatformShortNameWatch;
extern void* const kUnmountApprovalTimeout;
extern void* const kUnmountVeryLongTimeout;

#endif

