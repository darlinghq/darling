/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel
    
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

#ifndef _LOGIN_LOGIN_H_
#define _LOGIN_LOGIN_H_

#include <login/SessionAgentCom.h>
#include <login/SessionAgentStatusCom.h>

void CFXPCRelease(void);
void CreateArgumentsForFunction(void);
void GetSessionAgentClient(void);
void GetSessionManagementClient(void);
void JoinAuditSessionIfNeeded(void);
void LFAppleIDBreadcrumbCheckin(void);
void LFAppleIDBreadcrumbCopyAppleIDForUser(void);
void LFAppleIDBreadcrumbCopyOldKeychainPassword(void);
void LFAppleIDBreadcrumbUpdate(void);
void LFArgumentsAddAuditSessionID(void);
void LFArgumentsAddBoolean(void);
void LFArgumentsAddCFObject(void);
void LFArgumentsAddCString(void);
void LFArgumentsAddConnection(void);
void LFArgumentsAddConnectionEndpoint(void);
void LFArgumentsAddInt64(void);
void LFArgumentsAddLFClient(void);
void LFArgumentsAddLFServer(void);
void LFArgumentsAddMachPort(void);
void LFArgumentsAddNull(void);
void LFArgumentsAddUInt64(void);
void LFArgumentsCallFunction(void);
void LFArgumentsCopyFunctionIdentifier(void);
void LFArgumentsCreate(void);
void LFArgumentsCreateWithArguments(void);
void LFArgumentsCreateWithXPCDictionary(void);
void LFArgumentsGetDictionary(void);
void LFArgumentsGetFunction(void);
void LFArgumentsIsValid(void);
void LFArgumentsShouldAddArgumentForFlag(void);
void LFClientAddErrorHandler(void);
void LFClientCreateWithConnection(void);
void LFClientCreateWithConnectionAndServer(void);
void LFClientCreateWithServiceName(void);
void LFClientGetConnection(void);
void LFClientGetEndpoint(void);
void LFClientGetPID(void);
void LFClientGetServer(void);
void LFClientGetTypeID(void);
void LFClientRemoveErrorHandler(void);
void LFClientSendArgumentsAndGetCFReply(void);
void LFClientSendArgumentsAndGetReply(void);
void LFClientSetEndpoint(void);
void LFClientSetEventHandler(void);
void LFClientSetInstance(void);
void LFClientSetIsMachService(void);
void LFClientSetPrivileged(void);
void LFClientSetUserID(void);
void LFClientStart(void);
void LFClientStop(void);
void LFDBLoggingControllerStart(void);
void LFDBLoggingControllerStop(void);
void LFDictionaryCreateMutableWithObjectForKey(void);
void LFDictionaryGetBooleanForKey(void);
void LFDictionaryGetCFIndexForKey(void);
void LFDictionaryGetDictionaryForKey(void);
void LFDictionaryGetDoubleForKey(void);
void LFDictionarySetBoolean(void);
void LFDictionarySetCFIndex(void);
void LFDictionarySetDictionary(void);
void LFFunctionCacheAddFunction(void);
void LFFunctionCacheCreateServerFunction(void);
void LFFunctionCacheGetFunction(void);
void LFFunctionCreateClientFunction(void);
void LFFunctionCreateServerFunction(void);
void LFFunctionGetIdentifier(void);
void LFLogUnhandledEventType(void);
void LFSMCloseSession(void);
void LFSMCopyAllSessions(void);
void LFSMCopyCurrentSession(void);
void LFSMCopySessionProperties(void);
void LFSMCopySessionWithCGSessionID(void);
void LFSMCreateDefaultSessionOptions(void);
void LFSMCreateSessionWithOptions(void);
void LFSMCreateSessionWithOptionsByStartingServer(void);
void LFSMSessionGetTypeID(void);
void LFSMSwitchToSessionWithOptions(void);
void LFServerCreate(void);
void LFServerGetClientForConnection(void);
void LFServerGetConnection(void);
void LFServerGetEventHandler(void);
void LFServerGetServiceName(void);
void LFServerGetTypeID(void);
void LFServerSetIsMachService(void);
void LFServerStart(void);
void LFServerStop(void);
void LFStringCreateFromString(void);
void LogindServerCreate(void);
void LogindServerRelease(void);
void SACBeginLoginTransition(void);
void SACCopyOnceMiniBuddyItem(void);
void SACDeleteMiniBuddyItem(void);
void SACDisableFileVaultForCurrentUserWithPassword(void);
void SACEnableFileVaultForCurrentUserWithPassword(void);
void SACFaceTimeCallRingStart(void);
void SACFaceTimeCallRingStop(void);
void SACLOFinishDelayedLogout(void);
void SACLORegisterLogoutStatusCallbacks(void);
void SACLOStartLogout(void);
void SACLOStartLogoutWithOptions(void);
void SACLegacyFileVaultCloseProgressWindow(void);
void SACLegacyFileVaultSetBarPercentage(void);
void SACLegacyFileVaultSetPhase(void);
void SACLegacyFileVaultSetTask(void);
void SACLegacyFileVaultShowErrorMessage(void);
void SACLegacyFileVaultShowProgressWindow(void);
void SACLegacyFileVaultUpdateTimeRemaining(void);
void SACLegacyFileVaultUpdateUI(void);
void SACLockScreenImmediate(void);
void SACLogoutComplete(void);
void SACMessageFaceTimeCancelCall(void);
void SACMessageFaceTimeMuteCall(void);
void SACMessageLogoutStatusClientLogoutFailed(void);
void SACMessageLogoutStatusClientLogoutSuccess(void);
void SACMessageMiniBuddyUIShowing(void);
void SACMiniBuddyCopyUpgradeDictionary(void);
void SACMiniBuddySignalCompleted(void);
void SACMiniBuddySignalFinishedStage1WithOptions(void);
void SACProximityPreferencesChanged(void);
void SACRestartForUser(void);
void SACSaveSetupUserScreenShots(void);
void SACScreenLockEnabled(void);
void SACScreenLockPreferencesChanged(void);
void SACScreenSaverCanRun(void);
void SACScreenSaverDidFadeInBackground(void);
void SACScreenSaverIsRunning(void);
void SACScreenSaverIsRunningInBackground(void);
void SACScreenSaverStartNow(void);
void SACScreenSaverStopNow(void);
void SACScreenSaverStopNowWithOptions(void);
void SACScreenSaverTimeRemaining(void);
void SACSetAppleIDVerified(void);
void SACSetAutoLoginPassword(void);
void SACSetFileVaultPassword(void);
void SACSetFinalSnapshot(void);
void SACSetMiniBuddyItem(void);
void SACSetPhoneProximity(void);
void SACSetScreenSaverCanRun(void);
void SACShieldWindowShowing(void);
void SACStartProgressIndicator(void);
void SACStartSessionForLoginWindow(void);
void SACStartSessionForUser(void);
void SACStopProgressIndicator(void);
void SACStopSessionForLoginWindow(void);
void SACSwitchToLoginWindow(void);
void SACSwitchToUser(void);
void SACUpdateAppleIDUserLogin(void);
void SACWriteKeyboardType(void);
void SAClearLWScreenShots(void);
void SAClearLaunchSoftwareUpdateTrigger(void);
void SAClearSoftwareUpdateOptions(void);
void SAPrepareForSetupUserScreenShots(void);
void SAResetLoginKeychainForAutoLoginPassword(void);
void SASetAppleIDVerified(void);
void SASetAutoLoginUserScreenLocked(void);
void SASetAutologinPW(void);
void SASetLaunchSoftwareUpdateTrigger(void);
void SASetPreviousStartupWasPanic(void);
void SASetSCDynamicStoreConsoleInformation(void);
void SASetSessionAuthenticatedFlag(void);
void SASetSessionHasConsoleAccessFlag(void);
void SASetSessionState(void);
void SASetSoftwareUpdateOption(void);
void SASetSwapCompaction(void);
void SASetTextEncodingBias(void);
void SASync(void);
void SASystemNotifyPost(void);
void SAWriteKeyboardType(void);
void SLOAddErrorHandler(void);
void SLOLegacyFileVaultSetState(void);
void SLOStartLogoutOfType(void);
void SMCopySessionUserInfo(void);
void SMGetSessionAgentConnection(void);
void SMRegisterSessionAgent(void);
void SMSetSessionUserInfo(void);
void SOStartSessionWithOptions(void);
void SessionAgentServerCreate(void);
void SessionAgentServerGetServer(void);
void SessionAgentServerRelease(void);
void SessionLogoutServerCreate(void);
void SessionLogoutServerRelease(void);
void SessionOwnerServerCreate(void);
void SessionOwnerServerRelease(void);
void _LFArgumentsCopyDebugDesc(void);
void _LFArgumentsCopyFormattingDesc(void);
void _LFArgumentsFinalize(void);
void _LFArgumentsInit(void);
void _LFFunctionCopyDebugDesc(void);
void _LFFunctionCopyFormattingDesc(void);
void _LFFunctionCreateServerFunction(void);
void _LFFunctionEqual(void);
void _LFFunctionFinalize(void);
void _LFFunctionHash(void);
void _LFFunctionInit(void);
void _LFGetBreadcrumbEncryptedKeyWithBlock(void);
void _LFServerCopyDebugDesc(void);
void _LFServerCopyFormattingDesc(void);
void _LFServerEqual(void);
void _LFServerFinalize(void);
void _LFServerHash(void);
void _LFSetBreadcrumbEncryptedKeyWithBlock(void);
void logindServerGetServer(void);

#endif /* _LOGIN_LOGIN_H_ */
