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


#include <CoreSimulator/CoreSimulator.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


void* const SimDeviceBootKeyBinPref = (void*)0;
void* const SimDeviceBootKeyDeathPort = (void*)0;
void* const SimDeviceBootKeyDeviceType = (void*)0;
void* const SimDeviceBootKeyDisabledJobs = (void*)0;
void* const SimDeviceBootKeyEnvironment = (void*)0;
void* const SimDeviceBootKeyPersist = (void*)0;
void* const SimDeviceBootKeyRuntime = (void*)0;
void* const SimDeviceInstallOptionKeySkipWatchAppInstall = (void*)0;
void* const SimDeviceLaunchApplicationEventTypeKeyBackgroundFetch = (void*)0;
void* const SimDeviceLaunchApplicationKeyArguments = (void*)0;
void* const SimDeviceLaunchApplicationKeyBackgrounded = (void*)0;
void* const SimDeviceLaunchApplicationKeyEnvironment = (void*)0;
void* const SimDeviceLaunchApplicationKeyEventType = (void*)0;
void* const SimDeviceLaunchApplicationKeyStandardErrPath = (void*)0;
void* const SimDeviceLaunchApplicationKeyStandardOutPath = (void*)0;
void* const SimDeviceLaunchApplicationKeyTerminateRunningProcess = (void*)0;
void* const SimDeviceLaunchApplicationKeyWaitForDebugger = (void*)0;
void* const SimDeviceNotificationType_BootStatus = (void*)0;
void* const SimDeviceSetOptionHostPort = (void*)0;
void* const SimDeviceSpawnKeyArguments = (void*)0;
void* const SimDeviceSpawnKeyBinPref = (void*)0;
void* const SimDeviceSpawnKeyEnvironment = (void*)0;
void* const SimDeviceSpawnKeyStandalone = (void*)0;
void* const SimDeviceSpawnKeyStderr = (void*)0;
void* const SimDeviceSpawnKeyStdin = (void*)0;
void* const SimDeviceSpawnKeyStdout = (void*)0;
void* const SimDeviceSpawnKeyWaitForDebugger = (void*)0;
void* const SimNSPasteboardItemResolvedTypes = (void*)0;
void* const kSimDeviceNotification_NewBootStatus = (void*)0;
void* const kSimDeviceNotification_PreviousBootStatus = (void*)0;
void* const kSimDevicePropertyKeyDeviceType = (void*)0;
void* const kSimDevicePropertyKeyIsDeleted = (void*)0;
void* const kSimDevicePropertyKeyIsEphemeral = (void*)0;
void* const kSimDevicePropertyKeyLastBootedAt = (void*)0;
void* const kSimDevicePropertyKeyName = (void*)0;
void* const kSimDevicePropertyKeyRuntime = (void*)0;
void* const kSimDevicePropertyKeyRuntimePolicy = (void*)0;
void* const kSimDevicePropertyKeyRuntimeSpecifier = (void*)0;
void* const kSimDevicePropertyKeyState = (void*)0;
void* const kSimDevicePropertyKeyUDID = (void*)0;
void* const kSimDeviceRuntimePolicyKeyLatest = (void*)0;
void* const kSimDeviceRuntimePolicyKeyNewest = (void*)0;
void* const kSimDeviceRuntimePolicyKeyPinnedToBuild = (void*)0;
void* const kSimDeviceRuntimePolicyKeyPinnedToPath = (void*)0;
void* const kSimDeviceRuntimePolicyKeySystem = (void*)0;
void* const kSimPlatformIdentifierPhone = (void*)0;
void* const kSimPlatformIdentifierTV = (void*)0;
void* const kSimPlatformIdentifierWatch = (void*)0;
void* const kSimPlatformShortNamePhone = (void*)0;
void* const kSimPlatformShortNameTV = (void*)0;
void* const kSimPlatformShortNameWatch = (void*)0;
void* const kUnmountApprovalTimeout = (void*)0;
void* const kUnmountVeryLongTimeout = (void*)0;

void *SimDiskImageStateDisplayString(void) {
    if (verbose) puts("STUB: SimDiskImageStateDisplayString called");
    return NULL;
}

void *SimDiskImageStateIsTerminal(void) {
    if (verbose) puts("STUB: SimDiskImageStateIsTerminal called");
    return NULL;
}

void *SimLogFence(void) {
    if (verbose) puts("STUB: SimLogFence called");
    return NULL;
}

void *SimLogSetHandler(void) {
    if (verbose) puts("STUB: SimLogSetHandler called");
    return NULL;
}

void *_DADiskGetVolumePath(void) {
    if (verbose) puts("STUB: _DADiskGetVolumePath called");
    return NULL;
}

void *_GetDiskDescription(void) {
    if (verbose) puts("STUB: _GetDiskDescription called");
    return NULL;
}

void *_ShouldIgnoreDisk(void) {
    if (verbose) puts("STUB: _ShouldIgnoreDisk called");
    return NULL;
}

void *_SimLog(void) {
    if (verbose) puts("STUB: _SimLog called");
    return NULL;
}

void *_SimLogASL(void) {
    if (verbose) puts("STUB: _SimLogASL called");
    return NULL;
}

void *_SimLogCSS(void) {
    if (verbose) puts("STUB: _SimLogCSS called");
    return NULL;
}

void *_SimLogSTDOUT(void) {
    if (verbose) puts("STUB: _SimLogSTDOUT called");
    return NULL;
}

void *pb_promise_support_mig_read_item_promised_data(void) {
    if (verbose) puts("STUB: pb_promise_support_mig_read_item_promised_data called");
    return NULL;
}

