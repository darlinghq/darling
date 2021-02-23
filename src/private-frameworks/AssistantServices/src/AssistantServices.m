/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <AssistantServices/AssistantServices.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AFAccessibilitySupportsVibrationDisablement(void)
{
    if (verbose) puts("STUB: AFAccessibilitySupportsVibrationDisablement called");
    return NULL;
}

void* AFAceErrorCodeFromMRMediaRemoteCommandHandlerStatus(void)
{
    if (verbose) puts("STUB: AFAceErrorCodeFromMRMediaRemoteCommandHandlerStatus called");
    return NULL;
}

void* AFActivationEventGetDescription(void)
{
    if (verbose) puts("STUB: AFActivationEventGetDescription called");
    return NULL;
}

void* AFAddInterruptingAudioSessionInfoToAnalyticsContext(void)
{
    if (verbose) puts("STUB: AFAddInterruptingAudioSessionInfoToAnalyticsContext called");
    return NULL;
}

void* AFAnalyticsContextCreateForCurrentProcess(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateForCurrentProcess called");
    return NULL;
}

void* AFAnalyticsContextCreateWithCommand(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithCommand called");
    return NULL;
}

void* AFAnalyticsContextCreateWithDictationOptions(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithDictationOptions called");
    return NULL;
}

void* AFAnalyticsContextCreateWithError(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithError called");
    return NULL;
}

void* AFAnalyticsContextCreateWithExperiment(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithExperiment called");
    return NULL;
}

void* AFAnalyticsContextCreateWithLanguageDetectorMetrics(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithLanguageDetectorMetrics called");
    return NULL;
}

void* AFAnalyticsContextCreateWithRequestInfo(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithRequestInfo called");
    return NULL;
}

void* AFAnalyticsContextCreateWithSpeechRequestOptions(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextCreateWithSpeechRequestOptions called");
    return NULL;
}

void* AFAnalyticsContextsMerge(void)
{
    if (verbose) puts("STUB: AFAnalyticsContextsMerge called");
    return NULL;
}

void* AFAnalyticsEventCreateCurrent(void)
{
    if (verbose) puts("STUB: AFAnalyticsEventCreateCurrent called");
    return NULL;
}

void* AFAnalyticsEventCreateWithGenericInstrumentation(void)
{
    if (verbose) puts("STUB: AFAnalyticsEventCreateWithGenericInstrumentation called");
    return NULL;
}

void* AFAnalyticsEventTypeGetLegacyName(void)
{
    if (verbose) puts("STUB: AFAnalyticsEventTypeGetLegacyName called");
    return NULL;
}

void* AFAnalyticsEventTypeGetName(void)
{
    if (verbose) puts("STUB: AFAnalyticsEventTypeGetName called");
    return NULL;
}

void* AFAnalyticsEventTypeIsValid(void)
{
    if (verbose) puts("STUB: AFAnalyticsEventTypeIsValid called");
    return NULL;
}

void* AFAnalyticsLogsDirectory(void)
{
    if (verbose) puts("STUB: AFAnalyticsLogsDirectory called");
    return NULL;
}

void* AFAnalyticsObservationServiceGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFAnalyticsObservationServiceGetXPCInterface called");
    return NULL;
}

void* AFAnalyticsObserverGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFAnalyticsObserverGetXPCInterface called");
    return NULL;
}

void* AFAnalyticsSafeContextForJson(void)
{
    if (verbose) puts("STUB: AFAnalyticsSafeContextForJson called");
    return NULL;
}

void* AFAnalyticsServiceDelegateGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFAnalyticsServiceDelegateGetXPCInterface called");
    return NULL;
}

void* AFAnalyticsServiceGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFAnalyticsServiceGetXPCInterface called");
    return NULL;
}

void* AFAppleAudioDeviceConnectedInLast24Hours(void)
{
    if (verbose) puts("STUB: AFAppleAudioDeviceConnectedInLast24Hours called");
    return NULL;
}

void* AFApplicationClientIdentifierForApp(void)
{
    if (verbose) puts("STUB: AFApplicationClientIdentifierForApp called");
    return NULL;
}

void* AFApplicationClientIdentifierForWatchApp(void)
{
    if (verbose) puts("STUB: AFApplicationClientIdentifierForWatchApp called");
    return NULL;
}

void* AFApplicationWorkspaceEnumerateInstalledApplications(void)
{
    if (verbose) puts("STUB: AFApplicationWorkspaceEnumerateInstalledApplications called");
    return NULL;
}

void* AFApplicationWorkspaceEnumerateInstalledUserVisibleApplications(void)
{
    if (verbose) puts("STUB: AFApplicationWorkspaceEnumerateInstalledUserVisibleApplications called");
    return NULL;
}

void* AFAssistantCapable(void)
{
    if (verbose) puts("STUB: AFAssistantCapable called");
    return NULL;
}

void* AFAssistantRestricted(void)
{
    if (verbose) puts("STUB: AFAssistantRestricted called");
    return NULL;
}

void* AFAudioPlaybackOptionsGetNames(void)
{
    if (verbose) puts("STUB: AFAudioPlaybackOptionsGetNames called");
    return NULL;
}

void* AFAudioPlaybackOptionsValidate(void)
{
    if (verbose) puts("STUB: AFAudioPlaybackOptionsValidate called");
    return NULL;
}

void* AFBackedUpPreferencesSynchronize(void)
{
    if (verbose) puts("STUB: AFBackedUpPreferencesSynchronize called");
    return NULL;
}

void* AFBluetoothHeadphoneEarbudSideGetFromName(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneEarbudSideGetFromName called");
    return NULL;
}

void* AFBluetoothHeadphoneEarbudSideGetIsValid(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneEarbudSideGetIsValid called");
    return NULL;
}

void* AFBluetoothHeadphoneEarbudSideGetName(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneEarbudSideGetName called");
    return NULL;
}

void* AFBluetoothHeadphoneInEarStatusGetFromName(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneInEarStatusGetFromName called");
    return NULL;
}

void* AFBluetoothHeadphoneInEarStatusGetIsValid(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneInEarStatusGetIsValid called");
    return NULL;
}

void* AFBluetoothHeadphoneInEarStatusGetName(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneInEarStatusGetName called");
    return NULL;
}

void* AFBluetoothHeadphoneListeningModeGetFromName(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneListeningModeGetFromName called");
    return NULL;
}

void* AFBluetoothHeadphoneListeningModeGetIsValid(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneListeningModeGetIsValid called");
    return NULL;
}

void* AFBluetoothHeadphoneListeningModeGetName(void)
{
    if (verbose) puts("STUB: AFBluetoothHeadphoneListeningModeGetName called");
    return NULL;
}

void* AFBluetoothWirelessSplitterSessionStateGetFromName(void)
{
    if (verbose) puts("STUB: AFBluetoothWirelessSplitterSessionStateGetFromName called");
    return NULL;
}

void* AFBluetoothWirelessSplitterSessionStateGetIsValid(void)
{
    if (verbose) puts("STUB: AFBluetoothWirelessSplitterSessionStateGetIsValid called");
    return NULL;
}

void* AFBluetoothWirelessSplitterSessionStateGetName(void)
{
    if (verbose) puts("STUB: AFBluetoothWirelessSplitterSessionStateGetName called");
    return NULL;
}

void* AFBooleanGetFromName(void)
{
    if (verbose) puts("STUB: AFBooleanGetFromName called");
    return NULL;
}

void* AFBooleanGetIsValid(void)
{
    if (verbose) puts("STUB: AFBooleanGetIsValid called");
    return NULL;
}

void* AFBooleanGetName(void)
{
    if (verbose) puts("STUB: AFBooleanGetName called");
    return NULL;
}

void* AFBuildVersion(void)
{
    if (verbose) puts("STUB: AFBuildVersion called");
    return NULL;
}

void* AFBundleIDFromApplicationClientIdentifier(void)
{
    if (verbose) puts("STUB: AFBundleIDFromApplicationClientIdentifier called");
    return NULL;
}

void* AFBundleServicesCachePath(void)
{
    if (verbose) puts("STUB: AFBundleServicesCachePath called");
    return NULL;
}

void* AFCallSiteInfoGetCurrent(void)
{
    if (verbose) puts("STUB: AFCallSiteInfoGetCurrent called");
    return NULL;
}

void* AFCarouselBundleIdentifier(void)
{
    if (verbose) puts("STUB: AFCarouselBundleIdentifier called");
    return NULL;
}

void* AFClearInternalLogFiles(void)
{
    if (verbose) puts("STUB: AFClearInternalLogFiles called");
    return NULL;
}

void* AFClientLiteClientObjectInterface(void)
{
    if (verbose) puts("STUB: AFClientLiteClientObjectInterface called");
    return NULL;
}

void* AFClientLiteRemoteObjectInterface(void)
{
    if (verbose) puts("STUB: AFClientLiteRemoteObjectInterface called");
    return NULL;
}

void* AFClientServiceDelegateXPCInterface(void)
{
    if (verbose) puts("STUB: AFClientServiceDelegateXPCInterface called");
    return NULL;
}

void* AFClientServiceXPCInterface(void)
{
    if (verbose) puts("STUB: AFClientServiceXPCInterface called");
    return NULL;
}

void* AFClockAlarmCreateFromMTAlarm(void)
{
    if (verbose) puts("STUB: AFClockAlarmCreateFromMTAlarm called");
    return NULL;
}

void* AFClockAlarmRepeatOptionsGetNames(void)
{
    if (verbose) puts("STUB: AFClockAlarmRepeatOptionsGetNames called");
    return NULL;
}

void* AFClockAlarmRepeatOptionsValidate(void)
{
    if (verbose) puts("STUB: AFClockAlarmRepeatOptionsValidate called");
    return NULL;
}

void* AFClockTimerCreateFromMTTimer(void)
{
    if (verbose) puts("STUB: AFClockTimerCreateFromMTTimer called");
    return NULL;
}

void* AFClockTimerGetTimerStateFromMTTimer(void)
{
    if (verbose) puts("STUB: AFClockTimerGetTimerStateFromMTTimer called");
    return NULL;
}

void* AFClockTimerStateGetFromName(void)
{
    if (verbose) puts("STUB: AFClockTimerStateGetFromName called");
    return NULL;
}

void* AFClockTimerStateGetIsValid(void)
{
    if (verbose) puts("STUB: AFClockTimerStateGetIsValid called");
    return NULL;
}

void* AFClockTimerStateGetName(void)
{
    if (verbose) puts("STUB: AFClockTimerStateGetName called");
    return NULL;
}

void* AFClockTimerTypeGetFromName(void)
{
    if (verbose) puts("STUB: AFClockTimerTypeGetFromName called");
    return NULL;
}

void* AFClockTimerTypeGetIsValid(void)
{
    if (verbose) puts("STUB: AFClockTimerTypeGetIsValid called");
    return NULL;
}

void* AFClockTimerTypeGetName(void)
{
    if (verbose) puts("STUB: AFClockTimerTypeGetName called");
    return NULL;
}

void* AFCommandOutcomeFromMediaRemoteStatuses(void)
{
    if (verbose) puts("STUB: AFCommandOutcomeFromMediaRemoteStatuses called");
    return NULL;
}

void* AFConnectionHasEntitlement(void)
{
    if (verbose) puts("STUB: AFConnectionHasEntitlement called");
    return NULL;
}

void* AFConnectionHasEntitlementCached(void)
{
    if (verbose) puts("STUB: AFConnectionHasEntitlementCached called");
    return NULL;
}

void* AFContextProviderNameForBundleId(void)
{
    if (verbose) puts("STUB: AFContextProviderNameForBundleId called");
    return NULL;
}

void* AFCopyKeychainBaseQueryDictionary(void)
{
    if (verbose) puts("STUB: AFCopyKeychainBaseQueryDictionary called");
    return NULL;
}

void* AFCurrentUserInterfaceIdiom(void)
{
    if (verbose) puts("STUB: AFCurrentUserInterfaceIdiom called");
    return NULL;
}

void* AFDeleteCoreDuetEventsInStream(void)
{
    if (verbose) puts("STUB: AFDeleteCoreDuetEventsInStream called");
    return NULL;
}

void* AFDescriptionOfAFQuickStopActions(void)
{
    if (verbose) puts("STUB: AFDescriptionOfAFQuickStopActions called");
    return NULL;
}

void* AFDescriptionOfBKSApplicationState(void)
{
    if (verbose) puts("STUB: AFDescriptionOfBKSApplicationState called");
    return NULL;
}

void* AFDeviceColor(void)
{
    if (verbose) puts("STUB: AFDeviceColor called");
    return NULL;
}

void* AFDeviceEnclosureColor(void)
{
    if (verbose) puts("STUB: AFDeviceEnclosureColor called");
    return NULL;
}

void* AFDeviceHasAddressBook(void)
{
    if (verbose) puts("STUB: AFDeviceHasAddressBook called");
    return NULL;
}

void* AFDeviceIsVictory(void)
{
    if (verbose) puts("STUB: AFDeviceIsVictory called");
    return NULL;
}

void* AFDeviceName(void)
{
    if (verbose) puts("STUB: AFDeviceName called");
    return NULL;
}

void* AFDeviceRegionCode(void)
{
    if (verbose) puts("STUB: AFDeviceRegionCode called");
    return NULL;
}

void* AFDeviceRingerSwitchStateGetFromName(void)
{
    if (verbose) puts("STUB: AFDeviceRingerSwitchStateGetFromName called");
    return NULL;
}

void* AFDeviceRingerSwitchStateGetIsValid(void)
{
    if (verbose) puts("STUB: AFDeviceRingerSwitchStateGetIsValid called");
    return NULL;
}

void* AFDeviceRingerSwitchStateGetName(void)
{
    if (verbose) puts("STUB: AFDeviceRingerSwitchStateGetName called");
    return NULL;
}

void* AFDeviceSupportsAOP(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsAOP called");
    return NULL;
}

void* AFDeviceSupportsBargeIn(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsBargeIn called");
    return NULL;
}

void* AFDeviceSupportsEchoCancellation(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsEchoCancellation called");
    return NULL;
}

void* AFDeviceSupportsNavigation(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsNavigation called");
    return NULL;
}

void* AFDeviceSupportsSpokenMessages(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsSpokenMessages called");
    return NULL;
}

void* AFDeviceSupportsTTS(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsTTS called");
    return NULL;
}

void* AFDeviceSupportsWiFiLPASMode(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsWiFiLPASMode called");
    return NULL;
}

void* AFDeviceSupportsZLL(void)
{
    if (verbose) puts("STUB: AFDeviceSupportsZLL called");
    return NULL;
}

void* AFDiagnosticsSubmissionAllowed(void)
{
    if (verbose) puts("STUB: AFDiagnosticsSubmissionAllowed called");
    return NULL;
}

void* AFDictationCapable(void)
{
    if (verbose) puts("STUB: AFDictationCapable called");
    return NULL;
}

void* AFDictationLanguageForKeyboardLanguage(void)
{
    if (verbose) puts("STUB: AFDictationLanguageForKeyboardLanguage called");
    return NULL;
}

void* AFDictationRestricted(void)
{
    if (verbose) puts("STUB: AFDictationRestricted called");
    return NULL;
}

void* AFDictationServiceDelegateXPCInterface(void)
{
    if (verbose) puts("STUB: AFDictationServiceDelegateXPCInterface called");
    return NULL;
}

void* AFDictationTaskHintTypeFromString(void)
{
    if (verbose) puts("STUB: AFDictationTaskHintTypeFromString called");
    return NULL;
}

void* AFDirectActionCreateRequestInfo(void)
{
    if (verbose) puts("STUB: AFDirectActionCreateRequestInfo called");
    return NULL;
}

void* AFDisambiguationHistoryReset(void)
{
    if (verbose) puts("STUB: AFDisambiguationHistoryReset called");
    return NULL;
}

void* AFDispatchAsyncLogged(void)
{
    if (verbose) puts("STUB: AFDispatchAsyncLogged called");
    return NULL;
}

void* AFDispatchSyncLogged(void)
{
    if (verbose) puts("STUB: AFDispatchSyncLogged called");
    return NULL;
}

void* AFEffectiveSiriBundleForLocation(void)
{
    if (verbose) puts("STUB: AFEffectiveSiriBundleForLocation called");
    return NULL;
}

void* AFErrorEnumerate(void)
{
    if (verbose) puts("STUB: AFErrorEnumerate called");
    return NULL;
}

void* AFExperimentConfigurationTypeGetFromName(void)
{
    if (verbose) puts("STUB: AFExperimentConfigurationTypeGetFromName called");
    return NULL;
}

void* AFExperimentConfigurationTypeGetIsValid(void)
{
    if (verbose) puts("STUB: AFExperimentConfigurationTypeGetIsValid called");
    return NULL;
}

void* AFExperimentConfigurationTypeGetName(void)
{
    if (verbose) puts("STUB: AFExperimentConfigurationTypeGetName called");
    return NULL;
}

void* AFExperimentDeploymentReasonGetFromName(void)
{
    if (verbose) puts("STUB: AFExperimentDeploymentReasonGetFromName called");
    return NULL;
}

void* AFExperimentDeploymentReasonGetIsValid(void)
{
    if (verbose) puts("STUB: AFExperimentDeploymentReasonGetIsValid called");
    return NULL;
}

void* AFExperimentDeploymentReasonGetName(void)
{
    if (verbose) puts("STUB: AFExperimentDeploymentReasonGetName called");
    return NULL;
}

void* AFFileURLForConversationWithIdentifier(void)
{
    if (verbose) puts("STUB: AFFileURLForConversationWithIdentifier called");
    return NULL;
}

void* AFGetCurrentItemAdamIDFromNowPlayingInfo(void)
{
    if (verbose) puts("STUB: AFGetCurrentItemAdamIDFromNowPlayingInfo called");
    return NULL;
}

void* AFGetFlashlightLevel(void)
{
    if (verbose) puts("STUB: AFGetFlashlightLevel called");
    return NULL;
}

void* AFGetNowPlayingInfoDictionary(void)
{
    if (verbose) puts("STUB: AFGetNowPlayingInfoDictionary called");
    return NULL;
}

void* AFGetNowPlayingQueueState(void)
{
    if (verbose) puts("STUB: AFGetNowPlayingQueueState called");
    return NULL;
}

void* AFHasBaseband(void)
{
    if (verbose) puts("STUB: AFHasBaseband called");
    return NULL;
}

void* AFHasCellularData(void)
{
    if (verbose) puts("STUB: AFHasCellularData called");
    return NULL;
}

void* AFHasCellularTelephony(void)
{
    if (verbose) puts("STUB: AFHasCellularTelephony called");
    return NULL;
}

void* AFHasGPS(void)
{
    if (verbose) puts("STUB: AFHasGPS called");
    return NULL;
}

void* AFHasHapticEngine(void)
{
    if (verbose) puts("STUB: AFHasHapticEngine called");
    return NULL;
}

void* AFHasPairedBTDeviceForSpokenMessages(void)
{
    if (verbose) puts("STUB: AFHasPairedBTDeviceForSpokenMessages called");
    return NULL;
}

void* AFHasProxyCellularTelephony(void)
{
    if (verbose) puts("STUB: AFHasProxyCellularTelephony called");
    return NULL;
}

void* AFHasRingerSwitch(void)
{
    if (verbose) puts("STUB: AFHasRingerSwitch called");
    return NULL;
}

void* AFHasUnlockedSinceBoot(void)
{
    if (verbose) puts("STUB: AFHasUnlockedSinceBoot called");
    return NULL;
}

void* AFInternalConfigValueForKey(void)
{
    if (verbose) puts("STUB: AFInternalConfigValueForKey called");
    return NULL;
}

void* AFInternalPreferencesSynchronize(void)
{
    if (verbose) puts("STUB: AFInternalPreferencesSynchronize called");
    return NULL;
}

void* AFInterstitialCreateCommand(void)
{
    if (verbose) puts("STUB: AFInterstitialCreateCommand called");
    return NULL;
}

void* AFInterstitialGetDialogIdentifierForPhase(void)
{
    if (verbose) puts("STUB: AFInterstitialGetDialogIdentifierForPhase called");
    return NULL;
}

void* AFInterstitialIsCommandInterstitial(void)
{
    if (verbose) puts("STUB: AFInterstitialIsCommandInterstitial called");
    return NULL;
}

void* AFInterstitialIsDialogIdentifierInterstitial(void)
{
    if (verbose) puts("STUB: AFInterstitialIsDialogIdentifierInterstitial called");
    return NULL;
}

void* AFInterstitialPhaseGetFromName(void)
{
    if (verbose) puts("STUB: AFInterstitialPhaseGetFromName called");
    return NULL;
}

void* AFInterstitialPhaseGetIsValid(void)
{
    if (verbose) puts("STUB: AFInterstitialPhaseGetIsValid called");
    return NULL;
}

void* AFInterstitialPhaseGetName(void)
{
    if (verbose) puts("STUB: AFInterstitialPhaseGetName called");
    return NULL;
}

void* AFInterstitialStyleGetFromName(void)
{
    if (verbose) puts("STUB: AFInterstitialStyleGetFromName called");
    return NULL;
}

void* AFInterstitialStyleGetIsValid(void)
{
    if (verbose) puts("STUB: AFInterstitialStyleGetIsValid called");
    return NULL;
}

void* AFInterstitialStyleGetName(void)
{
    if (verbose) puts("STUB: AFInterstitialStyleGetName called");
    return NULL;
}

void* AFInterstitialSupportsRequestWithEvent(void)
{
    if (verbose) puts("STUB: AFInterstitialSupportsRequestWithEvent called");
    return NULL;
}

void* AFInvocationFeedbackExperimentFeatureGroupsGetNames(void)
{
    if (verbose) puts("STUB: AFInvocationFeedbackExperimentFeatureGroupsGetNames called");
    return NULL;
}

void* AFInvocationFeedbackExperimentFeatureGroupsValidate(void)
{
    if (verbose) puts("STUB: AFInvocationFeedbackExperimentFeatureGroupsValidate called");
    return NULL;
}

void* AFIsATV(void)
{
    if (verbose) puts("STUB: AFIsATV called");
    return NULL;
}

void* AFIsCustomerInstall(void)
{
    if (verbose) puts("STUB: AFIsCustomerInstall called");
    return NULL;
}

void* AFIsCustomerInstallOrPPT(void)
{
    if (verbose) puts("STUB: AFIsCustomerInstallOrPPT called");
    return NULL;
}

void* AFIsFileNotFoundError(void)
{
    if (verbose) puts("STUB: AFIsFileNotFoundError called");
    return NULL;
}

void* AFIsHorseman(void)
{
    if (verbose) puts("STUB: AFIsHorseman called");
    return NULL;
}

void* AFIsIPod(void)
{
    if (verbose) puts("STUB: AFIsIPod called");
    return NULL;
}

void* AFIsInternalInstall(void)
{
    if (verbose) puts("STUB: AFIsInternalInstall called");
    return NULL;
}

void* AFIsMac(void)
{
    if (verbose) puts("STUB: AFIsMac called");
    return NULL;
}

void* AFIsMultiUserCompanion(void)
{
    if (verbose) puts("STUB: AFIsMultiUserCompanion called");
    return NULL;
}

void* AFIsNano(void)
{
    if (verbose) puts("STUB: AFIsNano called");
    return NULL;
}

void* AFIsPPT(void)
{
    if (verbose) puts("STUB: AFIsPPT called");
    return NULL;
}

void* AFIsPad(void)
{
    if (verbose) puts("STUB: AFIsPad called");
    return NULL;
}

void* AFIsSyncNeeded(void)
{
    if (verbose) puts("STUB: AFIsSyncNeeded called");
    return NULL;
}

void* AFKeyboardLMLocaleOverride(void)
{
    if (verbose) puts("STUB: AFKeyboardLMLocaleOverride called");
    return NULL;
}

void* AFKeyboardResourcesDirectoryPath(void)
{
    if (verbose) puts("STUB: AFKeyboardResourcesDirectoryPath called");
    return NULL;
}

void* AFKeychainRemoveValueForAccountAndKey(void)
{
    if (verbose) puts("STUB: AFKeychainRemoveValueForAccountAndKey called");
    return NULL;
}

void* AFKeychainSetValueForAccountAndKey(void)
{
    if (verbose) puts("STUB: AFKeychainSetValueForAccountAndKey called");
    return NULL;
}

void* AFKeychainValueForAccountAndKey(void)
{
    if (verbose) puts("STUB: AFKeychainValueForAccountAndKey called");
    return NULL;
}

void* AFLanguageOptionsForNowPlayingInfo(void)
{
    if (verbose) puts("STUB: AFLanguageOptionsForNowPlayingInfo called");
    return NULL;
}

void* AFLanguagePrefixForLanguageCode(void)
{
    if (verbose) puts("STUB: AFLanguagePrefixForLanguageCode called");
    return NULL;
}

void* AFLibraryDirectoryWithSubPath(void)
{
    if (verbose) puts("STUB: AFLibraryDirectoryWithSubPath called");
    return NULL;
}

void* AFLocationStatusIsAllowed(void)
{
    if (verbose) puts("STUB: AFLocationStatusIsAllowed called");
    return NULL;
}

void* AFLocationStatusIsDenied(void)
{
    if (verbose) puts("STUB: AFLocationStatusIsDenied called");
    return NULL;
}

void* AFLogDirectory(void)
{
    if (verbose) puts("STUB: AFLogDirectory called");
    return NULL;
}

void* AFLogFlush(void)
{
    if (verbose) puts("STUB: AFLogFlush called");
    return NULL;
}

void* AFLogInitIfNeeded(void)
{
    if (verbose) puts("STUB: AFLogInitIfNeeded called");
    return NULL;
}

void* AFLogParsedCommandObject(void)
{
    if (verbose) puts("STUB: AFLogParsedCommandObject called");
    return NULL;
}

void* AFLogWantsToLogParsedCommandObjects(void)
{
    if (verbose) puts("STUB: AFLogWantsToLogParsedCommandObjects called");
    return NULL;
}

void* AFMachAbsoluteTimeAddTimeInterval(void)
{
    if (verbose) puts("STUB: AFMachAbsoluteTimeAddTimeInterval called");
    return NULL;
}

void* AFMachAbsoluteTimeGetMicroseconds(void)
{
    if (verbose) puts("STUB: AFMachAbsoluteTimeGetMicroseconds called");
    return NULL;
}

void* AFMachAbsoluteTimeGetMilliseconds(void)
{
    if (verbose) puts("STUB: AFMachAbsoluteTimeGetMilliseconds called");
    return NULL;
}

void* AFMachAbsoluteTimeGetNanoseconds(void)
{
    if (verbose) puts("STUB: AFMachAbsoluteTimeGetNanoseconds called");
    return NULL;
}

void* AFMachAbsoluteTimeGetTimeInterval(void)
{
    if (verbose) puts("STUB: AFMachAbsoluteTimeGetTimeInterval called");
    return NULL;
}

void* AFMachAbsoluteTimeSubtractTimeInterval(void)
{
    if (verbose) puts("STUB: AFMachAbsoluteTimeSubtractTimeInterval called");
    return NULL;
}

void* AFMainScreenScale(void)
{
    if (verbose) puts("STUB: AFMainScreenScale called");
    return NULL;
}

void* AFManagedPreferencesDomainForDomain(void)
{
    if (verbose) puts("STUB: AFManagedPreferencesDomainForDomain called");
    return NULL;
}

void* AFMapsNavigationStatusIsActive(void)
{
    if (verbose) puts("STUB: AFMapsNavigationStatusIsActive called");
    return NULL;
}

void* AFMediaPlaybackStateGetFromName(void)
{
    if (verbose) puts("STUB: AFMediaPlaybackStateGetFromName called");
    return NULL;
}

void* AFMediaPlaybackStateGetIsValid(void)
{
    if (verbose) puts("STUB: AFMediaPlaybackStateGetIsValid called");
    return NULL;
}

void* AFMediaPlaybackStateGetName(void)
{
    if (verbose) puts("STUB: AFMediaPlaybackStateGetName called");
    return NULL;
}

void* AFMediaRemoteSendCommandNoCheck(void)
{
    if (verbose) puts("STUB: AFMediaRemoteSendCommandNoCheck called");
    return NULL;
}

void* AFMediaRemoteSendCommandWithOptions(void)
{
    if (verbose) puts("STUB: AFMediaRemoteSendCommandWithOptions called");
    return NULL;
}

void* AFMediaRemoteSendCommandWithOptionsNoCheck(void)
{
    if (verbose) puts("STUB: AFMediaRemoteSendCommandWithOptionsNoCheck called");
    return NULL;
}

void* AFMemoryInfoGetCurrent(void)
{
    if (verbose) puts("STUB: AFMemoryInfoGetCurrent called");
    return NULL;
}

void* AFMobileUserBundleServicesCachePath(void)
{
    if (verbose) puts("STUB: AFMobileUserBundleServicesCachePath called");
    return NULL;
}

void* AFMultiUserServiceXPCInterface(void)
{
    if (verbose) puts("STUB: AFMultiUserServiceXPCInterface called");
    return NULL;
}

void* AFMyraidCoordinatorAudioHashFilePath(void)
{
    if (verbose) puts("STUB: AFMyraidCoordinatorAudioHashFilePath called");
    return NULL;
}

void* AFMyriadCoordinatorAudioHashFileBaseDirectory(void)
{
    if (verbose) puts("STUB: AFMyriadCoordinatorAudioHashFileBaseDirectory called");
    return NULL;
}

void* AFNavigationStateIsNavigating(void)
{
    if (verbose) puts("STUB: AFNavigationStateIsNavigating called");
    return NULL;
}

void* AFOfflineDictationCapable(void)
{
    if (verbose) puts("STUB: AFOfflineDictationCapable called");
    return NULL;
}

void* AFOfflineDictationCapableWithoutOverride(void)
{
    if (verbose) puts("STUB: AFOfflineDictationCapableWithoutOverride called");
    return NULL;
}

void* AFOfflineDictationLanguageForKeyboardLanguage(void)
{
    if (verbose) puts("STUB: AFOfflineDictationLanguageForKeyboardLanguage called");
    return NULL;
}

void* AFOfflineDictationStatusStringForVersion(void)
{
    if (verbose) puts("STUB: AFOfflineDictationStatusStringForVersion called");
    return NULL;
}

void* AFOfflineDictationStatusStringIsInstalled(void)
{
    if (verbose) puts("STUB: AFOfflineDictationStatusStringIsInstalled called");
    return NULL;
}

void* AFOfflineDictationStatusStringIsInstalling(void)
{
    if (verbose) puts("STUB: AFOfflineDictationStatusStringIsInstalling called");
    return NULL;
}

void* AFOpportuneSpeakingModelFeedbackGetDescription(void)
{
    if (verbose) puts("STUB: AFOpportuneSpeakingModelFeedbackGetDescription called");
    return NULL;
}

void* AFOpportuneSpeakingModelServiceInterface(void)
{
    if (verbose) puts("STUB: AFOpportuneSpeakingModelServiceInterface called");
    return NULL;
}

void* AFOpportuneSpeakingModelTypeGetDescription(void)
{
    if (verbose) puts("STUB: AFOpportuneSpeakingModelTypeGetDescription called");
    return NULL;
}

void* AFOutputVoiceLanguageForRecognitionLanguage(void)
{
    if (verbose) puts("STUB: AFOutputVoiceLanguageForRecognitionLanguage called");
    return NULL;
}

void* AFOverrideLanguageCode(void)
{
    if (verbose) puts("STUB: AFOverrideLanguageCode called");
    return NULL;
}

void* AFPairedDeviceIsVictory(void)
{
    if (verbose) puts("STUB: AFPairedDeviceIsVictory called");
    return NULL;
}

void* AFPauseCommandsInterruptAudio(void)
{
    if (verbose) puts("STUB: AFPauseCommandsInterruptAudio called");
    return NULL;
}

void* AFPendingSpeechRequestServiceInterface(void)
{
    if (verbose) puts("STUB: AFPendingSpeechRequestServiceInterface called");
    return NULL;
}

void* AFPerformDeviceVolumeAction(void)
{
    if (verbose) puts("STUB: AFPerformDeviceVolumeAction called");
    return NULL;
}

void* AFPlaybackRouteIsHandsFree(void)
{
    if (verbose) puts("STUB: AFPlaybackRouteIsHandsFree called");
    return NULL;
}

void* AFPreferencesATVStopRecordingDelay(void)
{
    if (verbose) puts("STUB: AFPreferencesATVStopRecordingDelay called");
    return NULL;
}

void* AFPreferencesAlwaysEyesFreeEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesAlwaysEyesFreeEnabled called");
    return NULL;
}

void* AFPreferencesAssistantAllowed(void)
{
    if (verbose) puts("STUB: AFPreferencesAssistantAllowed called");
    return NULL;
}

void* AFPreferencesAssistantEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesAssistantEnabled called");
    return NULL;
}

void* AFPreferencesAudioDuckingAllowed(void)
{
    if (verbose) puts("STUB: AFPreferencesAudioDuckingAllowed called");
    return NULL;
}

void* AFPreferencesBorealisEducationHeaderDisplayCount(void)
{
    if (verbose) puts("STUB: AFPreferencesBorealisEducationHeaderDisplayCount called");
    return NULL;
}

void* AFPreferencesBufferedSpeechCapturePacketInterval(void)
{
    if (verbose) puts("STUB: AFPreferencesBufferedSpeechCapturePacketInterval called");
    return NULL;
}

void* AFPreferencesDictationAllowed(void)
{
    if (verbose) puts("STUB: AFPreferencesDictationAllowed called");
    return NULL;
}

void* AFPreferencesDisableDictationMode(void)
{
    if (verbose) puts("STUB: AFPreferencesDisableDictationMode called");
    return NULL;
}

void* AFPreferencesHoldToTalkForTypeToSiriEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesHoldToTalkForTypeToSiriEnabled called");
    return NULL;
}

void* AFPreferencesHorsemanSupplementalLanguageDictionary(void)
{
    if (verbose) puts("STUB: AFPreferencesHorsemanSupplementalLanguageDictionary called");
    return NULL;
}

void* AFPreferencesIgnoreServerManualEndpointingThreshold(void)
{
    if (verbose) puts("STUB: AFPreferencesIgnoreServerManualEndpointingThreshold called");
    return NULL;
}

void* AFPreferencesLanguageCodeForUser(void)
{
    if (verbose) puts("STUB: AFPreferencesLanguageCodeForUser called");
    return NULL;
}

void* AFPreferencesLanguageIsExplicitlySupportedForDictation(void)
{
    if (verbose) puts("STUB: AFPreferencesLanguageIsExplicitlySupportedForDictation called");
    return NULL;
}

void* AFPreferencesLanguageIsSupported(void)
{
    if (verbose) puts("STUB: AFPreferencesLanguageIsSupported called");
    return NULL;
}

void* AFPreferencesLanguageIsSupportedForDeviceSettings(void)
{
    if (verbose) puts("STUB: AFPreferencesLanguageIsSupportedForDeviceSettings called");
    return NULL;
}

void* AFPreferencesLanguageIsSupportedForDictation(void)
{
    if (verbose) puts("STUB: AFPreferencesLanguageIsSupportedForDictation called");
    return NULL;
}

void* AFPreferencesManualEndpointingThreshold(void)
{
    if (verbose) puts("STUB: AFPreferencesManualEndpointingThreshold called");
    return NULL;
}

void* AFPreferencesMobileUserSessionLanguage(void)
{
    if (verbose) puts("STUB: AFPreferencesMobileUserSessionLanguage called");
    return NULL;
}

void* AFPreferencesProfanityFilterForced(void)
{
    if (verbose) puts("STUB: AFPreferencesProfanityFilterForced called");
    return NULL;
}

void* AFPreferencesSetAlwaysEyesFreeEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesSetAlwaysEyesFreeEnabled called");
    return NULL;
}

void* AFPreferencesSetBorealisEducationHeaderDisplayCount(void)
{
    if (verbose) puts("STUB: AFPreferencesSetBorealisEducationHeaderDisplayCount called");
    return NULL;
}

void* AFPreferencesSetHoldToTalkForTypeToSiriEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesSetHoldToTalkForTypeToSiriEnabled called");
    return NULL;
}

void* AFPreferencesSetHorsemanSupplementalLanguageDictionary(void)
{
    if (verbose) puts("STUB: AFPreferencesSetHorsemanSupplementalLanguageDictionary called");
    return NULL;
}

void* AFPreferencesSetIgnoreServerManualEndpointingThreshold(void)
{
    if (verbose) puts("STUB: AFPreferencesSetIgnoreServerManualEndpointingThreshold called");
    return NULL;
}

void* AFPreferencesSetManualEndpointingThreshold(void)
{
    if (verbose) puts("STUB: AFPreferencesSetManualEndpointingThreshold called");
    return NULL;
}

void* AFPreferencesSetShouldIgnoreInternalConfig(void)
{
    if (verbose) puts("STUB: AFPreferencesSetShouldIgnoreInternalConfig called");
    return NULL;
}

void* AFPreferencesSetStartAlertEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesSetStartAlertEnabled called");
    return NULL;
}

void* AFPreferencesSetStreamingDictationEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesSetStreamingDictationEnabled called");
    return NULL;
}

void* AFPreferencesSetTypeToSiriEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesSetTypeToSiriEnabled called");
    return NULL;
}

void* AFPreferencesShouldIgnoreInternalConfig(void)
{
    if (verbose) puts("STUB: AFPreferencesShouldIgnoreInternalConfig called");
    return NULL;
}

void* AFPreferencesSimulateImmediateSpeechCapture(void)
{
    if (verbose) puts("STUB: AFPreferencesSimulateImmediateSpeechCapture called");
    return NULL;
}

void* AFPreferencesSiriTaskBackgroundAssertionTimeout(void)
{
    if (verbose) puts("STUB: AFPreferencesSiriTaskBackgroundAssertionTimeout called");
    return NULL;
}

void* AFPreferencesSiriTaskResponseTimeout(void)
{
    if (verbose) puts("STUB: AFPreferencesSiriTaskResponseTimeout called");
    return NULL;
}

void* AFPreferencesStartAlertEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesStartAlertEnabled called");
    return NULL;
}

void* AFPreferencesStreamingDictationEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesStreamingDictationEnabled called");
    return NULL;
}

void* AFPreferencesSupplementalLanguages(void)
{
    if (verbose) puts("STUB: AFPreferencesSupplementalLanguages called");
    return NULL;
}

void* AFPreferencesSupplementalLanguagesModificationDate(void)
{
    if (verbose) puts("STUB: AFPreferencesSupplementalLanguagesModificationDate called");
    return NULL;
}

void* AFPreferencesSupportedDictationLanguages(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedDictationLanguages called");
    return NULL;
}

void* AFPreferencesSupportedDictationLanguagesSet(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedDictationLanguagesSet called");
    return NULL;
}

void* AFPreferencesSupportedLanguages(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedLanguages called");
    return NULL;
}

void* AFPreferencesSupportedLanguagesForCurrentStoreFront(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedLanguagesForCurrentStoreFront called");
    return NULL;
}

void* AFPreferencesSupportedLanguagesForRemote(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedLanguagesForRemote called");
    return NULL;
}

void* AFPreferencesSupportedLanguagesForStorefrontIdentifier(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedLanguagesForStorefrontIdentifier called");
    return NULL;
}

void* AFPreferencesSupportedMultiUserLanguages(void)
{
    if (verbose) puts("STUB: AFPreferencesSupportedMultiUserLanguages called");
    return NULL;
}

void* AFPreferencesSynchronizeSupportedLanguages(void)
{
    if (verbose) puts("STUB: AFPreferencesSynchronizeSupportedLanguages called");
    return NULL;
}

void* AFPreferencesTestAccessEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesTestAccessEnabled called");
    return NULL;
}

void* AFPreferencesTypeToSiriEnabled(void)
{
    if (verbose) puts("STUB: AFPreferencesTypeToSiriEnabled called");
    return NULL;
}

void* AFPreferencesUserGeneratedContentAllowed(void)
{
    if (verbose) puts("STUB: AFPreferencesUserGeneratedContentAllowed called");
    return NULL;
}

void* AFPresentationPluginsURL(void)
{
    if (verbose) puts("STUB: AFPresentationPluginsURL called");
    return NULL;
}

void* AFProductName(void)
{
    if (verbose) puts("STUB: AFProductName called");
    return NULL;
}

void* AFProductType(void)
{
    if (verbose) puts("STUB: AFProductType called");
    return NULL;
}

void* AFProductVersion(void)
{
    if (verbose) puts("STUB: AFProductVersion called");
    return NULL;
}

void* AFRecordCoreDuetContext(void)
{
    if (verbose) puts("STUB: AFRecordCoreDuetContext called");
    return NULL;
}

void* AFRecordCoreDuetEvent(void)
{
    if (verbose) puts("STUB: AFRecordCoreDuetEvent called");
    return NULL;
}

void* AFRecordCoreDuetEventAtTimestamps(void)
{
    if (verbose) puts("STUB: AFRecordCoreDuetEventAtTimestamps called");
    return NULL;
}

void* AFRecordCoreDuetEventWithStream(void)
{
    if (verbose) puts("STUB: AFRecordCoreDuetEventWithStream called");
    return NULL;
}

void* AFRecordRouteIsHearst(void)
{
    if (verbose) puts("STUB: AFRecordRouteIsHearst called");
    return NULL;
}

void* AFRequestCancellationReasonGetFromName(void)
{
    if (verbose) puts("STUB: AFRequestCancellationReasonGetFromName called");
    return NULL;
}

void* AFRequestCancellationReasonGetIsValid(void)
{
    if (verbose) puts("STUB: AFRequestCancellationReasonGetIsValid called");
    return NULL;
}

void* AFRequestCancellationReasonGetName(void)
{
    if (verbose) puts("STUB: AFRequestCancellationReasonGetName called");
    return NULL;
}

void* AFSecurityDecryptData(void)
{
    if (verbose) puts("STUB: AFSecurityDecryptData called");
    return NULL;
}

void* AFSecurityDigestChunksWithProvider(void)
{
    if (verbose) puts("STUB: AFSecurityDigestChunksWithProvider called");
    return NULL;
}

void* AFSecurityDigestData(void)
{
    if (verbose) puts("STUB: AFSecurityDigestData called");
    return NULL;
}

void* AFSecurityEncryptData(void)
{
    if (verbose) puts("STUB: AFSecurityEncryptData called");
    return NULL;
}

void* AFSecurityGenerateRandomData(void)
{
    if (verbose) puts("STUB: AFSecurityGenerateRandomData called");
    return NULL;
}

void* AFSecurityServiceGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFSecurityServiceGetXPCInterface called");
    return NULL;
}

void* AFSerializeNSUserActivity(void)
{
    if (verbose) puts("STUB: AFSerializeNSUserActivity called");
    return NULL;
}

void* AFServicePluginsURL(void)
{
    if (verbose) puts("STUB: AFServicePluginsURL called");
    return NULL;
}

void* AFSetSpokenNotificationDataCollectionEnabled(void)
{
    if (verbose) puts("STUB: AFSetSpokenNotificationDataCollectionEnabled called");
    return NULL;
}

void* AFSettingsServiceDelegateXPCInterface(void)
{
    if (verbose) puts("STUB: AFSettingsServiceDelegateXPCInterface called");
    return NULL;
}

void* AFSettingsServiceXPCInterface(void)
{
    if (verbose) puts("STUB: AFSettingsServiceXPCInterface called");
    return NULL;
}

void* AFSharedResourcesDirectory(void)
{
    if (verbose) puts("STUB: AFSharedResourcesDirectory called");
    return NULL;
}

void* AFShouldRunDESRecording(void)
{
    if (verbose) puts("STUB: AFShouldRunDESRecording called");
    return NULL;
}

void* AFShouldUseLanguageDetector(void)
{
    if (verbose) puts("STUB: AFShouldUseLanguageDetector called");
    return NULL;
}

void* AFSiriActivationActionTypeGetFromName(void)
{
    if (verbose) puts("STUB: AFSiriActivationActionTypeGetFromName called");
    return NULL;
}

void* AFSiriActivationActionTypeGetIsValid(void)
{
    if (verbose) puts("STUB: AFSiriActivationActionTypeGetIsValid called");
    return NULL;
}

void* AFSiriActivationActionTypeGetName(void)
{
    if (verbose) puts("STUB: AFSiriActivationActionTypeGetName called");
    return NULL;
}

void* AFSiriActivationBluetoothDeviceBoronSignal(void)
{
    if (verbose) puts("STUB: AFSiriActivationBluetoothDeviceBoronSignal called");
    return NULL;
}

void* AFSiriActivationBluetoothDeviceDoubleTap(void)
{
    if (verbose) puts("STUB: AFSiriActivationBluetoothDeviceDoubleTap called");
    return NULL;
}

void* AFSiriActivationBluetoothDeviceTriggerless(void)
{
    if (verbose) puts("STUB: AFSiriActivationBluetoothDeviceTriggerless called");
    return NULL;
}

void* AFSiriActivationBluetoothDeviceVoice(void)
{
    if (verbose) puts("STUB: AFSiriActivationBluetoothDeviceVoice called");
    return NULL;
}

void* AFSiriActivationBluetoothDeviceVoiceTrigger(void)
{
    if (verbose) puts("STUB: AFSiriActivationBluetoothDeviceVoiceTrigger called");
    return NULL;
}

void* AFSiriActivationBluetoothDeviceVoiceTriggerPrewarm(void)
{
    if (verbose) puts("STUB: AFSiriActivationBluetoothDeviceVoiceTriggerPrewarm called");
    return NULL;
}

void* AFSiriActivationBuiltInMicVoice(void)
{
    if (verbose) puts("STUB: AFSiriActivationBuiltInMicVoice called");
    return NULL;
}

void* AFSiriActivationBuiltInMicVoiceTrigger(void)
{
    if (verbose) puts("STUB: AFSiriActivationBuiltInMicVoiceTrigger called");
    return NULL;
}

void* AFSiriActivationBuiltInMicVoiceTriggerPrewarm(void)
{
    if (verbose) puts("STUB: AFSiriActivationBuiltInMicVoiceTriggerPrewarm called");
    return NULL;
}

void* AFSiriActivationCarPlayDeviceVoice(void)
{
    if (verbose) puts("STUB: AFSiriActivationCarPlayDeviceVoice called");
    return NULL;
}

void* AFSiriActivationCarPlayDeviceVoiceTrigger(void)
{
    if (verbose) puts("STUB: AFSiriActivationCarPlayDeviceVoiceTrigger called");
    return NULL;
}

void* AFSiriActivationCarPlayDeviceVoiceTriggerPrewarm(void)
{
    if (verbose) puts("STUB: AFSiriActivationCarPlayDeviceVoiceTriggerPrewarm called");
    return NULL;
}

void* AFSiriActivationCreateContextFromNSDictionary(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateContextFromNSDictionary called");
    return NULL;
}

void* AFSiriActivationCreateContextFromXPCDictionary(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateContextFromXPCDictionary called");
    return NULL;
}

void* AFSiriActivationCreateNSDictionaryFromContext(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateNSDictionaryFromContext called");
    return NULL;
}

void* AFSiriActivationCreateRequestInfoFromContext(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateRequestInfoFromContext called");
    return NULL;
}

void* AFSiriActivationCreateResultFromXPCDictionary(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateResultFromXPCDictionary called");
    return NULL;
}

void* AFSiriActivationCreateXPCDictionaryFromContext(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateXPCDictionaryFromContext called");
    return NULL;
}

void* AFSiriActivationCreateXPCDictionaryFromResult(void)
{
    if (verbose) puts("STUB: AFSiriActivationCreateXPCDictionaryFromResult called");
    return NULL;
}

void* AFSiriActivationEventGetFromName(void)
{
    if (verbose) puts("STUB: AFSiriActivationEventGetFromName called");
    return NULL;
}

void* AFSiriActivationEventGetIsValid(void)
{
    if (verbose) puts("STUB: AFSiriActivationEventGetIsValid called");
    return NULL;
}

void* AFSiriActivationEventGetName(void)
{
    if (verbose) puts("STUB: AFSiriActivationEventGetName called");
    return NULL;
}

void* AFSiriActivationGetHandlerIDFromSourceAndEvent(void)
{
    if (verbose) puts("STUB: AFSiriActivationGetHandlerIDFromSourceAndEvent called");
    return NULL;
}

void* AFSiriActivationHearst(void)
{
    if (verbose) puts("STUB: AFSiriActivationHearst called");
    return NULL;
}

void* AFSiriActivationHearstBoron(void)
{
    if (verbose) puts("STUB: AFSiriActivationHearstBoron called");
    return NULL;
}

void* AFSiriActivationHearstDoubleTap(void)
{
    if (verbose) puts("STUB: AFSiriActivationHearstDoubleTap called");
    return NULL;
}

void* AFSiriActivationHearstVoice(void)
{
    if (verbose) puts("STUB: AFSiriActivationHearstVoice called");
    return NULL;
}

void* AFSiriActivationOptionsGetNames(void)
{
    if (verbose) puts("STUB: AFSiriActivationOptionsGetNames called");
    return NULL;
}

void* AFSiriActivationOptionsValidate(void)
{
    if (verbose) puts("STUB: AFSiriActivationOptionsValidate called");
    return NULL;
}

void* AFSiriActivationPerform(void)
{
    if (verbose) puts("STUB: AFSiriActivationPerform called");
    return NULL;
}

void* AFSiriActivationPerformLegacy(void)
{
    if (verbose) puts("STUB: AFSiriActivationPerformLegacy called");
    return NULL;
}

void* AFSiriActivationRaiseToSpeak(void)
{
    if (verbose) puts("STUB: AFSiriActivationRaiseToSpeak called");
    return NULL;
}

void* AFSiriActivationRaiseToSpeakPrewarm(void)
{
    if (verbose) puts("STUB: AFSiriActivationRaiseToSpeakPrewarm called");
    return NULL;
}

void* AFSiriActivationServiceGetMachServiceName(void)
{
    if (verbose) puts("STUB: AFSiriActivationServiceGetMachServiceName called");
    return NULL;
}

void* AFSiriActivationServiceGetPort(void)
{
    if (verbose) puts("STUB: AFSiriActivationServiceGetPort called");
    return NULL;
}

void* AFSiriActivationServiceGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFSiriActivationServiceGetXPCInterface called");
    return NULL;
}

void* AFSiriActivationServicePortGetFromName(void)
{
    if (verbose) puts("STUB: AFSiriActivationServicePortGetFromName called");
    return NULL;
}

void* AFSiriActivationServicePortGetIsValid(void)
{
    if (verbose) puts("STUB: AFSiriActivationServicePortGetIsValid called");
    return NULL;
}

void* AFSiriActivationServicePortGetName(void)
{
    if (verbose) puts("STUB: AFSiriActivationServicePortGetName called");
    return NULL;
}

void* AFSiriActivationSetOverrideHandlerMapProvider(void)
{
    if (verbose) puts("STUB: AFSiriActivationSetOverrideHandlerMapProvider called");
    return NULL;
}

void* AFSiriActivationSourceGetFromName(void)
{
    if (verbose) puts("STUB: AFSiriActivationSourceGetFromName called");
    return NULL;
}

void* AFSiriActivationSourceGetIsValid(void)
{
    if (verbose) puts("STUB: AFSiriActivationSourceGetIsValid called");
    return NULL;
}

void* AFSiriActivationSourceGetName(void)
{
    if (verbose) puts("STUB: AFSiriActivationSourceGetName called");
    return NULL;
}

void* AFSiriActivationTriggerless(void)
{
    if (verbose) puts("STUB: AFSiriActivationTriggerless called");
    return NULL;
}

void* AFSiriActivationValidate(void)
{
    if (verbose) puts("STUB: AFSiriActivationValidate called");
    return NULL;
}

void* AFSiriDataSharingOptInStatusGetFromName(void)
{
    if (verbose) puts("STUB: AFSiriDataSharingOptInStatusGetFromName called");
    return NULL;
}

void* AFSiriDataSharingOptInStatusGetIsValid(void)
{
    if (verbose) puts("STUB: AFSiriDataSharingOptInStatusGetIsValid called");
    return NULL;
}

void* AFSiriDataSharingOptInStatusGetName(void)
{
    if (verbose) puts("STUB: AFSiriDataSharingOptInStatusGetName called");
    return NULL;
}

void* AFSiriDeactivationOptionsGetNames(void)
{
    if (verbose) puts("STUB: AFSiriDeactivationOptionsGetNames called");
    return NULL;
}

void* AFSiriDeactivationOptionsValidate(void)
{
    if (verbose) puts("STUB: AFSiriDeactivationOptionsValidate called");
    return NULL;
}

void* AFSiriDeactivationReasonGetFromName(void)
{
    if (verbose) puts("STUB: AFSiriDeactivationReasonGetFromName called");
    return NULL;
}

void* AFSiriDeactivationReasonGetIsValid(void)
{
    if (verbose) puts("STUB: AFSiriDeactivationReasonGetIsValid called");
    return NULL;
}

void* AFSiriDeactivationReasonGetName(void)
{
    if (verbose) puts("STUB: AFSiriDeactivationReasonGetName called");
    return NULL;
}

void* AFSiriLocationAuthorizationStatus(void)
{
    if (verbose) puts("STUB: AFSiriLocationAuthorizationStatus called");
    return NULL;
}

void* AFSiriResponseHandlingInterface(void)
{
    if (verbose) puts("STUB: AFSiriResponseHandlingInterface called");
    return NULL;
}

void* AFSiriUsageDescriptionForAppID(void)
{
    if (verbose) puts("STUB: AFSiriUsageDescriptionForAppID called");
    return NULL;
}

void* AFSiriUsageResultHandlingInterface(void)
{
    if (verbose) puts("STUB: AFSiriUsageResultHandlingInterface called");
    return NULL;
}

void* AFSpeechEndpointerOperationModeGetFromName(void)
{
    if (verbose) puts("STUB: AFSpeechEndpointerOperationModeGetFromName called");
    return NULL;
}

void* AFSpeechEndpointerOperationModeGetIsValid(void)
{
    if (verbose) puts("STUB: AFSpeechEndpointerOperationModeGetIsValid called");
    return NULL;
}

void* AFSpeechEndpointerOperationModeGetName(void)
{
    if (verbose) puts("STUB: AFSpeechEndpointerOperationModeGetName called");
    return NULL;
}

void* AFSpeechEventGetDescription(void)
{
    if (verbose) puts("STUB: AFSpeechEventGetDescription called");
    return NULL;
}

void* AFSpeechEventIsBluetooth(void)
{
    if (verbose) puts("STUB: AFSpeechEventIsBluetooth called");
    return NULL;
}

void* AFSpeechEventIsRemotePlugin(void)
{
    if (verbose) puts("STUB: AFSpeechEventIsRemotePlugin called");
    return NULL;
}

void* AFSpeechEventIsTVRemote(void)
{
    if (verbose) puts("STUB: AFSpeechEventIsTVRemote called");
    return NULL;
}

void* AFSpeechEventIsVoiceTrigger(void)
{
    if (verbose) puts("STUB: AFSpeechEventIsVoiceTrigger called");
    return NULL;
}

void* AFSpeechLogsDirectory(void)
{
    if (verbose) puts("STUB: AFSpeechLogsDirectory called");
    return NULL;
}

void* AFSpeechRecognitionUsageDescriptionForAppID(void)
{
    if (verbose) puts("STUB: AFSpeechRecognitionUsageDescriptionForAppID called");
    return NULL;
}

void* AFSpeechRecordingEventListenerGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFSpeechRecordingEventListenerGetXPCInterface called");
    return NULL;
}

void* AFSpeechServiceDelegateGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFSpeechServiceDelegateGetXPCInterface called");
    return NULL;
}

void* AFSpeechServiceGetXPCInterface(void)
{
    if (verbose) puts("STUB: AFSpeechServiceGetXPCInterface called");
    return NULL;
}

void* AFSpokenMessagesExecutionTimeout(void)
{
    if (verbose) puts("STUB: AFSpokenMessagesExecutionTimeout called");
    return NULL;
}

void* AFSpokenNotificationDataCollectionEnabled(void)
{
    if (verbose) puts("STUB: AFSpokenNotificationDataCollectionEnabled called");
    return NULL;
}

void* AFStringFromMRPlaybackState(void)
{
    if (verbose) puts("STUB: AFStringFromMRPlaybackState called");
    return NULL;
}

void* AFSupportBaseURL(void)
{
    if (verbose) puts("STUB: AFSupportBaseURL called");
    return NULL;
}

void* AFSupportPreferencesSynchronize(void)
{
    if (verbose) puts("STUB: AFSupportPreferencesSynchronize called");
    return NULL;
}

void* AFSupportsAirPlayEndpointRoute(void)
{
    if (verbose) puts("STUB: AFSupportsAirPlayEndpointRoute called");
    return NULL;
}

void* AFSupportsMultiUser(void)
{
    if (verbose) puts("STUB: AFSupportsMultiUser called");
    return NULL;
}

void* AFSupportsOdeon(void)
{
    if (verbose) puts("STUB: AFSupportsOdeon called");
    return NULL;
}

void* AFSupportsPairedDevice(void)
{
    if (verbose) puts("STUB: AFSupportsPairedDevice called");
    return NULL;
}

void* AFSupportsPlayThisOnThat(void)
{
    if (verbose) puts("STUB: AFSupportsPlayThisOnThat called");
    return NULL;
}

void* AFSupportsPlayThisOnThatPlayback(void)
{
    if (verbose) puts("STUB: AFSupportsPlayThisOnThatPlayback called");
    return NULL;
}

void* AFSupportsSiriSpeaks(void)
{
    if (verbose) puts("STUB: AFSupportsSiriSpeaks called");
    return NULL;
}

void* AFSupportsVibration(void)
{
    if (verbose) puts("STUB: AFSupportsVibration called");
    return NULL;
}

void* AFSupportsWAPI(void)
{
    if (verbose) puts("STUB: AFSupportsWAPI called");
    return NULL;
}

void* AFSupportsWatchListKit(void)
{
    if (verbose) puts("STUB: AFSupportsWatchListKit called");
    return NULL;
}

void* AFSupportsWirelessSplitter(void)
{
    if (verbose) puts("STUB: AFSupportsWirelessSplitter called");
    return NULL;
}

void* AFSyncKeysThatNeedToBeSynced(void)
{
    if (verbose) puts("STUB: AFSyncKeysThatNeedToBeSynced called");
    return NULL;
}

void* AFSystemGetBootTimeInMicroseconds(void)
{
    if (verbose) puts("STUB: AFSystemGetBootTimeInMicroseconds called");
    return NULL;
}

void* AFSystemGetBootTimeInTimeInterval(void)
{
    if (verbose) puts("STUB: AFSystemGetBootTimeInTimeInterval called");
    return NULL;
}

void* AFSystemRootDirectory(void)
{
    if (verbose) puts("STUB: AFSystemRootDirectory called");
    return NULL;
}

void* AFTruncateMachTimeToTenthOfASecond(void)
{
    if (verbose) puts("STUB: AFTruncateMachTimeToTenthOfASecond called");
    return NULL;
}

void* AFTurnIdentifierCreateFromBytes(void)
{
    if (verbose) puts("STUB: AFTurnIdentifierCreateFromBytes called");
    return NULL;
}

void* AFTurnIdentifierGenerate(void)
{
    if (verbose) puts("STUB: AFTurnIdentifierGenerate called");
    return NULL;
}

void* AFTurnIdentifierGetBytes(void)
{
    if (verbose) puts("STUB: AFTurnIdentifierGetBytes called");
    return NULL;
}

void* AFTurnIdentifierGetString(void)
{
    if (verbose) puts("STUB: AFTurnIdentifierGetString called");
    return NULL;
}

void* AFUIBundlesURL(void)
{
    if (verbose) puts("STUB: AFUIBundlesURL called");
    return NULL;
}

void* AFUserAssignedDeviceName(void)
{
    if (verbose) puts("STUB: AFUserAssignedDeviceName called");
    return NULL;
}

void* AFUserSupportDirectoryPath(void)
{
    if (verbose) puts("STUB: AFUserSupportDirectoryPath called");
    return NULL;
}

void* AFWatchOSVersion(void)
{
    if (verbose) puts("STUB: AFWatchOSVersion called");
    return NULL;
}

void* AFWatchOSVersionIsGreaterThanOrEqual(void)
{
    if (verbose) puts("STUB: AFWatchOSVersionIsGreaterThanOrEqual called");
    return NULL;
}

void* SAMPPlayStateFromMRPlaybackState(void)
{
    if (verbose) puts("STUB: SAMPPlayStateFromMRPlaybackState called");
    return NULL;
}

void* SAVoiceFromVoiceAsset(void)
{
    if (verbose) puts("STUB: SAVoiceFromVoiceAsset called");
    return NULL;
}

void* SAVoiceFromVoiceResourceAsset(void)
{
    if (verbose) puts("STUB: SAVoiceFromVoiceResourceAsset called");
    return NULL;
}

void* SISchemaClientAnyEventReadFrom(void)
{
    if (verbose) puts("STUB: SISchemaClientAnyEventReadFrom called");
    return NULL;
}

void* SISchemaClientTurnBasedEventReadFrom(void)
{
    if (verbose) puts("STUB: SISchemaClientTurnBasedEventReadFrom called");
    return NULL;
}

void* SISchemaClientTurnContextReadFrom(void)
{
    if (verbose) puts("STUB: SISchemaClientTurnContextReadFrom called");
    return NULL;
}

void* StarkHasBeenActiveInLast24Hours(void)
{
    if (verbose) puts("STUB: StarkHasBeenActiveInLast24Hours called");
    return NULL;
}

void* _AFBackedUpPreferencesBoolValueForKey(void)
{
    if (verbose) puts("STUB: _AFBackedUpPreferencesBoolValueForKey called");
    return NULL;
}

void* _AFBackedUpPreferencesBoolValueForKeyWithDefault(void)
{
    if (verbose) puts("STUB: _AFBackedUpPreferencesBoolValueForKeyWithDefault called");
    return NULL;
}

void* _AFBackedUpPreferencesKeyList(void)
{
    if (verbose) puts("STUB: _AFBackedUpPreferencesKeyList called");
    return NULL;
}

void* _AFBackedUpPreferencesSetValueForKey(void)
{
    if (verbose) puts("STUB: _AFBackedUpPreferencesSetValueForKey called");
    return NULL;
}

void* _AFBackedUpPreferencesValueForKey(void)
{
    if (verbose) puts("STUB: _AFBackedUpPreferencesValueForKey called");
    return NULL;
}

void* _AFBackedUpPreferencesValueForKeyForUser(void)
{
    if (verbose) puts("STUB: _AFBackedUpPreferencesValueForKeyForUser called");
    return NULL;
}

void* _AFDictationLanguageForKeyboardLanguageAndConfiguration(void)
{
    if (verbose) puts("STUB: _AFDictationLanguageForKeyboardLanguageAndConfiguration called");
    return NULL;
}

void* _AFMemoryPressureConditionGetName(void)
{
    if (verbose) puts("STUB: _AFMemoryPressureConditionGetName called");
    return NULL;
}

void* _AFNanoPreferencesSetValueForKey(void)
{
    if (verbose) puts("STUB: _AFNanoPreferencesSetValueForKey called");
    return NULL;
}

void* _AFNanoPreferencesValueForKey(void)
{
    if (verbose) puts("STUB: _AFNanoPreferencesValueForKey called");
    return NULL;
}

void* _AFPreferencesAcousticIDAutoEndpoint(void)
{
    if (verbose) puts("STUB: _AFPreferencesAcousticIDAutoEndpoint called");
    return NULL;
}

void* _AFPreferencesAlwaysUseLocalScripts(void)
{
    if (verbose) puts("STUB: _AFPreferencesAlwaysUseLocalScripts called");
    return NULL;
}

void* _AFPreferencesApplySASToFirstPartyDomains(void)
{
    if (verbose) puts("STUB: _AFPreferencesApplySASToFirstPartyDomains called");
    return NULL;
}

void* _AFPreferencesAuthenticationDisabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesAuthenticationDisabled called");
    return NULL;
}

void* _AFPreferencesBLECompanionConnectionsEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesBLECompanionConnectionsEnabled called");
    return NULL;
}

void* _AFPreferencesBuiltIniOSLanguages(void)
{
    if (verbose) puts("STUB: _AFPreferencesBuiltIniOSLanguages called");
    return NULL;
}

void* _AFPreferencesClearOpportuneSpeakingEdgeDetectorSignalOverride(void)
{
    if (verbose) puts("STUB: _AFPreferencesClearOpportuneSpeakingEdgeDetectorSignalOverride called");
    return NULL;
}

void* _AFPreferencesClearShouldCensorSpeechInternal(void)
{
    if (verbose) puts("STUB: _AFPreferencesClearShouldCensorSpeechInternal called");
    return NULL;
}

void* _AFPreferencesClearSpokenNotificationTemporarilyDisabledStatus(void)
{
    if (verbose) puts("STUB: _AFPreferencesClearSpokenNotificationTemporarilyDisabledStatus called");
    return NULL;
}

void* _AFPreferencesClientFlowLocalDevelopment(void)
{
    if (verbose) puts("STUB: _AFPreferencesClientFlowLocalDevelopment called");
    return NULL;
}

void* _AFPreferencesCloudSyncEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesCloudSyncEnabled called");
    return NULL;
}

void* _AFPreferencesCloudSyncEnabledModificationDate(void)
{
    if (verbose) puts("STUB: _AFPreferencesCloudSyncEnabledModificationDate called");
    return NULL;
}

void* _AFPreferencesCloudUserID(void)
{
    if (verbose) puts("STUB: _AFPreferencesCloudUserID called");
    return NULL;
}

void* _AFPreferencesCompanionMessagingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesCompanionMessagingEnabled called");
    return NULL;
}

void* _AFPreferencesCountryCode(void)
{
    if (verbose) puts("STUB: _AFPreferencesCountryCode called");
    return NULL;
}

void* _AFPreferencesDatabaseSyncingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesDatabaseSyncingEnabled called");
    return NULL;
}

void* _AFPreferencesDictationLanguageDetectorEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesDictationLanguageDetectorEnabled called");
    return NULL;
}

void* _AFPreferencesDictationLanguageDetectorEnabledByServer(void)
{
    if (verbose) puts("STUB: _AFPreferencesDictationLanguageDetectorEnabledByServer called");
    return NULL;
}

void* _AFPreferencesDictationRequestSamplingRates(void)
{
    if (verbose) puts("STUB: _AFPreferencesDictationRequestSamplingRates called");
    return NULL;
}

void* _AFPreferencesDisableTuscany(void)
{
    if (verbose) puts("STUB: _AFPreferencesDisableTuscany called");
    return NULL;
}

void* _AFPreferencesEnableBLECompanionConnections(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableBLECompanionConnections called");
    return NULL;
}

void* _AFPreferencesEnableCloudSync(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableCloudSync called");
    return NULL;
}

void* _AFPreferencesEnableServerDebugFlags(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableServerDebugFlags called");
    return NULL;
}

void* _AFPreferencesEnableSiriDebugUI(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableSiriDebugUI called");
    return NULL;
}

void* _AFPreferencesEnableSiriMini(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableSiriMini called");
    return NULL;
}

void* _AFPreferencesEnableSiriMiniExperimentation(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableSiriMiniExperimentation called");
    return NULL;
}

void* _AFPreferencesEnableSiriMiniPreferred(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableSiriMiniPreferred called");
    return NULL;
}

void* _AFPreferencesEnableSpeakerGradingDebugUI(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnableSpeakerGradingDebugUI called");
    return NULL;
}

void* _AFPreferencesEnabledDragAndDrop(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnabledDragAndDrop called");
    return NULL;
}

void* _AFPreferencesEnabledNonFullScreen(void)
{
    if (verbose) puts("STUB: _AFPreferencesEnabledNonFullScreen called");
    return NULL;
}

void* _AFPreferencesForceRTL(void)
{
    if (verbose) puts("STUB: _AFPreferencesForceRTL called");
    return NULL;
}

void* _AFPreferencesForceSpeexAudioRecording(void)
{
    if (verbose) puts("STUB: _AFPreferencesForceSpeexAudioRecording called");
    return NULL;
}

void* _AFPreferencesForceUncompressedAudioRecording(void)
{
    if (verbose) puts("STUB: _AFPreferencesForceUncompressedAudioRecording called");
    return NULL;
}

void* _AFPreferencesGetClientCommandDeliveryDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetClientCommandDeliveryDelay called");
    return NULL;
}

void* _AFPreferencesGetEnableServerDebugFlags(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetEnableServerDebugFlags called");
    return NULL;
}

void* _AFPreferencesGetForcedInterstitialStyle(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetForcedInterstitialStyle called");
    return NULL;
}

void* _AFPreferencesGetInitialInterstitialDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetInitialInterstitialDelay called");
    return NULL;
}

void* _AFPreferencesGetInitialInterstitialDelayForVoiceTriggerOneShotWithoutAudioDucking(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetInitialInterstitialDelayForVoiceTriggerOneShotWithoutAudioDucking called");
    return NULL;
}

void* _AFPreferencesGetInterstitialDisplayDisabledBeforeSpeechRecognition(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetInterstitialDisplayDisabledBeforeSpeechRecognition called");
    return NULL;
}

void* _AFPreferencesGetNonManateeIdentity(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetNonManateeIdentity called");
    return NULL;
}

void* _AFPreferencesGetTuscanyStatus(void)
{
    if (verbose) puts("STUB: _AFPreferencesGetTuscanyStatus called");
    return NULL;
}

void* _AFPreferencesIgnoreEVFailures(void)
{
    if (verbose) puts("STUB: _AFPreferencesIgnoreEVFailures called");
    return NULL;
}

void* _AFPreferencesInProgressOutputVoice(void)
{
    if (verbose) puts("STUB: _AFPreferencesInProgressOutputVoice called");
    return NULL;
}

void* _AFPreferencesIsSiriServerLoggingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesIsSiriServerLoggingEnabled called");
    return NULL;
}

void* _AFPreferencesLanguageCode(void)
{
    if (verbose) puts("STUB: _AFPreferencesLanguageCode called");
    return NULL;
}

void* _AFPreferencesLastPushedCensorSpeechSetting(void)
{
    if (verbose) puts("STUB: _AFPreferencesLastPushedCensorSpeechSetting called");
    return NULL;
}

void* _AFPreferencesLimitedPersistentAudioLogging(void)
{
    if (verbose) puts("STUB: _AFPreferencesLimitedPersistentAudioLogging called");
    return NULL;
}

void* _AFPreferencesLowerWristEducationHeaderDisplayCount(void)
{
    if (verbose) puts("STUB: _AFPreferencesLowerWristEducationHeaderDisplayCount called");
    return NULL;
}

void* _AFPreferencesMirroredTCCInfo(void)
{
    if (verbose) puts("STUB: _AFPreferencesMirroredTCCInfo called");
    return NULL;
}

void* _AFPreferencesMultiSiriLanguageScenarios(void)
{
    if (verbose) puts("STUB: _AFPreferencesMultiSiriLanguageScenarios called");
    return NULL;
}

void* _AFPreferencesMultilingualDictationTimeoutInMilliSeconds(void)
{
    if (verbose) puts("STUB: _AFPreferencesMultilingualDictationTimeoutInMilliSeconds called");
    return NULL;
}

void* _AFPreferencesMyriadConstantGoodness(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadConstantGoodness called");
    return NULL;
}

void* _AFPreferencesMyriadDeviceAdjust(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadDeviceAdjust called");
    return NULL;
}

void* _AFPreferencesMyriadDeviceClass(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadDeviceClass called");
    return NULL;
}

void* _AFPreferencesMyriadDeviceDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadDeviceDelay called");
    return NULL;
}

void* _AFPreferencesMyriadDeviceGroup(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadDeviceGroup called");
    return NULL;
}

void* _AFPreferencesMyriadDeviceSlowdown(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadDeviceSlowdown called");
    return NULL;
}

void* _AFPreferencesMyriadDeviceTrumpDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadDeviceTrumpDelay called");
    return NULL;
}

void* _AFPreferencesMyriadLastWin(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadLastWin called");
    return NULL;
}

void* _AFPreferencesMyriadServerHasProvisioned(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadServerHasProvisioned called");
    return NULL;
}

void* _AFPreferencesMyriadServerProvisioning(void)
{
    if (verbose) puts("STUB: _AFPreferencesMyriadServerProvisioning called");
    return NULL;
}

void* _AFPreferencesNetworkLoggingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesNetworkLoggingEnabled called");
    return NULL;
}

void* _AFPreferencesNetworkStackOverride(void)
{
    if (verbose) puts("STUB: _AFPreferencesNetworkStackOverride called");
    return NULL;
}

void* _AFPreferencesNotificationPreviewRestrictedApps(void)
{
    if (verbose) puts("STUB: _AFPreferencesNotificationPreviewRestrictedApps called");
    return NULL;
}

void* _AFPreferencesOpportuneSpeakingEdgeDetectorSignalOverride(void)
{
    if (verbose) puts("STUB: _AFPreferencesOpportuneSpeakingEdgeDetectorSignalOverride called");
    return NULL;
}

void* _AFPreferencesOpportuneSpeakingModelUsed(void)
{
    if (verbose) puts("STUB: _AFPreferencesOpportuneSpeakingModelUsed called");
    return NULL;
}

void* _AFPreferencesOpportuneSpeakingModuleEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesOpportuneSpeakingModuleEnabled called");
    return NULL;
}

void* _AFPreferencesOpportuneSpeakingPause(void)
{
    if (verbose) puts("STUB: _AFPreferencesOpportuneSpeakingPause called");
    return NULL;
}

void* _AFPreferencesOpportuneSpeakingTimeout(void)
{
    if (verbose) puts("STUB: _AFPreferencesOpportuneSpeakingTimeout called");
    return NULL;
}

void* _AFPreferencesOutputVoice(void)
{
    if (verbose) puts("STUB: _AFPreferencesOutputVoice called");
    return NULL;
}

void* _AFPreferencesPluginTimeoutDisabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesPluginTimeoutDisabled called");
    return NULL;
}

void* _AFPreferencesReplacementLanguageForLocalRecognizerLanguageCode(void)
{
    if (verbose) puts("STUB: _AFPreferencesReplacementLanguageForLocalRecognizerLanguageCode called");
    return NULL;
}

void* _AFPreferencesSetAcousticIDAutoEndpoint(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetAcousticIDAutoEndpoint called");
    return NULL;
}

void* _AFPreferencesSetAlwaysUseLocalScripts(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetAlwaysUseLocalScripts called");
    return NULL;
}

void* _AFPreferencesSetApplySASToFirstPartyDomains(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetApplySASToFirstPartyDomains called");
    return NULL;
}

void* _AFPreferencesSetClientFlowLocalDevelopment(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetClientFlowLocalDevelopment called");
    return NULL;
}

void* _AFPreferencesSetCloudSyncEnabledModificationDate(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetCloudSyncEnabledModificationDate called");
    return NULL;
}

void* _AFPreferencesSetCloudUserID(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetCloudUserID called");
    return NULL;
}

void* _AFPreferencesSetCompanionMessagingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetCompanionMessagingEnabled called");
    return NULL;
}

void* _AFPreferencesSetCountryCode(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetCountryCode called");
    return NULL;
}

void* _AFPreferencesSetDatabaseSyncingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetDatabaseSyncingEnabled called");
    return NULL;
}

void* _AFPreferencesSetDictationLanguageDetectorEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetDictationLanguageDetectorEnabled called");
    return NULL;
}

void* _AFPreferencesSetDictationLanguageDetectorEnabledByServer(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetDictationLanguageDetectorEnabledByServer called");
    return NULL;
}

void* _AFPreferencesSetEnableSiriDebugUI(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetEnableSiriDebugUI called");
    return NULL;
}

void* _AFPreferencesSetEnableSpeakerGradingDebugUI(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetEnableSpeakerGradingDebugUI called");
    return NULL;
}

void* _AFPreferencesSetEnabledDragAndDrop(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetEnabledDragAndDrop called");
    return NULL;
}

void* _AFPreferencesSetEnabledNonFullScreen(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetEnabledNonFullScreen called");
    return NULL;
}

void* _AFPreferencesSetForceRTL(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetForceRTL called");
    return NULL;
}

void* _AFPreferencesSetForceSpeexAudioRecording(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetForceSpeexAudioRecording called");
    return NULL;
}

void* _AFPreferencesSetForceUncompressedAudioRecording(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetForceUncompressedAudioRecording called");
    return NULL;
}

void* _AFPreferencesSetIgnoreEVFailures(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetIgnoreEVFailures called");
    return NULL;
}

void* _AFPreferencesSetInProgressOutputVoice(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetInProgressOutputVoice called");
    return NULL;
}

void* _AFPreferencesSetInitialInterstitialDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetInitialInterstitialDelay called");
    return NULL;
}

void* _AFPreferencesSetInitialInterstitialDelayForVoiceTriggerOneShotWithoutAudioDucking(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetInitialInterstitialDelayForVoiceTriggerOneShotWithoutAudioDucking called");
    return NULL;
}

void* _AFPreferencesSetLanguageCode(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetLanguageCode called");
    return NULL;
}

void* _AFPreferencesSetLastPushedCensorSpeechSetting(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetLastPushedCensorSpeechSetting called");
    return NULL;
}

void* _AFPreferencesSetLimitedPersistentAudioLogging(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetLimitedPersistentAudioLogging called");
    return NULL;
}

void* _AFPreferencesSetLowerWristEducationHeaderDisplayCount(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetLowerWristEducationHeaderDisplayCount called");
    return NULL;
}

void* _AFPreferencesSetMirroredTCCInfo(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMirroredTCCInfo called");
    return NULL;
}

void* _AFPreferencesSetMultilingualDictationTimeoutInMilliSeconds(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMultilingualDictationTimeoutInMilliSeconds called");
    return NULL;
}

void* _AFPreferencesSetMyriadConstantGoodness(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadConstantGoodness called");
    return NULL;
}

void* _AFPreferencesSetMyriadDeviceAdjust(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadDeviceAdjust called");
    return NULL;
}

void* _AFPreferencesSetMyriadDeviceClass(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadDeviceClass called");
    return NULL;
}

void* _AFPreferencesSetMyriadDeviceDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadDeviceDelay called");
    return NULL;
}

void* _AFPreferencesSetMyriadDeviceGroup(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadDeviceGroup called");
    return NULL;
}

void* _AFPreferencesSetMyriadDeviceSlowdown(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadDeviceSlowdown called");
    return NULL;
}

void* _AFPreferencesSetMyriadDeviceTrumpDelay(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadDeviceTrumpDelay called");
    return NULL;
}

void* _AFPreferencesSetMyriadLastWin(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadLastWin called");
    return NULL;
}

void* _AFPreferencesSetMyriadServerHasProvisioned(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadServerHasProvisioned called");
    return NULL;
}

void* _AFPreferencesSetMyriadServerProvisioning(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetMyriadServerProvisioning called");
    return NULL;
}

void* _AFPreferencesSetNetworkLoggingEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetNetworkLoggingEnabled called");
    return NULL;
}

void* _AFPreferencesSetNonManateeIdentity(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetNonManateeIdentity called");
    return NULL;
}

void* _AFPreferencesSetNotificationPreviewRestrictedApps(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetNotificationPreviewRestrictedApps called");
    return NULL;
}

void* _AFPreferencesSetOpportuneSpeakingEdgeDetectorSignalOverride(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetOpportuneSpeakingEdgeDetectorSignalOverride called");
    return NULL;
}

void* _AFPreferencesSetOpportuneSpeakingModelUsed(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetOpportuneSpeakingModelUsed called");
    return NULL;
}

void* _AFPreferencesSetOpportuneSpeakingModuleEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetOpportuneSpeakingModuleEnabled called");
    return NULL;
}

void* _AFPreferencesSetOpportuneSpeakingPause(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetOpportuneSpeakingPause called");
    return NULL;
}

void* _AFPreferencesSetOpportuneSpeakingTimeout(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetOpportuneSpeakingTimeout called");
    return NULL;
}

void* _AFPreferencesSetOutputVoice(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetOutputVoice called");
    return NULL;
}

void* _AFPreferencesSetPluginTimeoutDisabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetPluginTimeoutDisabled called");
    return NULL;
}

void* _AFPreferencesSetShouldCensorSpeechInternal(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetShouldCensorSpeechInternal called");
    return NULL;
}

void* _AFPreferencesSetShouldSyncFirstPartyAppIntentPolicy(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetShouldSyncFirstPartyAppIntentPolicy called");
    return NULL;
}

void* _AFPreferencesSetShowAllDialogVariantsEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetShowAllDialogVariantsEnabled called");
    return NULL;
}

void* _AFPreferencesSetSiriDataSharingOptInAlertPresented(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSiriDataSharingOptInAlertPresented called");
    return NULL;
}

void* _AFPreferencesSetSiriDataSharingOptInStatus(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSiriDataSharingOptInStatus called");
    return NULL;
}

void* _AFPreferencesSetSiriDesignModeEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSiriDesignModeEnabled called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationPlayAlertBeforeTriggerlessListening(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationPlayAlertBeforeTriggerlessListening called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationQuickIgnoreTime(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationQuickIgnoreTime called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationShouldAlwaysSpeakNotifications(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationShouldAlwaysSpeakNotifications called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationShouldSkipTriggerlessReplies(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationShouldSkipTriggerlessReplies called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationShowUIDuringTriggerlessListening(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationShowUIDuringTriggerlessListening called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationSkipTriggerlessReplyConfirmation(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationSkipTriggerlessReplyConfirmation called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationTemporarilyDisabledEndDate(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationTemporarilyDisabledEndDate called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationTemporarilyDisabledEndDateForApp(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationTemporarilyDisabledEndDateForApp called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationsEnabledForNano(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationsEnabledForNano called");
    return NULL;
}

void* _AFPreferencesSetSpokenNotificationsProxCardSeen(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetSpokenNotificationsProxCardSeen called");
    return NULL;
}

void* _AFPreferencesSetTFO(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetTFO called");
    return NULL;
}

void* _AFPreferencesSetUseMyriadCoordination(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetUseMyriadCoordination called");
    return NULL;
}

void* _AFPreferencesSetUseMyriadDucking(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetUseMyriadDucking called");
    return NULL;
}

void* _AFPreferencesSetUseSASAutoSelectionFeature(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetUseSASAutoSelectionFeature called");
    return NULL;
}

void* _AFPreferencesSetValueForKey(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetValueForKey called");
    return NULL;
}

void* _AFPreferencesSetVibrateWithAlert(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetVibrateWithAlert called");
    return NULL;
}

void* _AFPreferencesSetVibrationCount(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetVibrationCount called");
    return NULL;
}

void* _AFPreferencesSetVibrationDuration(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetVibrationDuration called");
    return NULL;
}

void* _AFPreferencesSetVibrationGap(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetVibrationGap called");
    return NULL;
}

void* _AFPreferencesSetVibrationIntensity(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetVibrationIntensity called");
    return NULL;
}

void* _AFPreferencesSetWebInspectorEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetWebInspectorEnabled called");
    return NULL;
}

void* _AFPreferencesSetiRATManagerRecommendationEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSetiRATManagerRecommendationEnabled called");
    return NULL;
}

void* _AFPreferencesShouldCensorSpeechInternal(void)
{
    if (verbose) puts("STUB: _AFPreferencesShouldCensorSpeechInternal called");
    return NULL;
}

void* _AFPreferencesShouldSuppressSiriDataSharingOptInAlert(void)
{
    if (verbose) puts("STUB: _AFPreferencesShouldSuppressSiriDataSharingOptInAlert called");
    return NULL;
}

void* _AFPreferencesShouldSyncFirstPartyAppIntentPolicy(void)
{
    if (verbose) puts("STUB: _AFPreferencesShouldSyncFirstPartyAppIntentPolicy called");
    return NULL;
}

void* _AFPreferencesShouldUseTFO(void)
{
    if (verbose) puts("STUB: _AFPreferencesShouldUseTFO called");
    return NULL;
}

void* _AFPreferencesShowAllDialogVariantsEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesShowAllDialogVariantsEnabled called");
    return NULL;
}

void* _AFPreferencesSiriDataSharingOptInStatus(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriDataSharingOptInStatus called");
    return NULL;
}

void* _AFPreferencesSiriDesignModeIsEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriDesignModeIsEnabled called");
    return NULL;
}

void* _AFPreferencesSiriMiniEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriMiniEnabled called");
    return NULL;
}

void* _AFPreferencesSiriMiniExperimentationEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriMiniExperimentationEnabled called");
    return NULL;
}

void* _AFPreferencesSiriMiniPreferredEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriMiniPreferredEnabled called");
    return NULL;
}

void* _AFPreferencesSiriMiniResultHoldTime(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriMiniResultHoldTime called");
    return NULL;
}

void* _AFPreferencesSiriMiniSupported(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriMiniSupported called");
    return NULL;
}

void* _AFPreferencesSiriPersonalizedASRDisabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSiriPersonalizedASRDisabled called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationPlayAlertBeforeTriggerlessListening(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationPlayAlertBeforeTriggerlessListening called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationQuickIgnoreTime(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationQuickIgnoreTime called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationShouldAlwaysSpeakNotifications(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationShouldAlwaysSpeakNotifications called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationShouldSkipTriggerlessReplies(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationShouldSkipTriggerlessReplies called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationShowUIDuringTriggerlessListening(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationShowUIDuringTriggerlessListening called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationSkipTriggerlessReplyConfirmation(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationSkipTriggerlessReplyConfirmation called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationTemporarilyDisabledEndDate(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationTemporarilyDisabledEndDate called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationTemporarilyDisabledEndDateForApp(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationTemporarilyDisabledEndDateForApp called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationsEnabledForNano(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationsEnabledForNano called");
    return NULL;
}

void* _AFPreferencesSpokenNotificationsProxCardSeen(void)
{
    if (verbose) puts("STUB: _AFPreferencesSpokenNotificationsProxCardSeen called");
    return NULL;
}

void* _AFPreferencesSyncFromWatchEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesSyncFromWatchEnabled called");
    return NULL;
}

void* _AFPreferencesUseMyriadCoordination(void)
{
    if (verbose) puts("STUB: _AFPreferencesUseMyriadCoordination called");
    return NULL;
}

void* _AFPreferencesUseMyriadDucking(void)
{
    if (verbose) puts("STUB: _AFPreferencesUseMyriadDucking called");
    return NULL;
}

void* _AFPreferencesUseSASAutoSelectionFeature(void)
{
    if (verbose) puts("STUB: _AFPreferencesUseSASAutoSelectionFeature called");
    return NULL;
}

void* _AFPreferencesValueForKey(void)
{
    if (verbose) puts("STUB: _AFPreferencesValueForKey called");
    return NULL;
}

void* _AFPreferencesVibrateWithAlert(void)
{
    if (verbose) puts("STUB: _AFPreferencesVibrateWithAlert called");
    return NULL;
}

void* _AFPreferencesVibrationCount(void)
{
    if (verbose) puts("STUB: _AFPreferencesVibrationCount called");
    return NULL;
}

void* _AFPreferencesVibrationDuration(void)
{
    if (verbose) puts("STUB: _AFPreferencesVibrationDuration called");
    return NULL;
}

void* _AFPreferencesVibrationGap(void)
{
    if (verbose) puts("STUB: _AFPreferencesVibrationGap called");
    return NULL;
}

void* _AFPreferencesVibrationIntensity(void)
{
    if (verbose) puts("STUB: _AFPreferencesVibrationIntensity called");
    return NULL;
}

void* _AFPreferencesWebInspectorEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesWebInspectorEnabled called");
    return NULL;
}

void* _AFPreferencesiRATManagerRecommendationEnabled(void)
{
    if (verbose) puts("STUB: _AFPreferencesiRATManagerRecommendationEnabled called");
    return NULL;
}

void* _AFRedactedCopyPerson(void)
{
    if (verbose) puts("STUB: _AFRedactedCopyPerson called");
    return NULL;
}

void* _AFRedactedFamilyNamesCopyPerson(void)
{
    if (verbose) puts("STUB: _AFRedactedFamilyNamesCopyPerson called");
    return NULL;
}

void* _AFServiceLog(void)
{
    if (verbose) puts("STUB: _AFServiceLog called");
    return NULL;
}

void* _AFSupportPreferencesSetValueForKey(void)
{
    if (verbose) puts("STUB: _AFSupportPreferencesSetValueForKey called");
    return NULL;
}

void* _AFSupportPreferencesValueForKey(void)
{
    if (verbose) puts("STUB: _AFSupportPreferencesValueForKey called");
    return NULL;
}

void* _ExternalRequestConnection(void)
{
    if (verbose) puts("STUB: _ExternalRequestConnection called");
    return NULL;
}

void* carplayTriggerSeenCallback(void)
{
    if (verbose) puts("STUB: carplayTriggerSeenCallback called");
    return NULL;
}

void* defaultsCallback(void)
{
    if (verbose) puts("STUB: defaultsCallback called");
    return NULL;
}

void* emergencyCallInitiatedCallback(void)
{
    if (verbose) puts("STUB: emergencyCallInitiatedCallback called");
    return NULL;
}

void* emergencyCallPunchoutCallback(void)
{
    if (verbose) puts("STUB: emergencyCallPunchoutCallback called");
    return NULL;
}

void* emergencyCallback(void)
{
    if (verbose) puts("STUB: emergencyCallback called");
    return NULL;
}

void* emergencyHandledCallback(void)
{
    if (verbose) puts("STUB: emergencyHandledCallback called");
    return NULL;
}

void* hammingDistance(void)
{
    if (verbose) puts("STUB: hammingDistance called");
    return NULL;
}

void* inEarTriggerSeenCallback(void)
{
    if (verbose) puts("STUB: inEarTriggerSeenCallback called");
    return NULL;
}

void* myriadDecisionRequestCallback(void)
{
    if (verbose) puts("STUB: myriadDecisionRequestCallback called");
    return NULL;
}

void* notificationCallback(void)
{
    if (verbose) puts("STUB: notificationCallback called");
    return NULL;
}

void* outputTriggerSeenCallback(void)
{
    if (verbose) puts("STUB: outputTriggerSeenCallback called");
    return NULL;
}

void* safelyGetAudioData(void)
{
    if (verbose) puts("STUB: safelyGetAudioData called");
    return NULL;
}
