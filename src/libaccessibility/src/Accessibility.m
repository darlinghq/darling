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


#include <Accessibility/Accessibility.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AXBeginListeningToUserDefaultsChanges(void)
{
    if (verbose) puts("STUB: AXBeginListeningToUserDefaultsChanges called");
    return NULL;
}

void* AXCPCopySharedResourcesPreferencesDomainForDomain(void)
{
    if (verbose) puts("STUB: AXCPCopySharedResourcesPreferencesDomainForDomain called");
    return NULL;
}

void* AXCPSharedResourcesDirectory(void)
{
    if (verbose) puts("STUB: AXCPSharedResourcesDirectory called");
    return NULL;
}

void* AXDisallowsUIBasedAccessibilityFeatures(void)
{
    if (verbose) puts("STUB: AXDisallowsUIBasedAccessibilityFeatures called");
    return NULL;
}

void* AXLocalizationForLocale(void)
{
    if (verbose) puts("STUB: AXLocalizationForLocale called");
    return NULL;
}

void* AXLocalizedStringForLocale(void)
{
    if (verbose) puts("STUB: AXLocalizedStringForLocale called");
    return NULL;
}

void* AXNSLocalizedStringForLocale(void)
{
    if (verbose) puts("STUB: AXNSLocalizedStringForLocale called");
    return NULL;
}

void* AXRetrieveSupportedAccessibilityLanguages(void)
{
    if (verbose) puts("STUB: AXRetrieveSupportedAccessibilityLanguages called");
    return NULL;
}

void* AXSAccessibilityUtilitiesPath(void)
{
    if (verbose) puts("STUB: AXSAccessibilityUtilitiesPath called");
    return NULL;
}

void* _AXDarkenSystemColors(void)
{
    if (verbose) puts("STUB: _AXDarkenSystemColors called");
    return NULL;
}

void* _AXSAXInspectorEnabled(void)
{
    if (verbose) puts("STUB: _AXSAXInspectorEnabled called");
    return NULL;
}

void* _AXSAXInspectorSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSAXInspectorSetEnabled called");
    return NULL;
}

void* _AXSAccessibilityBundlesPath(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityBundlesPath called");
    return NULL;
}

void* _AXSAccessibilityCopyiTunesPreference(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityCopyiTunesPreference called");
    return NULL;
}

void* _AXSAccessibilityEnabled(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityEnabled called");
    return NULL;
}

void* _AXSAccessibilityEnablers(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityEnablers called");
    return NULL;
}

void* _AXSAccessibilityGetBooleaniTunesPreference(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityGetBooleaniTunesPreference called");
    return NULL;
}

void* _AXSAccessibilityHasNoticedOpaqueTouchDevice(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityHasNoticedOpaqueTouchDevice called");
    return NULL;
}

void* _AXSAccessibilityPreferenceDomain(void)
{
    if (verbose) puts("STUB: _AXSAccessibilityPreferenceDomain called");
    return NULL;
}

void* _AXSAccessibilitySetiTunesPreference(void)
{
    if (verbose) puts("STUB: _AXSAccessibilitySetiTunesPreference called");
    return NULL;
}

void* _AXSAllowOpaqueTouchGestures(void)
{
    if (verbose) puts("STUB: _AXSAllowOpaqueTouchGestures called");
    return NULL;
}

void* _AXSAppleTVRemoteUsesSimpleGestures(void)
{
    if (verbose) puts("STUB: _AXSAppleTVRemoteUsesSimpleGestures called");
    return NULL;
}

void* _AXSApplicationAccessibilityEnabled(void)
{
    if (verbose) puts("STUB: _AXSApplicationAccessibilityEnabled called");
    return NULL;
}

void* _AXSApplicationAccessibilitySetEnabled(void)
{
    if (verbose) puts("STUB: _AXSApplicationAccessibilitySetEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchHardwareEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchHardwareEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchRepairIncarnationModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchRepairIncarnationModeEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchScannerEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchScannerEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchScannerSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchScannerSetEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchSetEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchSetHardwareEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchSetHardwareEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchSetRepairIncarnationModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchSetRepairIncarnationModeEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchSetUIEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchSetUIEnabled called");
    return NULL;
}

void* _AXSAssistiveTouchUIEnabled(void)
{
    if (verbose) puts("STUB: _AXSAssistiveTouchUIEnabled called");
    return NULL;
}

void* _AXSAttentionAwarenessFeaturesEnabled(void)
{
    if (verbose) puts("STUB: _AXSAttentionAwarenessFeaturesEnabled called");
    return NULL;
}

void* _AXSAuditInspectionModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSAuditInspectionModeEnabled called");
    return NULL;
}

void* _AXSAutoBrightnessEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutoBrightnessEnabled called");
    return NULL;
}

void* _AXSAutomationEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationEnabled called");
    return NULL;
}

void* _AXSAutomationFauxCollectionViewCellsEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationFauxCollectionViewCellsEnabled called");
    return NULL;
}

void* _AXSAutomationFauxTableViewCellsEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationFauxTableViewCellsEnabled called");
    return NULL;
}

void* _AXSAutomationHitpointWarpingEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationHitpointWarpingEnabled called");
    return NULL;
}

void* _AXSAutomationLocalizedStringLookupInfoEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationLocalizedStringLookupInfoEnabled called");
    return NULL;
}

void* _AXSAutomationLocalizedStringLookupInfoSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationLocalizedStringLookupInfoSetEnabled called");
    return NULL;
}

void* _AXSAutomationPreferredLocalization(void)
{
    if (verbose) puts("STUB: _AXSAutomationPreferredLocalization called");
    return NULL;
}

void* _AXSAutomationSetFauxCollectionViewCellsEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationSetFauxCollectionViewCellsEnabled called");
    return NULL;
}

void* _AXSAutomationSetFauxTableViewCellsEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationSetFauxTableViewCellsEnabled called");
    return NULL;
}

void* _AXSAutomationSetHitpointWarpingEnabled(void)
{
    if (verbose) puts("STUB: _AXSAutomationSetHitpointWarpingEnabled called");
    return NULL;
}

void* _AXSBlueYellowFilterEnabled(void)
{
    if (verbose) puts("STUB: _AXSBlueYellowFilterEnabled called");
    return NULL;
}

void* _AXSBlueYellowFilterSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSBlueYellowFilterSetEnabled called");
    return NULL;
}

void* _AXSButtonShapesEnabled(void)
{
    if (verbose) puts("STUB: _AXSButtonShapesEnabled called");
    return NULL;
}

void* _AXSCachedValueForBrightnessFilter(void)
{
    if (verbose) puts("STUB: _AXSCachedValueForBrightnessFilter called");
    return NULL;
}

void* _AXSCanDisableApplicationAccessibility(void)
{
    if (verbose) puts("STUB: _AXSCanDisableApplicationAccessibility called");
    return NULL;
}

void* _AXSCharacterVoiceDefaultSupportedLanguages(void)
{
    if (verbose) puts("STUB: _AXSCharacterVoiceDefaultSupportedLanguages called");
    return NULL;
}

void* _AXSCharacterVoiceSupportedForLocale(void)
{
    if (verbose) puts("STUB: _AXSCharacterVoiceSupportedForLocale called");
    return NULL;
}

void* _AXSCharacterVoiceSupportedForLocaleIdentifier(void)
{
    if (verbose) puts("STUB: _AXSCharacterVoiceSupportedForLocaleIdentifier called");
    return NULL;
}

void* _AXSCheckForNonStandardApplicationAccessibilityEnabler(void)
{
    if (verbose) puts("STUB: _AXSCheckForNonStandardApplicationAccessibilityEnabler called");
    return NULL;
}

void* _AXSClassicInvertColorsEnabled(void)
{
    if (verbose) puts("STUB: _AXSClassicInvertColorsEnabled called");
    return NULL;
}

void* _AXSClassicInvertColorsSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSClassicInvertColorsSetEnabled called");
    return NULL;
}

void* _AXSClipTracerAccessibilityModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSClipTracerAccessibilityModeEnabled called");
    return NULL;
}

void* _AXSClosedCaptionsEnabled(void)
{
    if (verbose) puts("STUB: _AXSClosedCaptionsEnabled called");
    return NULL;
}

void* _AXSClosedCaptionsSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSClosedCaptionsSetEnabled called");
    return NULL;
}

void* _AXSColorTintFilterEnabled(void)
{
    if (verbose) puts("STUB: _AXSColorTintFilterEnabled called");
    return NULL;
}

void* _AXSColorTintSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSColorTintSetEnabled called");
    return NULL;
}

void* _AXSCommandAndControlEnabled(void)
{
    if (verbose) puts("STUB: _AXSCommandAndControlEnabled called");
    return NULL;
}

void* _AXSCommandAndControlSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSCommandAndControlSetEnabled called");
    return NULL;
}

void* _AXSContextKitAppIsWhitelisted(void)
{
    if (verbose) puts("STUB: _AXSContextKitAppIsWhitelisted called");
    return NULL;
}

void* _AXSContextKitContextIsLoneApplicationAccessibilityEnabler(void)
{
    if (verbose) puts("STUB: _AXSContextKitContextIsLoneApplicationAccessibilityEnabler called");
    return NULL;
}

void* _AXSContextKitContextIsLoneEnabler(void)
{
    if (verbose) puts("STUB: _AXSContextKitContextIsLoneEnabler called");
    return NULL;
}

void* _AXSCopyPreferenceValue(void)
{
    if (verbose) puts("STUB: _AXSCopyPreferenceValue called");
    return NULL;
}

void* _AXSCopyPreferredContentSizeCategoryName(void)
{
    if (verbose) puts("STUB: _AXSCopyPreferredContentSizeCategoryName called");
    return NULL;
}

void* _AXSCopySettingsDataBlobForBuddy(void)
{
    if (verbose) puts("STUB: _AXSCopySettingsDataBlobForBuddy called");
    return NULL;
}

void* _AXSCrashOnValidationErrors(void)
{
    if (verbose) puts("STUB: _AXSCrashOnValidationErrors called");
    return NULL;
}

void* _AXSCurrentAccessibilitySettingsCanSupportLoginUI(void)
{
    if (verbose) puts("STUB: _AXSCurrentAccessibilitySettingsCanSupportLoginUI called");
    return NULL;
}

void* _AXSDefaultRouteForCall(void)
{
    if (verbose) puts("STUB: _AXSDefaultRouteForCall called");
    return NULL;
}

void* _AXSDidInitializeNotificationListeners(void)
{
    if (verbose) puts("STUB: _AXSDidInitializeNotificationListeners called");
    return NULL;
}

void* _AXSDifferentiateWithoutColorEnabled(void)
{
    if (verbose) puts("STUB: _AXSDifferentiateWithoutColorEnabled called");
    return NULL;
}

void* _AXSDisableDomainSynching(void)
{
    if (verbose) puts("STUB: _AXSDisableDomainSynching called");
    return NULL;
}

void* _AXSDisableScreenFilters(void)
{
    if (verbose) puts("STUB: _AXSDisableScreenFilters called");
    return NULL;
}

void* _AXSEarpieceNoiseCancellationEnabled(void)
{
    if (verbose) puts("STUB: _AXSEarpieceNoiseCancellationEnabled called");
    return NULL;
}

void* _AXSEarpieceNoiseCancellationSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSEarpieceNoiseCancellationSetEnabled called");
    return NULL;
}

void* _AXSEnhanceBackgroundContrastEnabled(void)
{
    if (verbose) puts("STUB: _AXSEnhanceBackgroundContrastEnabled called");
    return NULL;
}

void* _AXSEnhanceTextLegibilityEnabled(void)
{
    if (verbose) puts("STUB: _AXSEnhanceTextLegibilityEnabled called");
    return NULL;
}

void* _AXSEnhanceTextTrackingEnabled(void)
{
    if (verbose) puts("STUB: _AXSEnhanceTextTrackingEnabled called");
    return NULL;
}

void* _AXSExtendedKeyboardPredictionsEnabled(void)
{
    if (verbose) puts("STUB: _AXSExtendedKeyboardPredictionsEnabled called");
    return NULL;
}

void* _AXSForcePreferenceUpdate(void)
{
    if (verbose) puts("STUB: _AXSForcePreferenceUpdate called");
    return NULL;
}

void* _AXSForceTouchEnabled(void)
{
    if (verbose) puts("STUB: _AXSForceTouchEnabled called");
    return NULL;
}

void* _AXSForceTouchSensitivity(void)
{
    if (verbose) puts("STUB: _AXSForceTouchSensitivity called");
    return NULL;
}

void* _AXSForceTouchTimeDuration(void)
{
    if (verbose) puts("STUB: _AXSForceTouchTimeDuration called");
    return NULL;
}

void* _AXSFullKeyboardAccessEnabled(void)
{
    if (verbose) puts("STUB: _AXSFullKeyboardAccessEnabled called");
    return NULL;
}

void* _AXSFullKeyboardAccessFocusRingEnabled(void)
{
    if (verbose) puts("STUB: _AXSFullKeyboardAccessFocusRingEnabled called");
    return NULL;
}

void* _AXSFullKeyboardAccessPassthroughModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSFullKeyboardAccessPassthroughModeEnabled called");
    return NULL;
}

void* _AXSFullKeyboardAccessSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSFullKeyboardAccessSetEnabled called");
    return NULL;
}

void* _AXSFullKeyboardAccessSetFocusRingEnabled(void)
{
    if (verbose) puts("STUB: _AXSFullKeyboardAccessSetFocusRingEnabled called");
    return NULL;
}

void* _AXSFullKeyboardAccessSetPassthroughModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSFullKeyboardAccessSetPassthroughModeEnabled called");
    return NULL;
}

void* _AXSGetUSBRMDisablers(void)
{
    if (verbose) puts("STUB: _AXSGetUSBRMDisablers called");
    return NULL;
}

void* _AXSGrayscaleEnabled(void)
{
    if (verbose) puts("STUB: _AXSGrayscaleEnabled called");
    return NULL;
}

void* _AXSGrayscaleSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSGrayscaleSetEnabled called");
    return NULL;
}

void* _AXSGreenRedFilterEnabled(void)
{
    if (verbose) puts("STUB: _AXSGreenRedFilterEnabled called");
    return NULL;
}

void* _AXSGreenRedFilterSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSGreenRedFilterSetEnabled called");
    return NULL;
}

void* _AXSGuidedAccessEnabled(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessEnabled called");
    return NULL;
}

void* _AXSGuidedAccessEnabledByManagedConfiguration(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessEnabledByManagedConfiguration called");
    return NULL;
}

void* _AXSGuidedAccessHasPasscode(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessHasPasscode called");
    return NULL;
}

void* _AXSGuidedAccessRequiresApplicationAccessibility(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessRequiresApplicationAccessibility called");
    return NULL;
}

void* _AXSGuidedAccessSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessSetEnabled called");
    return NULL;
}

void* _AXSGuidedAccessSetEnabledByManagedConfiguration(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessSetEnabledByManagedConfiguration called");
    return NULL;
}

void* _AXSGuidedAccessSetHasPasscode(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessSetHasPasscode called");
    return NULL;
}

void* _AXSGuidedAccessStartSession(void)
{
    if (verbose) puts("STUB: _AXSGuidedAccessStartSession called");
    return NULL;
}

void* _AXSHandleHomeButtonPressSwallowedForGuidedAccess(void)
{
    if (verbose) puts("STUB: _AXSHandleHomeButtonPressSwallowedForGuidedAccess called");
    return NULL;
}

void* _AXSHandleTripleClickHomeButtonPress(void)
{
    if (verbose) puts("STUB: _AXSHandleTripleClickHomeButtonPress called");
    return NULL;
}

void* _AXSHearingAidComplianceEnabled(void)
{
    if (verbose) puts("STUB: _AXSHearingAidComplianceEnabled called");
    return NULL;
}

void* _AXSHearingAidComplianceSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSHearingAidComplianceSetEnabled called");
    return NULL;
}

void* _AXSHearingAidRingtoneStreamingEnabled(void)
{
    if (verbose) puts("STUB: _AXSHearingAidRingtoneStreamingEnabled called");
    return NULL;
}

void* _AXSHearingAidRingtoneStreamingSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSHearingAidRingtoneStreamingSetEnabled called");
    return NULL;
}

void* _AXSHearingAidsPaired(void)
{
    if (verbose) puts("STUB: _AXSHearingAidsPaired called");
    return NULL;
}

void* _AXSHearingAidsSetPaired(void)
{
    if (verbose) puts("STUB: _AXSHearingAidsSetPaired called");
    return NULL;
}

void* _AXSHearingDevicePairedEars(void)
{
    if (verbose) puts("STUB: _AXSHearingDevicePairedEars called");
    return NULL;
}

void* _AXSHearingSetDevicePairedEars(void)
{
    if (verbose) puts("STUB: _AXSHearingSetDevicePairedEars called");
    return NULL;
}

void* _AXSHighContrastFocusIndicatorsEnabled(void)
{
    if (verbose) puts("STUB: _AXSHighContrastFocusIndicatorsEnabled called");
    return NULL;
}

void* _AXSHomeButtonAssistant(void)
{
    if (verbose) puts("STUB: _AXSHomeButtonAssistant called");
    return NULL;
}

void* _AXSHomeButtonRestingUnlock(void)
{
    if (verbose) puts("STUB: _AXSHomeButtonRestingUnlock called");
    return NULL;
}

void* _AXSHomeButtonRestingUnlockEverSet(void)
{
    if (verbose) puts("STUB: _AXSHomeButtonRestingUnlockEverSet called");
    return NULL;
}

void* _AXSHomeButtonSetRestingUnlock(void)
{
    if (verbose) puts("STUB: _AXSHomeButtonSetRestingUnlock called");
    return NULL;
}

void* _AXSHomeClickEnabled(void)
{
    if (verbose) puts("STUB: _AXSHomeClickEnabled called");
    return NULL;
}

void* _AXSHomeClickSetSpeed(void)
{
    if (verbose) puts("STUB: _AXSHomeClickSetSpeed called");
    return NULL;
}

void* _AXSHomeClickSpeed(void)
{
    if (verbose) puts("STUB: _AXSHomeClickSpeed called");
    return NULL;
}

void* _AXSInDataMigrationMode(void)
{
    if (verbose) puts("STUB: _AXSInDataMigrationMode called");
    return NULL;
}

void* _AXSInPerformanceTestMode(void)
{
    if (verbose) puts("STUB: _AXSInPerformanceTestMode called");
    return NULL;
}

void* _AXSInUnitTestMode(void)
{
    if (verbose) puts("STUB: _AXSInUnitTestMode called");
    return NULL;
}

void* _AXSIncreaseButtonLegibility(void)
{
    if (verbose) puts("STUB: _AXSIncreaseButtonLegibility called");
    return NULL;
}

void* _AXSInvertColorsDisplaySetEnabled(void)
{
    if (verbose) puts("STUB: _AXSInvertColorsDisplaySetEnabled called");
    return NULL;
}

void* _AXSInvertColorsEnabled(void)
{
    if (verbose) puts("STUB: _AXSInvertColorsEnabled called");
    return NULL;
}

void* _AXSInvertColorsMarkInvertColorsPreloadComplete(void)
{
    if (verbose) puts("STUB: _AXSInvertColorsMarkInvertColorsPreloadComplete called");
    return NULL;
}

void* _AXSInvertColorsReloadPreferenceImmediately(void)
{
    if (verbose) puts("STUB: _AXSInvertColorsReloadPreferenceImmediately called");
    return NULL;
}

void* _AXSInvertColorsSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSInvertColorsSetEnabled called");
    return NULL;
}

void* _AXSKeyRepeatDelay(void)
{
    if (verbose) puts("STUB: _AXSKeyRepeatDelay called");
    return NULL;
}

void* _AXSKeyRepeatEnabled(void)
{
    if (verbose) puts("STUB: _AXSKeyRepeatEnabled called");
    return NULL;
}

void* _AXSKeyRepeatInterval(void)
{
    if (verbose) puts("STUB: _AXSKeyRepeatInterval called");
    return NULL;
}

void* _AXSLargeTextUsesExtendedRange(void)
{
    if (verbose) puts("STUB: _AXSLargeTextUsesExtendedRange called");
    return NULL;
}

void* _AXSLeftRightAudioBalance(void)
{
    if (verbose) puts("STUB: _AXSLeftRightAudioBalance called");
    return NULL;
}

void* _AXSLetterFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSLetterFeedbackEnabled called");
    return NULL;
}

void* _AXSLocalizationCaptionMode(void)
{
    if (verbose) puts("STUB: _AXSLocalizationCaptionMode called");
    return NULL;
}

void* _AXSLockClickDownPressDurationScaleFactor(void)
{
    if (verbose) puts("STUB: _AXSLockClickDownPressDurationScaleFactor called");
    return NULL;
}

void* _AXSLockClickVarianceScaleFactor(void)
{
    if (verbose) puts("STUB: _AXSLockClickVarianceScaleFactor called");
    return NULL;
}

void* _AXSLogValidationErrors(void)
{
    if (verbose) puts("STUB: _AXSLogValidationErrors called");
    return NULL;
}

void* _AXSLowercaseKeyboardDisplayEnabled(void)
{
    if (verbose) puts("STUB: _AXSLowercaseKeyboardDisplayEnabled called");
    return NULL;
}

void* _AXSMonoAudioEnabled(void)
{
    if (verbose) puts("STUB: _AXSMonoAudioEnabled called");
    return NULL;
}

void* _AXSMonoAudioSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSMonoAudioSetEnabled called");
    return NULL;
}

void* _AXSNamedSingleSystemColor(void)
{
    if (verbose) puts("STUB: _AXSNamedSingleSystemColor called");
    return NULL;
}

void* _AXSOpaqueTouchTapSpeed(void)
{
    if (verbose) puts("STUB: _AXSOpaqueTouchTapSpeed called");
    return NULL;
}

void* _AXSPearlAuthenticationHapticsEnabled(void)
{
    if (verbose) puts("STUB: _AXSPearlAuthenticationHapticsEnabled called");
    return NULL;
}

void* _AXSPearlRestingUnlock(void)
{
    if (verbose) puts("STUB: _AXSPearlRestingUnlock called");
    return NULL;
}

void* _AXSPearlRestingUnlockEverSet(void)
{
    if (verbose) puts("STUB: _AXSPearlRestingUnlockEverSet called");
    return NULL;
}

void* _AXSPearlSetRestingUnlock(void)
{
    if (verbose) puts("STUB: _AXSPearlSetRestingUnlock called");
    return NULL;
}

void* _AXSPhoneticFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSPhoneticFeedbackEnabled called");
    return NULL;
}

void* _AXSPreferredFontSize(void)
{
    if (verbose) puts("STUB: _AXSPreferredFontSize called");
    return NULL;
}

void* _AXSPreferredFontSizeSetSize(void)
{
    if (verbose) puts("STUB: _AXSPreferredFontSizeSetSize called");
    return NULL;
}

void* _AXSProcessDrawsAssistiveUI(void)
{
    if (verbose) puts("STUB: _AXSProcessDrawsAssistiveUI called");
    return NULL;
}

void* _AXSQuickSpeakCopyPreferredLocalesForLanguages(void)
{
    if (verbose) puts("STUB: _AXSQuickSpeakCopyPreferredLocalesForLanguages called");
    return NULL;
}

void* _AXSQuickSpeakEnabled(void)
{
    if (verbose) puts("STUB: _AXSQuickSpeakEnabled called");
    return NULL;
}

void* _AXSQuickSpeakSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSQuickSpeakSetEnabled called");
    return NULL;
}

void* _AXSQuickSpeakSetPreferredLocaleForLanguage(void)
{
    if (verbose) puts("STUB: _AXSQuickSpeakSetPreferredLocaleForLanguage called");
    return NULL;
}

void* _AXSQuickTypePredictionFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSQuickTypePredictionFeedbackEnabled called");
    return NULL;
}

void* _AXSRedGreenFilterEnabled(void)
{
    if (verbose) puts("STUB: _AXSRedGreenFilterEnabled called");
    return NULL;
}

void* _AXSRedGreenFilterSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSRedGreenFilterSetEnabled called");
    return NULL;
}

void* _AXSReduceMotionAutoplayMessagesEffectsEnabled(void)
{
    if (verbose) puts("STUB: _AXSReduceMotionAutoplayMessagesEffectsEnabled called");
    return NULL;
}

void* _AXSReduceMotionAutoplayVideoPreviewsEnabled(void)
{
    if (verbose) puts("STUB: _AXSReduceMotionAutoplayVideoPreviewsEnabled called");
    return NULL;
}

void* _AXSReduceMotionEnabled(void)
{
    if (verbose) puts("STUB: _AXSReduceMotionEnabled called");
    return NULL;
}

void* _AXSReduceMotionReduceSlideTransitionsEnabled(void)
{
    if (verbose) puts("STUB: _AXSReduceMotionReduceSlideTransitionsEnabled called");
    return NULL;
}

void* _AXSReduceWhitePointEnabled(void)
{
    if (verbose) puts("STUB: _AXSReduceWhitePointEnabled called");
    return NULL;
}

void* _AXSReportValidationErrors(void)
{
    if (verbose) puts("STUB: _AXSReportValidationErrors called");
    return NULL;
}

void* _AXSResetAccessibilityFeatures(void)
{
    if (verbose) puts("STUB: _AXSResetAccessibilityFeatures called");
    return NULL;
}

void* _AXSRestoreSettingsFromDataBlobForBuddy(void)
{
    if (verbose) puts("STUB: _AXSRestoreSettingsFromDataBlobForBuddy called");
    return NULL;
}

void* _AXSScreenContrast(void)
{
    if (verbose) puts("STUB: _AXSScreenContrast called");
    return NULL;
}

void* _AXSScreenFilterApplied(void)
{
    if (verbose) puts("STUB: _AXSScreenFilterApplied called");
    return NULL;
}

void* _AXSScreenFilterSetShowInitialAlert(void)
{
    if (verbose) puts("STUB: _AXSScreenFilterSetShowInitialAlert called");
    return NULL;
}

void* _AXSScreenFilterShowInitialAlert(void)
{
    if (verbose) puts("STUB: _AXSScreenFilterShowInitialAlert called");
    return NULL;
}

void* _AXSSetAccessibilityEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetAccessibilityEnabled called");
    return NULL;
}

void* _AXSSetAccessibilityHasNoticedOpaqueTouchDevice(void)
{
    if (verbose) puts("STUB: _AXSSetAccessibilityHasNoticedOpaqueTouchDevice called");
    return NULL;
}

void* _AXSSetAccessibilityNeedsCameraOnLockScreen(void)
{
    if (verbose) puts("STUB: _AXSSetAccessibilityNeedsCameraOnLockScreen called");
    return NULL;
}

void* _AXSSetAllowOpaqueTouchGestures(void)
{
    if (verbose) puts("STUB: _AXSSetAllowOpaqueTouchGestures called");
    return NULL;
}

void* _AXSSetAppleTVRemoteUsesSimpleGestures(void)
{
    if (verbose) puts("STUB: _AXSSetAppleTVRemoteUsesSimpleGestures called");
    return NULL;
}

void* _AXSSetAttentionAwarenessFeaturesEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetAttentionAwarenessFeaturesEnabled called");
    return NULL;
}

void* _AXSSetAuditInspectionModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetAuditInspectionModeEnabled called");
    return NULL;
}

void* _AXSSetAutomationEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetAutomationEnabled called");
    return NULL;
}

void* _AXSSetButtonShapesEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetButtonShapesEnabled called");
    return NULL;
}

void* _AXSSetCacheForBrightnessFilter(void)
{
    if (verbose) puts("STUB: _AXSSetCacheForBrightnessFilter called");
    return NULL;
}

void* _AXSSetClipTracerAccessibilityModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetClipTracerAccessibilityModeEnabled called");
    return NULL;
}

void* _AXSSetCrashOnValidationErrors(void)
{
    if (verbose) puts("STUB: _AXSSetCrashOnValidationErrors called");
    return NULL;
}

void* _AXSSetDarkenSystemColors(void)
{
    if (verbose) puts("STUB: _AXSSetDarkenSystemColors called");
    return NULL;
}

void* _AXSSetDefaultRouteForCall(void)
{
    if (verbose) puts("STUB: _AXSSetDefaultRouteForCall called");
    return NULL;
}

void* _AXSSetDifferentiateWithoutColorEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetDifferentiateWithoutColorEnabled called");
    return NULL;
}

void* _AXSSetEnhanceBackgroundContrastEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetEnhanceBackgroundContrastEnabled called");
    return NULL;
}

void* _AXSSetEnhanceTextLegibilityEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetEnhanceTextLegibilityEnabled called");
    return NULL;
}

void* _AXSSetEnhanceTextTrackingEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetEnhanceTextTrackingEnabled called");
    return NULL;
}

void* _AXSSetForceTouchEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetForceTouchEnabled called");
    return NULL;
}

void* _AXSSetForceTouchSensitivity(void)
{
    if (verbose) puts("STUB: _AXSSetForceTouchSensitivity called");
    return NULL;
}

void* _AXSSetForceTouchTimeDuration(void)
{
    if (verbose) puts("STUB: _AXSSetForceTouchTimeDuration called");
    return NULL;
}

void* _AXSSetHighContrastFocusIndicatorsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetHighContrastFocusIndicatorsEnabled called");
    return NULL;
}

void* _AXSSetHomeButtonAssistant(void)
{
    if (verbose) puts("STUB: _AXSSetHomeButtonAssistant called");
    return NULL;
}

void* _AXSSetInDataMigrationMode(void)
{
    if (verbose) puts("STUB: _AXSSetInDataMigrationMode called");
    return NULL;
}

void* _AXSSetInPerformanceTestMode(void)
{
    if (verbose) puts("STUB: _AXSSetInPerformanceTestMode called");
    return NULL;
}

void* _AXSSetInUnitTestMode(void)
{
    if (verbose) puts("STUB: _AXSSetInUnitTestMode called");
    return NULL;
}

void* _AXSSetIncreaseButtonLegibility(void)
{
    if (verbose) puts("STUB: _AXSSetIncreaseButtonLegibility called");
    return NULL;
}

void* _AXSSetKeyRepeatDelay(void)
{
    if (verbose) puts("STUB: _AXSSetKeyRepeatDelay called");
    return NULL;
}

void* _AXSSetKeyRepeatEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetKeyRepeatEnabled called");
    return NULL;
}

void* _AXSSetKeyRepeatInterval(void)
{
    if (verbose) puts("STUB: _AXSSetKeyRepeatInterval called");
    return NULL;
}

void* _AXSSetLargeTextUsesExtendedRange(void)
{
    if (verbose) puts("STUB: _AXSSetLargeTextUsesExtendedRange called");
    return NULL;
}

void* _AXSSetLeftRightAudioBalance(void)
{
    if (verbose) puts("STUB: _AXSSetLeftRightAudioBalance called");
    return NULL;
}

void* _AXSSetLetterFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetLetterFeedbackEnabled called");
    return NULL;
}

void* _AXSSetLocalizationCaptionModeEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetLocalizationCaptionModeEnabled called");
    return NULL;
}

void* _AXSSetLogValidationErrors(void)
{
    if (verbose) puts("STUB: _AXSSetLogValidationErrors called");
    return NULL;
}

void* _AXSSetLowercaseKeyboardDisplayEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetLowercaseKeyboardDisplayEnabled called");
    return NULL;
}

void* _AXSSetNamedSingleSystemColor(void)
{
    if (verbose) puts("STUB: _AXSSetNamedSingleSystemColor called");
    return NULL;
}

void* _AXSSetOpaqueTouchTapSpeed(void)
{
    if (verbose) puts("STUB: _AXSSetOpaqueTouchTapSpeed called");
    return NULL;
}

void* _AXSSetPearlAuthenticationHapticsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetPearlAuthenticationHapticsEnabled called");
    return NULL;
}

void* _AXSSetPhoneticFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetPhoneticFeedbackEnabled called");
    return NULL;
}

void* _AXSSetPreferredContentSizeCategoryName(void)
{
    if (verbose) puts("STUB: _AXSSetPreferredContentSizeCategoryName called");
    return NULL;
}

void* _AXSSetQuickTypePredictionFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetQuickTypePredictionFeedbackEnabled called");
    return NULL;
}

void* _AXSSetReduceMotionAutoplayMessagesEffectsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetReduceMotionAutoplayMessagesEffectsEnabled called");
    return NULL;
}

void* _AXSSetReduceMotionAutoplayVideoPreviewsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetReduceMotionAutoplayVideoPreviewsEnabled called");
    return NULL;
}

void* _AXSSetReduceMotionEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetReduceMotionEnabled called");
    return NULL;
}

void* _AXSSetReduceMotionReduceSlideTransitionsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetReduceMotionReduceSlideTransitionsEnabled called");
    return NULL;
}

void* _AXSSetReduceWhitePointEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetReduceWhitePointEnabled called");
    return NULL;
}

void* _AXSSetReportValidationErrors(void)
{
    if (verbose) puts("STUB: _AXSSetReportValidationErrors called");
    return NULL;
}

void* _AXSSetScreenContrast(void)
{
    if (verbose) puts("STUB: _AXSSetScreenContrast called");
    return NULL;
}

void* _AXSSetShakeToUndoDisabled(void)
{
    if (verbose) puts("STUB: _AXSSetShakeToUndoDisabled called");
    return NULL;
}

void* _AXSSetShouldUseSpringBoardForHomeButtonDetection(void)
{
    if (verbose) puts("STUB: _AXSSetShouldUseSpringBoardForHomeButtonDetection called");
    return NULL;
}

void* _AXSSetSideButtonClickSpeed(void)
{
    if (verbose) puts("STUB: _AXSSetSideButtonClickSpeed called");
    return NULL;
}

void* _AXSSetSiriSemanticContextEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetSiriSemanticContextEnabled called");
    return NULL;
}

void* _AXSSetSlowKeysAcceptanceDelay(void)
{
    if (verbose) puts("STUB: _AXSSetSlowKeysAcceptanceDelay called");
    return NULL;
}

void* _AXSSetSlowKeysEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetSlowKeysEnabled called");
    return NULL;
}

void* _AXSSetSpeakThisEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetSpeakThisEnabled called");
    return NULL;
}

void* _AXSSetTripleClickOptions(void)
{
    if (verbose) puts("STUB: _AXSSetTripleClickOptions called");
    return NULL;
}

void* _AXSSetUSBRMDisablers(void)
{
    if (verbose) puts("STUB: _AXSSetUSBRMDisablers called");
    return NULL;
}

void* _AXSSetUseDarkerKeyboard(void)
{
    if (verbose) puts("STUB: _AXSSetUseDarkerKeyboard called");
    return NULL;
}

void* _AXSSetUseSingleSystemColor(void)
{
    if (verbose) puts("STUB: _AXSSetUseSingleSystemColor called");
    return NULL;
}

void* _AXSSetUserEnabledPasscodeForPurchases(void)
{
    if (verbose) puts("STUB: _AXSSetUserEnabledPasscodeForPurchases called");
    return NULL;
}

void* _AXSSetVibrationDisabled(void)
{
    if (verbose) puts("STUB: _AXSSetVibrationDisabled called");
    return NULL;
}

void* _AXSSetWebAccessibilityEventsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetWebAccessibilityEventsEnabled called");
    return NULL;
}

void* _AXSSetWordFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSSetWordFeedbackEnabled called");
    return NULL;
}

void* _AXSSetWriteableClient(void)
{
    if (verbose) puts("STUB: _AXSSetWriteableClient called");
    return NULL;
}

void* _AXSShakeToUndoDisabled(void)
{
    if (verbose) puts("STUB: _AXSShakeToUndoDisabled called");
    return NULL;
}

void* _AXSShouldUseSpringBoardForHomeButtonDetection(void)
{
    if (verbose) puts("STUB: _AXSShouldUseSpringBoardForHomeButtonDetection called");
    return NULL;
}

void* _AXSSideButtonClickSpeed(void)
{
    if (verbose) puts("STUB: _AXSSideButtonClickSpeed called");
    return NULL;
}

void* _AXSSideButtonClickSpeedEnabled(void)
{
    if (verbose) puts("STUB: _AXSSideButtonClickSpeedEnabled called");
    return NULL;
}

void* _AXSSingleSystemColorValues(void)
{
    if (verbose) puts("STUB: _AXSSingleSystemColorValues called");
    return NULL;
}

void* _AXSSiriSemanticContextEnabled(void)
{
    if (verbose) puts("STUB: _AXSSiriSemanticContextEnabled called");
    return NULL;
}

void* _AXSSlowKeysAcceptanceDelay(void)
{
    if (verbose) puts("STUB: _AXSSlowKeysAcceptanceDelay called");
    return NULL;
}

void* _AXSSlowKeysEnabled(void)
{
    if (verbose) puts("STUB: _AXSSlowKeysEnabled called");
    return NULL;
}

void* _AXSSoftwareTTYEnabled(void)
{
    if (verbose) puts("STUB: _AXSSoftwareTTYEnabled called");
    return NULL;
}

void* _AXSSoftwareTTYSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSSoftwareTTYSetEnabled called");
    return NULL;
}

void* _AXSSpeakThisEnabled(void)
{
    if (verbose) puts("STUB: _AXSSpeakThisEnabled called");
    return NULL;
}

void* _AXSSpeechSettingsDisabledByManagedConfiguration(void)
{
    if (verbose) puts("STUB: _AXSSpeechSettingsDisabledByManagedConfiguration called");
    return NULL;
}

void* _AXSSpeechSettingsSetDisabledByManagedConfiguration(void)
{
    if (verbose) puts("STUB: _AXSSpeechSettingsSetDisabledByManagedConfiguration called");
    return NULL;
}

void* _AXSSwitchControlEnabledAndNeedsCameraOnLockScreen(void)
{
    if (verbose) puts("STUB: _AXSSwitchControlEnabledAndNeedsCameraOnLockScreen called");
    return NULL;
}

void* _AXSSwitchControlExtendedKeyboardPredictionsEnabled(void)
{
    if (verbose) puts("STUB: _AXSSwitchControlExtendedKeyboardPredictionsEnabled called");
    return NULL;
}

void* _AXSSwitchControlExtendedKeyboardPredictionsSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSSwitchControlExtendedKeyboardPredictionsSetEnabled called");
    return NULL;
}

void* _AXSTripleClickAddOption(void)
{
    if (verbose) puts("STUB: _AXSTripleClickAddOption called");
    return NULL;
}

void* _AXSTripleClickContainsOption(void)
{
    if (verbose) puts("STUB: _AXSTripleClickContainsOption called");
    return NULL;
}

void* _AXSTripleClickCopyOptions(void)
{
    if (verbose) puts("STUB: _AXSTripleClickCopyOptions called");
    return NULL;
}

void* _AXSTripleClickRemoveOption(void)
{
    if (verbose) puts("STUB: _AXSTripleClickRemoveOption called");
    return NULL;
}

void* _AXSUseDarkerKeyboard(void)
{
    if (verbose) puts("STUB: _AXSUseDarkerKeyboard called");
    return NULL;
}

void* _AXSUseSingleSystemColor(void)
{
    if (verbose) puts("STUB: _AXSUseSingleSystemColor called");
    return NULL;
}

void* _AXSUserEnabledPasscodeForPurchases(void)
{
    if (verbose) puts("STUB: _AXSUserEnabledPasscodeForPurchases called");
    return NULL;
}

void* _AXSVibrationDisabled(void)
{
    if (verbose) puts("STUB: _AXSVibrationDisabled called");
    return NULL;
}

void* _AXSVideosPreferenceDomain(void)
{
    if (verbose) puts("STUB: _AXSVideosPreferenceDomain called");
    return NULL;
}

void* _AXSVisualAlertEnabled(void)
{
    if (verbose) puts("STUB: _AXSVisualAlertEnabled called");
    return NULL;
}

void* _AXSVisualAlertSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSVisualAlertSetEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchBrailleContractionMode(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchBrailleContractionMode called");
    return NULL;
}

void* _AXSVoiceOverTouchBrailleDisplayDisconnectOnSleep(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchBrailleDisplayDisconnectOnSleep called");
    return NULL;
}

void* _AXSVoiceOverTouchBrailleEightDotMode(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchBrailleEightDotMode called");
    return NULL;
}

void* _AXSVoiceOverTouchBrailleMasterStatusCellIndex(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchBrailleMasterStatusCellIndex called");
    return NULL;
}

void* _AXSVoiceOverTouchBrailleVirtualStatusAlignment(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchBrailleVirtualStatusAlignment called");
    return NULL;
}

void* _AXSVoiceOverTouchCopyBrailleBluetoothDisplay(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchCopyBrailleBluetoothDisplay called");
    return NULL;
}

void* _AXSVoiceOverTouchCopyBrailleLanguageRotorItems(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchCopyBrailleLanguageRotorItems called");
    return NULL;
}

void* _AXSVoiceOverTouchCopyBrailleTableIdentifier(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchCopyBrailleTableIdentifier called");
    return NULL;
}

void* _AXSVoiceOverTouchCopyLanguageRotorItems(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchCopyLanguageRotorItems called");
    return NULL;
}

void* _AXSVoiceOverTouchEnabled(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchEnabledThroughAccessory(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchEnabledThroughAccessory called");
    return NULL;
}

void* _AXSVoiceOverTouchScreenCurtainEnabled(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchScreenCurtainEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleBluetoothDisplay(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleBluetoothDisplay called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleContractionMode(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleContractionMode called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleDisplayDisconnectOnSleep(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleDisplayDisconnectOnSleep called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleEightDotMode(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleEightDotMode called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleLanguageRotorItems(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleLanguageRotorItems called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleMasterStatusCellIndex(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleMasterStatusCellIndex called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleTableIdentifier(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleTableIdentifier called");
    return NULL;
}

void* _AXSVoiceOverTouchSetBrailleVirtualStatusAlignment(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetBrailleVirtualStatusAlignment called");
    return NULL;
}

void* _AXSVoiceOverTouchSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchSetEnabledAndAutoConfirmUsage(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetEnabledAndAutoConfirmUsage called");
    return NULL;
}

void* _AXSVoiceOverTouchSetEnabledThroughAccessory(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetEnabledThroughAccessory called");
    return NULL;
}

void* _AXSVoiceOverTouchSetLanguageRotorItems(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetLanguageRotorItems called");
    return NULL;
}

void* _AXSVoiceOverTouchSetScreenCurtainEnabled(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetScreenCurtainEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchSetShouldRouteToSpeakerWithProximity(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetShouldRouteToSpeakerWithProximity called");
    return NULL;
}

void* _AXSVoiceOverTouchSetSpeakTimeOnWake(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetSpeakTimeOnWake called");
    return NULL;
}

void* _AXSVoiceOverTouchSetSpeakingRate(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetSpeakingRate called");
    return NULL;
}

void* _AXSVoiceOverTouchSetTypingMode(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetTypingMode called");
    return NULL;
}

void* _AXSVoiceOverTouchSetUIEnabled(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetUIEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchSetUsageConfirmed(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetUsageConfirmed called");
    return NULL;
}

void* _AXSVoiceOverTouchSetUserHasReadNoHomeButtonGestureDescription(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetUserHasReadNoHomeButtonGestureDescription called");
    return NULL;
}

void* _AXSVoiceOverTouchSetVolume(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSetVolume called");
    return NULL;
}

void* _AXSVoiceOverTouchShouldRouteToSpeakerWithProximity(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchShouldRouteToSpeakerWithProximity called");
    return NULL;
}

void* _AXSVoiceOverTouchSpeakTimeOnWake(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSpeakTimeOnWake called");
    return NULL;
}

void* _AXSVoiceOverTouchSpeakingRate(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchSpeakingRate called");
    return NULL;
}

void* _AXSVoiceOverTouchTypingMode(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchTypingMode called");
    return NULL;
}

void* _AXSVoiceOverTouchUIEnabled(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchUIEnabled called");
    return NULL;
}

void* _AXSVoiceOverTouchUsageConfirmed(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchUsageConfirmed called");
    return NULL;
}

void* _AXSVoiceOverTouchUserHasReadNoHomeButtonGestureDescription(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchUserHasReadNoHomeButtonGestureDescription called");
    return NULL;
}

void* _AXSVoiceOverTouchVolume(void)
{
    if (verbose) puts("STUB: _AXSVoiceOverTouchVolume called");
    return NULL;
}

void* _AXSVoiceOvierTouchLanguageRotorItemsExist(void)
{
    if (verbose) puts("STUB: _AXSVoiceOvierTouchLanguageRotorItemsExist called");
    return NULL;
}

void* _AXSWalkieTalkieSetTapToTalkEnabled(void)
{
    if (verbose) puts("STUB: _AXSWalkieTalkieSetTapToTalkEnabled called");
    return NULL;
}

void* _AXSWalkieTalkieTapToTalkEnabled(void)
{
    if (verbose) puts("STUB: _AXSWalkieTalkieTapToTalkEnabled called");
    return NULL;
}

void* _AXSWebAccessibilityEventsEnabled(void)
{
    if (verbose) puts("STUB: _AXSWebAccessibilityEventsEnabled called");
    return NULL;
}

void* _AXSWordFeedbackEnabled(void)
{
    if (verbose) puts("STUB: _AXSWordFeedbackEnabled called");
    return NULL;
}

void* _AXSZoomSpeakUnderFingerEnabled(void)
{
    if (verbose) puts("STUB: _AXSZoomSpeakUnderFingerEnabled called");
    return NULL;
}

void* _AXSZoomSpeakUnderFingerSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSZoomSpeakUnderFingerSetEnabled called");
    return NULL;
}

void* _AXSZoomTouchEnabled(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchEnabled called");
    return NULL;
}

void* _AXSZoomTouchSetEnabled(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchSetEnabled called");
    return NULL;
}

void* _AXSZoomTouchSetSmoothScalingDisabled(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchSetSmoothScalingDisabled called");
    return NULL;
}

void* _AXSZoomTouchSetToggledByPreferenceSwitch(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchSetToggledByPreferenceSwitch called");
    return NULL;
}

void* _AXSZoomTouchSetToggledByVoiceOver(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchSetToggledByVoiceOver called");
    return NULL;
}

void* _AXSZoomTouchSmoothScalingDisabled(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchSmoothScalingDisabled called");
    return NULL;
}

void* _AXSZoomTouchToggledByPreferenceSwitch(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchToggledByPreferenceSwitch called");
    return NULL;
}

void* _AXSZoomTouchToggledByVoiceOver(void)
{
    if (verbose) puts("STUB: _AXSZoomTouchToggledByVoiceOver called");
    return NULL;
}

void* _AXSetPreferenceWithNotification(void)
{
    if (verbose) puts("STUB: _AXSetPreferenceWithNotification called");
    return NULL;
}

void* _AXSystemRootDirectory(void)
{
    if (verbose) puts("STUB: _AXSystemRootDirectory called");
    return NULL;
}

void* _axsPrefsToNotificationMap(void)
{
    if (verbose) puts("STUB: _axsPrefsToNotificationMap called");
    return NULL;
}

void* _getFloatPreference(void)
{
    if (verbose) puts("STUB: _getFloatPreference called");
    return NULL;
}

void* _langSort(void)
{
    if (verbose) puts("STUB: _langSort called");
    return NULL;
}

void* _removePreference(void)
{
    if (verbose) puts("STUB: _removePreference called");
    return NULL;
}

void* _setNumberPreference(void)
{
    if (verbose) puts("STUB: _setNumberPreference called");
    return NULL;
}

void* _setValuePreference(void)
{
    if (verbose) puts("STUB: _setValuePreference called");
    return NULL;
}

void* _updateAccessibilitySettings(void)
{
    if (verbose) puts("STUB: _updateAccessibilitySettings called");
    return NULL;
}
