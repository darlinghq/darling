/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <CoreTelephony/CoreTelephony.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CTCallAddressBlocked(void)
{
    if (verbose) puts("STUB: CTCallAddressBlocked called");
    return NULL;
}

void* CTCallAnswer(void)
{
    if (verbose) puts("STUB: CTCallAnswer called");
    return NULL;
}

void* CTCallAnswerEndingActive(void)
{
    if (verbose) puts("STUB: CTCallAnswerEndingActive called");
    return NULL;
}

void* CTCallAnswerEndingActiveWithSourceIdentifier(void)
{
    if (verbose) puts("STUB: CTCallAnswerEndingActiveWithSourceIdentifier called");
    return NULL;
}

void* CTCallAnswerEndingHeld(void)
{
    if (verbose) puts("STUB: CTCallAnswerEndingHeld called");
    return NULL;
}

void* CTCallAnswerWithOptions(void)
{
    if (verbose) puts("STUB: CTCallAnswerWithOptions called");
    return NULL;
}

void* CTCallAnswerWithSourceIdentifier(void)
{
    if (verbose) puts("STUB: CTCallAnswerWithSourceIdentifier called");
    return NULL;
}

void* CTCallCopyAddress(void)
{
    if (verbose) puts("STUB: CTCallCopyAddress called");
    return NULL;
}

void* CTCallCopyCountryCode(void)
{
    if (verbose) puts("STUB: CTCallCopyCountryCode called");
    return NULL;
}

void* CTCallCopyISOCountryCode(void)
{
    if (verbose) puts("STUB: CTCallCopyISOCountryCode called");
    return NULL;
}

void* CTCallCopyName(void)
{
    if (verbose) puts("STUB: CTCallCopyName called");
    return NULL;
}

void* CTCallCopyNetworkCode(void)
{
    if (verbose) puts("STUB: CTCallCopyNetworkCode called");
    return NULL;
}

void* CTCallCopyUUID(void)
{
    if (verbose) puts("STUB: CTCallCopyUUID called");
    return NULL;
}

void* CTCallCopyUniqueStringID(void)
{
    if (verbose) puts("STUB: CTCallCopyUniqueStringID called");
    return NULL;
}

void* CTCallCreateCallRef(void)
{
    if (verbose) puts("STUB: CTCallCreateCallRef called");
    return NULL;
}

void* CTCallCreateFromSerializedData(void)
{
    if (verbose) puts("STUB: CTCallCreateFromSerializedData called");
    return NULL;
}

void* CTCallDial(void)
{
    if (verbose) puts("STUB: CTCallDial called");
    return NULL;
}

void* CTCallDialAny(void)
{
    if (verbose) puts("STUB: CTCallDialAny called");
    return NULL;
}

void* CTCallDialEmergency(void)
{
    if (verbose) puts("STUB: CTCallDialEmergency called");
    return NULL;
}

void* CTCallDialService(void)
{
    if (verbose) puts("STUB: CTCallDialService called");
    return NULL;
}

void* CTCallDialVoicemail(void)
{
    if (verbose) puts("STUB: CTCallDialVoicemail called");
    return NULL;
}

void* CTCallDialWithAssist(void)
{
    if (verbose) puts("STUB: CTCallDialWithAssist called");
    return NULL;
}

void* CTCallDialWithID(void)
{
    if (verbose) puts("STUB: CTCallDialWithID called");
    return NULL;
}

void* CTCallDialWithIDAndSourceIdentifier(void)
{
    if (verbose) puts("STUB: CTCallDialWithIDAndSourceIdentifier called");
    return NULL;
}

void* CTCallDialWithOptions(void)
{
    if (verbose) puts("STUB: CTCallDialWithOptions called");
    return NULL;
}

void* CTCallDialWithSourceIdentifier(void)
{
    if (verbose) puts("STUB: CTCallDialWithSourceIdentifier called");
    return NULL;
}

void* CTCallDidDeviceOriginateEnd(void)
{
    if (verbose) puts("STUB: CTCallDidDeviceOriginateEnd called");
    return NULL;
}

void* CTCallDisconnect(void)
{
    if (verbose) puts("STUB: CTCallDisconnect called");
    return NULL;
}

void* CTCallDisconnectWithCauseCode(void)
{
    if (verbose) puts("STUB: CTCallDisconnectWithCauseCode called");
    return NULL;
}

void* CTCallEndVideoConference(void)
{
    if (verbose) puts("STUB: CTCallEndVideoConference called");
    return NULL;
}

void* CTCallGetCallSubType(void)
{
    if (verbose) puts("STUB: CTCallGetCallSubType called");
    return NULL;
}

void* CTCallGetCallType(void)
{
    if (verbose) puts("STUB: CTCallGetCallType called");
    return NULL;
}

void* CTCallGetCauseCode(void)
{
    if (verbose) puts("STUB: CTCallGetCauseCode called");
    return NULL;
}

void* CTCallGetCauseCodeString(void)
{
    if (verbose) puts("STUB: CTCallGetCauseCodeString called");
    return NULL;
}

void* CTCallGetDuration(void)
{
    if (verbose) puts("STUB: CTCallGetDuration called");
    return NULL;
}

void* CTCallGetEmergencyStatus(void)
{
    if (verbose) puts("STUB: CTCallGetEmergencyStatus called");
    return NULL;
}

void* CTCallGetMultiPartyCallCountMax(void)
{
    if (verbose) puts("STUB: CTCallGetMultiPartyCallCountMax called");
    return NULL;
}

void* CTCallGetStartTime(void)
{
    if (verbose) puts("STUB: CTCallGetStartTime called");
    return NULL;
}

void* CTCallGetStatus(void)
{
    if (verbose) puts("STUB: CTCallGetStatus called");
    return NULL;
}

void* CTCallGetTypeID(void)
{
    if (verbose) puts("STUB: CTCallGetTypeID called");
    return NULL;
}

void* CTCallHold(void)
{
    if (verbose) puts("STUB: CTCallHold called");
    return NULL;
}

void* CTCallIsAlerting(void)
{
    if (verbose) puts("STUB: CTCallIsAlerting called");
    return NULL;
}

void* CTCallIsCallHandoffPossible(void)
{
    if (verbose) puts("STUB: CTCallIsCallHandoffPossible called");
    return NULL;
}

void* CTCallIsConferenced(void)
{
    if (verbose) puts("STUB: CTCallIsConferenced called");
    return NULL;
}

void* CTCallIsOutgoing(void)
{
    if (verbose) puts("STUB: CTCallIsOutgoing called");
    return NULL;
}

void* CTCallIsToVoicemail(void)
{
    if (verbose) puts("STUB: CTCallIsToVoicemail called");
    return NULL;
}

void* CTCallIsWaiting(void)
{
    if (verbose) puts("STUB: CTCallIsWaiting called");
    return NULL;
}

void* CTCallJoinConference(void)
{
    if (verbose) puts("STUB: CTCallJoinConference called");
    return NULL;
}

void* CTCallLeaveConference(void)
{
    if (verbose) puts("STUB: CTCallLeaveConference called");
    return NULL;
}

void* CTCallListDisconnect(void)
{
    if (verbose) puts("STUB: CTCallListDisconnect called");
    return NULL;
}

void* CTCallListDisconnectAll(void)
{
    if (verbose) puts("STUB: CTCallListDisconnectAll called");
    return NULL;
}

void* CTCallMediaStatusIsActive(void)
{
    if (verbose) puts("STUB: CTCallMediaStatusIsActive called");
    return NULL;
}

void* CTCallPhoneNumberIsMmiOrUssd(void)
{
    if (verbose) puts("STUB: CTCallPhoneNumberIsMmiOrUssd called");
    return NULL;
}

void* CTCallPullCallFromOtherDevice(void)
{
    if (verbose) puts("STUB: CTCallPullCallFromOtherDevice called");
    return NULL;
}

void* CTCallResume(void)
{
    if (verbose) puts("STUB: CTCallResume called");
    return NULL;
}

void* CTCallSetCallEndTime(void)
{
    if (verbose) puts("STUB: CTCallSetCallEndTime called");
    return NULL;
}

void* CTCallSetCallInfo(void)
{
    if (verbose) puts("STUB: CTCallSetCallInfo called");
    return NULL;
}

void* CTCallSetCallStartTime(void)
{
    if (verbose) puts("STUB: CTCallSetCallStartTime called");
    return NULL;
}

void* CTCallSetCallStatus(void)
{
    if (verbose) puts("STUB: CTCallSetCallStatus called");
    return NULL;
}

void* CTCallShouldPlayAudioTone(void)
{
    if (verbose) puts("STUB: CTCallShouldPlayAudioTone called");
    return NULL;
}

void* CTCallShouldSetupAudioInterruption(void)
{
    if (verbose) puts("STUB: CTCallShouldSetupAudioInterruption called");
    return NULL;
}

void* CTCallStartVideoConference(void)
{
    if (verbose) puts("STUB: CTCallStartVideoConference called");
    return NULL;
}

void* CTCallSwap(void)
{
    if (verbose) puts("STUB: CTCallSwap called");
    return NULL;
}

void* CTCallSwitchCallSource(void)
{
    if (verbose) puts("STUB: CTCallSwitchCallSource called");
    return NULL;
}

void* CTCallSwitchCallSourceAll(void)
{
    if (verbose) puts("STUB: CTCallSwitchCallSourceAll called");
    return NULL;
}

void* CTCellularDataPlanAccountUpdated(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanAccountUpdated called");
    return NULL;
}

void* CTCellularDataPlanActivationFailed(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanActivationFailed called");
    return NULL;
}

void* CTCellularDataPlanCopyAccountUrlString(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanCopyAccountUrlString called");
    return NULL;
}

void* CTCellularDataPlanCopyDeviceInfo(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanCopyDeviceInfo called");
    return NULL;
}

void* CTCellularDataPlanGetIsEnabled(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanGetIsEnabled called");
    return NULL;
}

void* CTCellularDataPlanSetIsEnabled(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanSetIsEnabled called");
    return NULL;
}

void* CTCellularDataPlanSetWebsheetVisibility(void)
{
    if (verbose) puts("STUB: CTCellularDataPlanSetWebsheetVisibility called");
    return NULL;
}

void* CTCopyCallManagementState(void)
{
    if (verbose) puts("STUB: CTCopyCallManagementState called");
    return NULL;
}

void* CTCopyCurrentCalls(void)
{
    if (verbose) puts("STUB: CTCopyCurrentCalls called");
    return NULL;
}

void* CTCopyCurrentCallsWithTypes(void)
{
    if (verbose) puts("STUB: CTCopyCurrentCallsWithTypes called");
    return NULL;
}

void* CTCreateCallInfoForHandoffCall(void)
{
    if (verbose) puts("STUB: CTCreateCallInfoForHandoffCall called");
    return NULL;
}

void* CTDTMFCancelHardPause(void)
{
    if (verbose) puts("STUB: CTDTMFCancelHardPause called");
    return NULL;
}

void* CTDTMFCopyHardPauseDigits(void)
{
    if (verbose) puts("STUB: CTDTMFCopyHardPauseDigits called");
    return NULL;
}

void* CTDTMFPlayStart(void)
{
    if (verbose) puts("STUB: CTDTMFPlayStart called");
    return NULL;
}

void* CTDTMFPlayStop(void)
{
    if (verbose) puts("STUB: CTDTMFPlayStop called");
    return NULL;
}

void* CTDTMFSendDigitsNow(void)
{
    if (verbose) puts("STUB: CTDTMFSendDigitsNow called");
    return NULL;
}

void* CTDataRateAsString(void)
{
    if (verbose) puts("STUB: CTDataRateAsString called");
    return NULL;
}

void* CTEmergencyModeTypeAsString(void)
{
    if (verbose) puts("STUB: CTEmergencyModeTypeAsString called");
    return NULL;
}

void* CTEnhancedLQMIndicationTypeAsString(void)
{
    if (verbose) puts("STUB: CTEnhancedLQMIndicationTypeAsString called");
    return NULL;
}

void* CTEsimCapabilityAsString(void)
{
    if (verbose) puts("STUB: CTEsimCapabilityAsString called");
    return NULL;
}

void* CTGetCurrentCallCount(void)
{
    if (verbose) puts("STUB: CTGetCurrentCallCount called");
    return NULL;
}

void* CTGetCurrentCallCountWithTypes(void)
{
    if (verbose) puts("STUB: CTGetCurrentCallCountWithTypes called");
    return NULL;
}

void* CTGetEmergencyWiFiConfig(void)
{
    if (verbose) puts("STUB: CTGetEmergencyWiFiConfig called");
    return NULL;
}

void* CTGetSignalStrength(void)
{
    if (verbose) puts("STUB: CTGetSignalStrength called");
    return NULL;
}

void* CTIndicatorsCopyVoicemailStatus(void)
{
    if (verbose) puts("STUB: CTIndicatorsCopyVoicemailStatus called");
    return NULL;
}

void* CTIndicatorsGetSignalStrength(void)
{
    if (verbose) puts("STUB: CTIndicatorsGetSignalStrength called");
    return NULL;
}

void* CTNetworkSelectionModeAsString(void)
{
    if (verbose) puts("STUB: CTNetworkSelectionModeAsString called");
    return NULL;
}

void* CTNetworkSelectionStateAsString(void)
{
    if (verbose) puts("STUB: CTNetworkSelectionStateAsString called");
    return NULL;
}

void* CTRegistrationAutomaticallySelectNetwork(void)
{
    if (verbose) puts("STUB: CTRegistrationAutomaticallySelectNetwork called");
    return NULL;
}

void* CTRegistrationCellularDataPlanAccountUpdated(void)
{
    if (verbose) puts("STUB: CTRegistrationCellularDataPlanAccountUpdated called");
    return NULL;
}

void* CTRegistrationCopyAbbreviatedOperatorName(void)
{
    if (verbose) puts("STUB: CTRegistrationCopyAbbreviatedOperatorName called");
    return NULL;
}

void* CTRegistrationCopyDataStatus(void)
{
    if (verbose) puts("STUB: CTRegistrationCopyDataStatus called");
    return NULL;
}

void* CTRegistrationCopyISOCountryCodeForMobileCountryCode(void)
{
    if (verbose) puts("STUB: CTRegistrationCopyISOCountryCodeForMobileCountryCode called");
    return NULL;
}

void* CTRegistrationCopyLocalizedOperatorName(void)
{
    if (verbose) puts("STUB: CTRegistrationCopyLocalizedOperatorName called");
    return NULL;
}

void* CTRegistrationCopyManualNetworkSelection(void)
{
    if (verbose) puts("STUB: CTRegistrationCopyManualNetworkSelection called");
    return NULL;
}

void* CTRegistrationCopyOperatorName(void)
{
    if (verbose) puts("STUB: CTRegistrationCopyOperatorName called");
    return NULL;
}

void* CTRegistrationCopySupportedDataRates(void)
{
    if (verbose) puts("STUB: CTRegistrationCopySupportedDataRates called");
    return NULL;
}

void* CTRegistrationDataCounterGetLastResetTime(void)
{
    if (verbose) puts("STUB: CTRegistrationDataCounterGetLastResetTime called");
    return NULL;
}

void* CTRegistrationGetCDMAInternationalRoaming(void)
{
    if (verbose) puts("STUB: CTRegistrationGetCDMAInternationalRoaming called");
    return NULL;
}

void* CTRegistrationGetCurrentMaxAllowedDataRate(void)
{
    if (verbose) puts("STUB: CTRegistrationGetCurrentMaxAllowedDataRate called");
    return NULL;
}

void* CTRegistrationGetDataAttached(void)
{
    if (verbose) puts("STUB: CTRegistrationGetDataAttached called");
    return NULL;
}

void* CTRegistrationGetDataContextActive(void)
{
    if (verbose) puts("STUB: CTRegistrationGetDataContextActive called");
    return NULL;
}

void* CTRegistrationGetDataIndicator(void)
{
    if (verbose) puts("STUB: CTRegistrationGetDataIndicator called");
    return NULL;
}

void* CTRegistrationGetEULocalBreakOutEnabled(void)
{
    if (verbose) puts("STUB: CTRegistrationGetEULocalBreakOutEnabled called");
    return NULL;
}

void* CTRegistrationGetEmergencyCallBackMode(void)
{
    if (verbose) puts("STUB: CTRegistrationGetEmergencyCallBackMode called");
    return NULL;
}

void* CTRegistrationGetInternationalDataAccessStatus(void)
{
    if (verbose) puts("STUB: CTRegistrationGetInternationalDataAccessStatus called");
    return NULL;
}

void* CTRegistrationGetNetworkSelectionMode(void)
{
    if (verbose) puts("STUB: CTRegistrationGetNetworkSelectionMode called");
    return NULL;
}

void* CTRegistrationGetNetworkSelectionState(void)
{
    if (verbose) puts("STUB: CTRegistrationGetNetworkSelectionState called");
    return NULL;
}

void* CTRegistrationGetStatus(void)
{
    if (verbose) puts("STUB: CTRegistrationGetStatus called");
    return NULL;
}

void* CTRegistrationIsCDMAInternationalRoamingSettingAllowed(void)
{
    if (verbose) puts("STUB: CTRegistrationIsCDMAInternationalRoamingSettingAllowed called");
    return NULL;
}

void* CTRegistrationIsEULocalBreakOutAvailable(void)
{
    if (verbose) puts("STUB: CTRegistrationIsEULocalBreakOutAvailable called");
    return NULL;
}

void* CTRegistrationIsInE911OverLTEMode(void)
{
    if (verbose) puts("STUB: CTRegistrationIsInE911OverLTEMode called");
    return NULL;
}

void* CTRegistrationManuallySelectNetwork(void)
{
    if (verbose) puts("STUB: CTRegistrationManuallySelectNetwork called");
    return NULL;
}

void* CTRegistrationNetworkSelectionMenuAvailable(void)
{
    if (verbose) puts("STUB: CTRegistrationNetworkSelectionMenuAvailable called");
    return NULL;
}

void* CTRegistrationRequestNetworkList(void)
{
    if (verbose) puts("STUB: CTRegistrationRequestNetworkList called");
    return NULL;
}

void* CTRegistrationSetCDMAInternationalRoaming(void)
{
    if (verbose) puts("STUB: CTRegistrationSetCDMAInternationalRoaming called");
    return NULL;
}

void* CTRegistrationSetDataContextActive(void)
{
    if (verbose) puts("STUB: CTRegistrationSetDataContextActive called");
    return NULL;
}

void* CTRegistrationSetEULocalBreakOutEnabled(void)
{
    if (verbose) puts("STUB: CTRegistrationSetEULocalBreakOutEnabled called");
    return NULL;
}

void* CTRegistrationSetInternationalDataAccessStatus(void)
{
    if (verbose) puts("STUB: CTRegistrationSetInternationalDataAccessStatus called");
    return NULL;
}

void* CTSIMSupportCopyMobileSubscriberCountryCode(void)
{
    if (verbose) puts("STUB: CTSIMSupportCopyMobileSubscriberCountryCode called");
    return NULL;
}

void* CTSIMSupportCopyMobileSubscriberIdentity(void)
{
    if (verbose) puts("STUB: CTSIMSupportCopyMobileSubscriberIdentity called");
    return NULL;
}

void* CTSIMSupportCopyMobileSubscriberNetworkCode(void)
{
    if (verbose) puts("STUB: CTSIMSupportCopyMobileSubscriberNetworkCode called");
    return NULL;
}

void* CTSIMSupportGetRemainingPINAttempts(void)
{
    if (verbose) puts("STUB: CTSIMSupportGetRemainingPINAttempts called");
    return NULL;
}

void* CTSIMSupportGetRemainingPUKAttempts(void)
{
    if (verbose) puts("STUB: CTSIMSupportGetRemainingPUKAttempts called");
    return NULL;
}

void* CTSIMSupportGetSIMStatus(void)
{
    if (verbose) puts("STUB: CTSIMSupportGetSIMStatus called");
    return NULL;
}

void* CTSIMSupportPINUnlock(void)
{
    if (verbose) puts("STUB: CTSIMSupportPINUnlock called");
    return NULL;
}

void* CTSIMSupportPUKUnlock(void)
{
    if (verbose) puts("STUB: CTSIMSupportPUKUnlock called");
    return NULL;
}

void* CTServerConnectionDualSimCapabilityAsString(void)
{
    if (verbose) puts("STUB: CTServerConnectionDualSimCapabilityAsString called");
    return NULL;
}

void* CTSettingCopyDualIPBringUp(void)
{
    if (verbose) puts("STUB: CTSettingCopyDualIPBringUp called");
    return NULL;
}

void* CTSettingCopyEPDGAddress(void)
{
    if (verbose) puts("STUB: CTSettingCopyEPDGAddress called");
    return NULL;
}

void* CTSettingCopyEffectiveSimInfo(void)
{
    if (verbose) puts("STUB: CTSettingCopyEffectiveSimInfo called");
    return NULL;
}

void* CTSettingCopyFakePrimaryDNSKey(void)
{
    if (verbose) puts("STUB: CTSettingCopyFakePrimaryDNSKey called");
    return NULL;
}

void* CTSettingCopyFakeSecondaryDNSKey(void)
{
    if (verbose) puts("STUB: CTSettingCopyFakeSecondaryDNSKey called");
    return NULL;
}

void* CTSettingCopyMyPhoneNumber(void)
{
    if (verbose) puts("STUB: CTSettingCopyMyPhoneNumber called");
    return NULL;
}

void* CTSettingCopyMyPhoneNumberExtended(void)
{
    if (verbose) puts("STUB: CTSettingCopyMyPhoneNumberExtended called");
    return NULL;
}

void* CTSettingCopyPCSCFAddress(void)
{
    if (verbose) puts("STUB: CTSettingCopyPCSCFAddress called");
    return NULL;
}

void* CTSettingCopyShowVoLTEStatusIndicator(void)
{
    if (verbose) puts("STUB: CTSettingCopyShowVoLTEStatusIndicator called");
    return NULL;
}

void* CTSettingCopyVerifyAPN(void)
{
    if (verbose) puts("STUB: CTSettingCopyVerifyAPN called");
    return NULL;
}

void* CTSettingCopyVerifyCarrierBundles(void)
{
    if (verbose) puts("STUB: CTSettingCopyVerifyCarrierBundles called");
    return NULL;
}

void* CTSettingCopyWiFiCallingInCallHandover(void)
{
    if (verbose) puts("STUB: CTSettingCopyWiFiCallingInCallHandover called");
    return NULL;
}

void* CTSettingRequest(void)
{
    if (verbose) puts("STUB: CTSettingRequest called");
    return NULL;
}

void* CTSettingSave(void)
{
    if (verbose) puts("STUB: CTSettingSave called");
    return NULL;
}

void* CTSettingSetDualIPBringUp(void)
{
    if (verbose) puts("STUB: CTSettingSetDualIPBringUp called");
    return NULL;
}

void* CTSettingSetEPDGAddress(void)
{
    if (verbose) puts("STUB: CTSettingSetEPDGAddress called");
    return NULL;
}

void* CTSettingSetFakePrimaryDNSKey(void)
{
    if (verbose) puts("STUB: CTSettingSetFakePrimaryDNSKey called");
    return NULL;
}

void* CTSettingSetFakeSecondaryDNSKey(void)
{
    if (verbose) puts("STUB: CTSettingSetFakeSecondaryDNSKey called");
    return NULL;
}

void* CTSettingSetIMSIOverride(void)
{
    if (verbose) puts("STUB: CTSettingSetIMSIOverride called");
    return NULL;
}

void* CTSettingSetPCSCFAddress(void)
{
    if (verbose) puts("STUB: CTSettingSetPCSCFAddress called");
    return NULL;
}

void* CTSettingSetShowVoLTEStatusIndicator(void)
{
    if (verbose) puts("STUB: CTSettingSetShowVoLTEStatusIndicator called");
    return NULL;
}

void* CTSettingSetVerifyAPN(void)
{
    if (verbose) puts("STUB: CTSettingSetVerifyAPN called");
    return NULL;
}

void* CTSettingSetVerifyCarrierBundles(void)
{
    if (verbose) puts("STUB: CTSettingSetVerifyCarrierBundles called");
    return NULL;
}

void* CTSettingSetWiFiCallingInCallHandover(void)
{
    if (verbose) puts("STUB: CTSettingSetWiFiCallingInCallHandover called");
    return NULL;
}

void* CTSettingsCopyiWlanGatewayAddress(void)
{
    if (verbose) puts("STUB: CTSettingsCopyiWlanGatewayAddress called");
    return NULL;
}

void* CTStartManualOTASP(void)
{
    if (verbose) puts("STUB: CTStartManualOTASP called");
    return NULL;
}

void* CTStartOTASP(void)
{
    if (verbose) puts("STUB: CTStartOTASP called");
    return NULL;
}

void* CTSubscriptionSlotAsString(void)
{
    if (verbose) puts("STUB: CTSubscriptionSlotAsString called");
    return NULL;
}

void* CTSubscriptionSlotAsUUID(void)
{
    if (verbose) puts("STUB: CTSubscriptionSlotAsUUID called");
    return NULL;
}

void* CTSweetgumConvertDataUnits(void)
{
    if (verbose) puts("STUB: CTSweetgumConvertDataUnits called");
    return NULL;
}

void* CTSweetgumDataUnitsAsString(void)
{
    if (verbose) puts("STUB: CTSweetgumDataUnitsAsString called");
    return NULL;
}

void* CTSweetgumDeviceTypeAsString(void)
{
    if (verbose) puts("STUB: CTSweetgumDeviceTypeAsString called");
    return NULL;
}

void* CTSweetgumPlanCategoryAsString(void)
{
    if (verbose) puts("STUB: CTSweetgumPlanCategoryAsString called");
    return NULL;
}

void* CTSweetgumPlanPurchaseStatusAsString(void)
{
    if (verbose) puts("STUB: CTSweetgumPlanPurchaseStatusAsString called");
    return NULL;
}

void* CTSweetgumPlanStatusAsString(void)
{
    if (verbose) puts("STUB: CTSweetgumPlanStatusAsString called");
    return NULL;
}

void* CTSweetgumRegistrationStatusAsString(void)
{
    if (verbose) puts("STUB: CTSweetgumRegistrationStatusAsString called");
    return NULL;
}

void* CTTelephonyCenterAddObserver(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterAddObserver called");
    return NULL;
}

void* CTTelephonyCenterGetDefault(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterGetDefault called");
    return NULL;
}

void* CTTelephonyCenterGetRunLoop(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterGetRunLoop called");
    return NULL;
}

void* CTTelephonyCenterRemoveEveryObserver(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterRemoveEveryObserver called");
    return NULL;
}

void* CTTelephonyCenterRemoveObserver(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterRemoveObserver called");
    return NULL;
}

void* CTTelephonyCenterSendBarrier(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterSendBarrier called");
    return NULL;
}

void* CTTelephonyCenterSetDefaultDispatchQueue(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterSetDefaultDispatchQueue called");
    return NULL;
}

void* CTTelephonyCenterSetDefaultDispatchQueuePermanently(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterSetDefaultDispatchQueuePermanently called");
    return NULL;
}

void* CTTelephonyCenterSetDefaultRunloop(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterSetDefaultRunloop called");
    return NULL;
}

void* CTTelephonyCenterSetDefaultRunloopPermanently(void)
{
    if (verbose) puts("STUB: CTTelephonyCenterSetDefaultRunloopPermanently called");
    return NULL;
}

void* CTUSSDSessionCancel(void)
{
    if (verbose) puts("STUB: CTUSSDSessionCancel called");
    return NULL;
}

void* CTUSSDSessionSendResponse(void)
{
    if (verbose) puts("STUB: CTUSSDSessionSendResponse called");
    return NULL;
}

void* ConvertAccountQueryTypesToMask(void)
{
    if (verbose) puts("STUB: ConvertAccountQueryTypesToMask called");
    return NULL;
}

void* GetCarrierEntitlementNameById(void)
{
    if (verbose) puts("STUB: GetCarrierEntitlementNameById called");
    return NULL;
}

void* GetCarrierEntitlementResultString(void)
{
    if (verbose) puts("STUB: GetCarrierEntitlementResultString called");
    return NULL;
}

void* HexToString(void)
{
    if (verbose) puts("STUB: HexToString called");
    return NULL;
}

void* VMCopyDictionaryForNotificationString(void)
{
    if (verbose) puts("STUB: VMCopyDictionaryForNotificationString called");
    return NULL;
}

void* _CFDataCreateFromHexString(void)
{
    if (verbose) puts("STUB: _CFDataCreateFromHexString called");
    return NULL;
}

void* _CFStringCopyMCCMNCFromIMSI(void)
{
    if (verbose) puts("STUB: _CFStringCopyMCCMNCFromIMSI called");
    return NULL;
}

void* _CFStringCreateHexFromBinary(void)
{
    if (verbose) puts("STUB: _CFStringCreateHexFromBinary called");
    return NULL;
}

void* _CFStringGetMobileUser(void)
{
    if (verbose) puts("STUB: _CFStringGetMobileUser called");
    return NULL;
}

void* _CFStringGetOrCreateCString(void)
{
    if (verbose) puts("STUB: _CFStringGetOrCreateCString called");
    return NULL;
}

void* _CFStringGetWirelessUser(void)
{
    if (verbose) puts("STUB: _CFStringGetWirelessUser called");
    return NULL;
}

void* _CFStringHasPrefixCaseInsensitive(void)
{
    if (verbose) puts("STUB: _CFStringHasPrefixCaseInsensitive called");
    return NULL;
}

void* _CTCallCopyCurrentCalls(void)
{
    if (verbose) puts("STUB: _CTCallCopyCurrentCalls called");
    return NULL;
}

void* _CTCallHistoryStoreCallTimersGetAll(void)
{
    if (verbose) puts("STUB: _CTCallHistoryStoreCallTimersGetAll called");
    return NULL;
}

void* _CTCallHistoryStoreGetCountOfMissedCallSince(void)
{
    if (verbose) puts("STUB: _CTCallHistoryStoreGetCountOfMissedCallSince called");
    return NULL;
}

void* _CTCallHistoryStoreGetCountOfMissedCallWithTypesSince(void)
{
    if (verbose) puts("STUB: _CTCallHistoryStoreGetCountOfMissedCallWithTypesSince called");
    return NULL;
}

void* _CTServerConnectionAddIdentifierException(void)
{
    if (verbose) puts("STUB: _CTServerConnectionAddIdentifierException called");
    return NULL;
}

void* _CTServerConnectionAddToRunLoop(void)
{
    if (verbose) puts("STUB: _CTServerConnectionAddToRunLoop called");
    return NULL;
}

void* _CTServerConnectionAddVictimFreq(void)
{
    if (verbose) puts("STUB: _CTServerConnectionAddVictimFreq called");
    return NULL;
}

void* _CTServerConnectionAudioSampleRateSwitchDidComplete(void)
{
    if (verbose) puts("STUB: _CTServerConnectionAudioSampleRateSwitchDidComplete called");
    return NULL;
}

void* _CTServerConnectionAutomaticallySelectNetwork(void)
{
    if (verbose) puts("STUB: _CTServerConnectionAutomaticallySelectNetwork called");
    return NULL;
}

void* _CTServerConnectionBlockBBFreqReport(void)
{
    if (verbose) puts("STUB: _CTServerConnectionBlockBBFreqReport called");
    return NULL;
}

void* _CTServerConnectionCanSetCapability(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCanSetCapability called");
    return NULL;
}

void* _CTServerConnectionCanSetCapabilityExtended(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCanSetCapabilityExtended called");
    return NULL;
}

void* _CTServerConnectionCarrierSettingsCopyValue(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCarrierSettingsCopyValue called");
    return NULL;
}

void* _CTServerConnectionCarrierSettingsCopyValueForPreferredDataSIM(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCarrierSettingsCopyValueForPreferredDataSIM called");
    return NULL;
}

void* _CTServerConnectionCellMonitorCopyCellInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorCopyCellInfo called");
    return NULL;
}

void* _CTServerConnectionCellMonitorGetCellCount(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorGetCellCount called");
    return NULL;
}

void* _CTServerConnectionCellMonitorGetCellInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorGetCellInfo called");
    return NULL;
}

void* _CTServerConnectionCellMonitorGetUmtsCellCount(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorGetUmtsCellCount called");
    return NULL;
}

void* _CTServerConnectionCellMonitorGetUmtsCellInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorGetUmtsCellInfo called");
    return NULL;
}

void* _CTServerConnectionCellMonitorStart(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorStart called");
    return NULL;
}

void* _CTServerConnectionCellMonitorStop(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellMonitorStop called");
    return NULL;
}

void* _CTServerConnectionCellularDataPlanAccountUpdated(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellularDataPlanAccountUpdated called");
    return NULL;
}

void* _CTServerConnectionCellularDataPlanActivationFailed(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCellularDataPlanActivationFailed called");
    return NULL;
}

void* _CTServerConnectionCleanBasebandLogs(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCleanBasebandLogs called");
    return NULL;
}

void* _CTServerConnectionClearVictimFreq(void)
{
    if (verbose) puts("STUB: _CTServerConnectionClearVictimFreq called");
    return NULL;
}

void* _CTServerConnectionConfigAndCopyTxController(void)
{
    if (verbose) puts("STUB: _CTServerConnectionConfigAndCopyTxController called");
    return NULL;
}

void* _CTServerConnectionConfigMaxRadioPower(void)
{
    if (verbose) puts("STUB: _CTServerConnectionConfigMaxRadioPower called");
    return NULL;
}

void* _CTServerConnectionConfigMaxTransmitPower(void)
{
    if (verbose) puts("STUB: _CTServerConnectionConfigMaxTransmitPower called");
    return NULL;
}

void* _CTServerConnectionConfigureLogging(void)
{
    if (verbose) puts("STUB: _CTServerConnectionConfigureLogging called");
    return NULL;
}

void* _CTServerConnectionCopyAbbreviatedOperatorName(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyAbbreviatedOperatorName called");
    return NULL;
}

void* _CTServerConnectionCopyAccountInformation(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyAccountInformation called");
    return NULL;
}

void* _CTServerConnectionCopyActiveAndSupportedBands(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyActiveAndSupportedBands called");
    return NULL;
}

void* _CTServerConnectionCopyAudioVocoderInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyAudioVocoderInfo called");
    return NULL;
}

void* _CTServerConnectionCopyBandInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyBandInfo called");
    return NULL;
}

void* _CTServerConnectionCopyBasebandSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyBasebandSettings called");
    return NULL;
}

void* _CTServerConnectionCopyBasebandThumbprint(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyBasebandThumbprint called");
    return NULL;
}

void* _CTServerConnectionCopyCallManagementState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCallManagementState called");
    return NULL;
}

void* _CTServerConnectionCopyCarrierBundleInfoArray(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCarrierBundleInfoArray called");
    return NULL;
}

void* _CTServerConnectionCopyCellBroadcastAlertTypes(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCellBroadcastAlertTypes called");
    return NULL;
}

void* _CTServerConnectionCopyCellularTransmitState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCellularTransmitState called");
    return NULL;
}

void* _CTServerConnectionCopyCellularUsagePolicy(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCellularUsagePolicy called");
    return NULL;
}

void* _CTServerConnectionCopyCellularUsageWorkspaceInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCellularUsageWorkspaceInfo called");
    return NULL;
}

void* _CTServerConnectionCopyCompanionPhoneNumber(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCompanionPhoneNumber called");
    return NULL;
}

void* _CTServerConnectionCopyCountryCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCountryCode called");
    return NULL;
}

void* _CTServerConnectionCopyCurrentMTU(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyCurrentMTU called");
    return NULL;
}

void* _CTServerConnectionCopyDataActivityAssertionProcesses(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyDataActivityAssertionProcesses called");
    return NULL;
}

void* _CTServerConnectionCopyDataStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyDataStatus called");
    return NULL;
}

void* _CTServerConnectionCopyDataUsageForSubscriber(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyDataUsageForSubscriber called");
    return NULL;
}

void* _CTServerConnectionCopyDualSimCapability(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyDualSimCapability called");
    return NULL;
}

void* _CTServerConnectionCopyEffectiveSimInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyEffectiveSimInfo called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareBaselineVersion(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareBaselineVersion called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareManifestData(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareManifestData called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareManifestStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareManifestStatus called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareNonce(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareNonce called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwarePreflightInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwarePreflightInfo called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareSecurityInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareSecurityInfo called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareUpdateInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareUpdateInfo called");
    return NULL;
}

void* _CTServerConnectionCopyFirmwareVersion(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyFirmwareVersion called");
    return NULL;
}

void* _CTServerConnectionCopyISOForMCC(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyISOForMCC called");
    return NULL;
}

void* _CTServerConnectionCopyLastKnownCountryCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyLastKnownCountryCode called");
    return NULL;
}

void* _CTServerConnectionCopyLastKnownMobileCountryCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyLastKnownMobileCountryCode called");
    return NULL;
}

void* _CTServerConnectionCopyLastKnownMobileSubscriberCountryCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyLastKnownMobileSubscriberCountryCode called");
    return NULL;
}

void* _CTServerConnectionCopyListVictimFreq(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyListVictimFreq called");
    return NULL;
}

void* _CTServerConnectionCopyLocalizedOperatorName(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyLocalizedOperatorName called");
    return NULL;
}

void* _CTServerConnectionCopyManualNetworkSelection(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyManualNetworkSelection called");
    return NULL;
}

void* _CTServerConnectionCopyMobileCountryCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileCountryCode called");
    return NULL;
}

void* _CTServerConnectionCopyMobileEquipmentInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileEquipmentInfo called");
    return NULL;
}

void* _CTServerConnectionCopyMobileNetworkCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileNetworkCode called");
    return NULL;
}

void* _CTServerConnectionCopyMobileSubscriberAndIsoCountryCodes(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileSubscriberAndIsoCountryCodes called");
    return NULL;
}

void* _CTServerConnectionCopyMobileSubscriberCountryCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileSubscriberCountryCode called");
    return NULL;
}

void* _CTServerConnectionCopyMobileSubscriberHomeCountryList(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileSubscriberHomeCountryList called");
    return NULL;
}

void* _CTServerConnectionCopyMobileSubscriberIdentity(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileSubscriberIdentity called");
    return NULL;
}

void* _CTServerConnectionCopyMobileSubscriberNetworkCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyMobileSubscriberNetworkCode called");
    return NULL;
}

void* _CTServerConnectionCopyNetworkCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyNetworkCode called");
    return NULL;
}

void* _CTServerConnectionCopyNetworkList(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyNetworkList called");
    return NULL;
}

void* _CTServerConnectionCopyNextCall(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyNextCall called");
    return NULL;
}

void* _CTServerConnectionCopyNumericNetworkCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyNumericNetworkCode called");
    return NULL;
}

void* _CTServerConnectionCopyOperatorName(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyOperatorName called");
    return NULL;
}

void* _CTServerConnectionCopyPacketContextInterfaceName(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPacketContextInterfaceName called");
    return NULL;
}

void* _CTServerConnectionCopyPacketContextInterfaceNameByServiceType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPacketContextInterfaceNameByServiceType called");
    return NULL;
}

void* _CTServerConnectionCopyPhoneBookEntry(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPhoneBookEntry called");
    return NULL;
}

void* _CTServerConnectionCopyPhoneNumber(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPhoneNumber called");
    return NULL;
}

void* _CTServerConnectionCopyPhoneNumberSignature(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPhoneNumberSignature called");
    return NULL;
}

void* _CTServerConnectionCopyPhoneServiceDeviceList(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPhoneServiceDeviceList called");
    return NULL;
}

void* _CTServerConnectionCopyPostponementStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyPostponementStatus called");
    return NULL;
}

void* _CTServerConnectionCopyProviderNameUsingCarrierBundle(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyProviderNameUsingCarrierBundle called");
    return NULL;
}

void* _CTServerConnectionCopyRegistrationInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyRegistrationInfo called");
    return NULL;
}

void* _CTServerConnectionCopyReliableNetworkFallbackSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyReliableNetworkFallbackSettings called");
    return NULL;
}

void* _CTServerConnectionCopySIMIdentity(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopySIMIdentity called");
    return NULL;
}

void* _CTServerConnectionCopySIMToolkitList(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopySIMToolkitList called");
    return NULL;
}

void* _CTServerConnectionCopySIMToolkitMenu(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopySIMToolkitMenu called");
    return NULL;
}

void* _CTServerConnectionCopyServiceCenterAddressValue(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyServiceCenterAddressValue called");
    return NULL;
}

void* _CTServerConnectionCopyServingPLMN(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyServingPLMN called");
    return NULL;
}

void* _CTServerConnectionCopyStartDateOfCellularDataUsageRecords(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyStartDateOfCellularDataUsageRecords called");
    return NULL;
}

void* _CTServerConnectionCopySupportedDataRates(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopySupportedDataRates called");
    return NULL;
}

void* _CTServerConnectionCopySystemCapabilities(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopySystemCapabilities called");
    return NULL;
}

void* _CTServerConnectionCopyTraceProperty(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyTraceProperty called");
    return NULL;
}

void* _CTServerConnectionCopyUSSDString(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyUSSDString called");
    return NULL;
}

void* _CTServerConnectionCopyUsageAlertParameters(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyUsageAlertParameters called");
    return NULL;
}

void* _CTServerConnectionCopyVoiceCarrierBundleValue(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyVoiceCarrierBundleValue called");
    return NULL;
}

void* _CTServerConnectionCopyVoiceMailInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCopyVoiceMailInfo called");
    return NULL;
}

void* _CTServerConnectionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCreate called");
    return NULL;
}

void* _CTServerConnectionCreateAndLaunchWithIdentifier(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCreateAndLaunchWithIdentifier called");
    return NULL;
}

void* _CTServerConnectionCreateEncryptedIdentity(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCreateEncryptedIdentity called");
    return NULL;
}

void* _CTServerConnectionCreateOnRunLoop(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCreateOnRunLoop called");
    return NULL;
}

void* _CTServerConnectionCreateOnTargetQueue(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCreateOnTargetQueue called");
    return NULL;
}

void* _CTServerConnectionCreateWithIdentifier(void)
{
    if (verbose) puts("STUB: _CTServerConnectionCreateWithIdentifier called");
    return NULL;
}

void* _CTServerConnectionDataActivationPopupAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDataActivationPopupAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionDeviceManagementUpdate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDeviceManagementUpdate called");
    return NULL;
}

void* _CTServerConnectionDisablePCMChannel(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDisablePCMChannel called");
    return NULL;
}

void* _CTServerConnectionDisableRegistration(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDisableRegistration called");
    return NULL;
}

void* _CTServerConnectionDoCSIPropertyOperation(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDoCSIPropertyOperation called");
    return NULL;
}

void* _CTServerConnectionDoModuleOperation(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDoModuleOperation called");
    return NULL;
}

void* _CTServerConnectionDormancySuspendAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDormancySuspendAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionDormancySuspendAssertionCreateWithExpirationCB(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDormancySuspendAssertionCreateWithExpirationCB called");
    return NULL;
}

void* _CTServerConnectionDropIPPackets(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDropIPPackets called");
    return NULL;
}

void* _CTServerConnectionDumpBasebandState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDumpBasebandState called");
    return NULL;
}

void* _CTServerConnectionDumpMemory(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDumpMemory called");
    return NULL;
}

void* _CTServerConnectionDumpState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionDumpState called");
    return NULL;
}

void* _CTServerConnectionEchoCancelationAndNoiseReduction(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEchoCancelationAndNoiseReduction called");
    return NULL;
}

void* _CTServerConnectionEnableEmergencyCallBackMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnableEmergencyCallBackMode called");
    return NULL;
}

void* _CTServerConnectionEnableHAC(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnableHAC called");
    return NULL;
}

void* _CTServerConnectionEnableMediaToDownlink(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnableMediaToDownlink called");
    return NULL;
}

void* _CTServerConnectionEnablePCMChannel(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnablePCMChannel called");
    return NULL;
}

void* _CTServerConnectionEnableRegistration(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnableRegistration called");
    return NULL;
}

void* _CTServerConnectionEnableRemoteDiagnostics(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnableRemoteDiagnostics called");
    return NULL;
}

void* _CTServerConnectionEnableVoicePrivacy(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEnableVoicePrivacy called");
    return NULL;
}

void* _CTServerConnectionEntitlementsClearSecondaryIccids(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEntitlementsClearSecondaryIccids called");
    return NULL;
}

void* _CTServerConnectionEntitlementsGetPreferredRoamingNetworks(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEntitlementsGetPreferredRoamingNetworks called");
    return NULL;
}

void* _CTServerConnectionEntitlementsGetSIMStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEntitlementsGetSIMStatus called");
    return NULL;
}

void* _CTServerConnectionEntitlementsSetPreferredRoamingNetworks(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEntitlementsSetPreferredRoamingNetworks called");
    return NULL;
}

void* _CTServerConnectionEntitlementsSetSecondaryIccids(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEntitlementsSetSecondaryIccids called");
    return NULL;
}

void* _CTServerConnectionEntitlementsSignUpSIMService(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEntitlementsSignUpSIMService called");
    return NULL;
}

void* _CTServerConnectionEraseAllUserProfiles(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEraseAllUserProfiles called");
    return NULL;
}

void* _CTServerConnectionEraseBasebandSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEraseBasebandSettings called");
    return NULL;
}

void* _CTServerConnectionEraseCellularDataUsageRecords(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEraseCellularDataUsageRecords called");
    return NULL;
}

void* _CTServerConnectionEraseCellularDataUsageRecordsEx(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEraseCellularDataUsageRecordsEx called");
    return NULL;
}

void* _CTServerConnectionEraseCommCentersPreferences(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEraseCommCentersPreferences called");
    return NULL;
}

void* _CTServerConnectionEraseNetworkSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEraseNetworkSettings called");
    return NULL;
}

void* _CTServerConnectionEvaluateMobileSubscriberIdentity(void)
{
    if (verbose) puts("STUB: _CTServerConnectionEvaluateMobileSubscriberIdentity called");
    return NULL;
}

void* _CTServerConnectionExecuteCommand(void)
{
    if (verbose) puts("STUB: _CTServerConnectionExecuteCommand called");
    return NULL;
}

void* _CTServerConnectionFetchNetworkList(void)
{
    if (verbose) puts("STUB: _CTServerConnectionFetchNetworkList called");
    return NULL;
}

void* _CTServerConnectionFetchPhonebook(void)
{
    if (verbose) puts("STUB: _CTServerConnectionFetchPhonebook called");
    return NULL;
}

void* _CTServerConnectionFetchTimeFromNetwork(void)
{
    if (verbose) puts("STUB: _CTServerConnectionFetchTimeFromNetwork called");
    return NULL;
}

void* _CTServerConnectionGetAPNConfiguration(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetAPNConfiguration called");
    return NULL;
}

void* _CTServerConnectionGetActiveWirelessTechnology(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetActiveWirelessTechnology called");
    return NULL;
}

void* _CTServerConnectionGetAllowedRadioMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetAllowedRadioMode called");
    return NULL;
}

void* _CTServerConnectionGetAttachAPNSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetAttachAPNSettings called");
    return NULL;
}

void* _CTServerConnectionGetAudioLoggingEnabled(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetAudioLoggingEnabled called");
    return NULL;
}

void* _CTServerConnectionGetCDMAInternationalRoaming(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCDMAInternationalRoaming called");
    return NULL;
}

void* _CTServerConnectionGetCDRXWithLTEState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCDRXWithLTEState called");
    return NULL;
}

void* _CTServerConnectionGetCapabilityStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCapabilityStatus called");
    return NULL;
}

void* _CTServerConnectionGetCapabilityStatusExtended(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCapabilityStatusExtended called");
    return NULL;
}

void* _CTServerConnectionGetCarrierEntitlements(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCarrierEntitlements called");
    return NULL;
}

void* _CTServerConnectionGetCarrierSpaceUpdates(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCarrierSpaceUpdates called");
    return NULL;
}

void* _CTServerConnectionGetCellBroadcastSettingForAlertType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellBroadcastSettingForAlertType called");
    return NULL;
}

void* _CTServerConnectionGetCellID(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellID called");
    return NULL;
}

void* _CTServerConnectionGetCellularDataIsDisallowed(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularDataIsDisallowed called");
    return NULL;
}

void* _CTServerConnectionGetCellularDataIsEnabled(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularDataIsEnabled called");
    return NULL;
}

void* _CTServerConnectionGetCellularDataIsEnabledEx(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularDataIsEnabledEx called");
    return NULL;
}

void* _CTServerConnectionGetCellularDataSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularDataSettings called");
    return NULL;
}

void* _CTServerConnectionGetCellularUsagePolicyEx(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularUsagePolicyEx called");
    return NULL;
}

void* _CTServerConnectionGetCellularUsagePolicyMirroring(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularUsagePolicyMirroring called");
    return NULL;
}

void* _CTServerConnectionGetCellularUsageWorkspaceInfoEx(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCellularUsageWorkspaceInfoEx called");
    return NULL;
}

void* _CTServerConnectionGetCommCenterInitializationState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCommCenterInitializationState called");
    return NULL;
}

void* _CTServerConnectionGetCurrentMaxAllowedDataRate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetCurrentMaxAllowedDataRate called");
    return NULL;
}

void* _CTServerConnectionGetDTMFType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetDTMFType called");
    return NULL;
}

void* _CTServerConnectionGetDataServiceAvailability(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetDataServiceAvailability called");
    return NULL;
}

void* _CTServerConnectionGetDisallowTelephonyFeature(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetDisallowTelephonyFeature called");
    return NULL;
}

void* _CTServerConnectionGetE911OverLTEModeStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetE911OverLTEModeStatus called");
    return NULL;
}

void* _CTServerConnectionGetE911OverWifiModeStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetE911OverWifiModeStatus called");
    return NULL;
}

void* _CTServerConnectionGetEMBMSSAIListInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEMBMSSAIListInfo called");
    return NULL;
}

void* _CTServerConnectionGetEMBMSSigInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEMBMSSigInfo called");
    return NULL;
}

void* _CTServerConnectionGetEMBMSStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEMBMSStatus called");
    return NULL;
}

void* _CTServerConnectionGetEmergencyCallBackMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEmergencyCallBackMode called");
    return NULL;
}

void* _CTServerConnectionGetEmergencyCallStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEmergencyCallStatus called");
    return NULL;
}

void* _CTServerConnectionGetEnableOnlyHomeNetwork(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEnableOnlyHomeNetwork called");
    return NULL;
}

void* _CTServerConnectionGetEnhancedVoiceLinkQualityMetric(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetEnhancedVoiceLinkQualityMetric called");
    return NULL;
}

void* _CTServerConnectionGetHACState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetHACState called");
    return NULL;
}

void* _CTServerConnectionGetInterfaceFamilies(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetInterfaceFamilies called");
    return NULL;
}

void* _CTServerConnectionGetInternationalDataAccessStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetInternationalDataAccessStatus called");
    return NULL;
}

void* _CTServerConnectionGetLocationAreaCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetLocationAreaCode called");
    return NULL;
}

void* _CTServerConnectionGetManagedCellularProfile(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetManagedCellularProfile called");
    return NULL;
}

void* _CTServerConnectionGetMultiPartyCallCountMaximum(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetMultiPartyCallCountMaximum called");
    return NULL;
}

void* _CTServerConnectionGetNATTKeepAliveOverCell(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetNATTKeepAliveOverCell called");
    return NULL;
}

void* _CTServerConnectionGetNetworkSelectionMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetNetworkSelectionMode called");
    return NULL;
}

void* _CTServerConnectionGetNetworkSelectionState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetNetworkSelectionState called");
    return NULL;
}

void* _CTServerConnectionGetOTAServerOverride(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetOTAServerOverride called");
    return NULL;
}

void* _CTServerConnectionGetPLMNPriorityInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPLMNPriorityInfo called");
    return NULL;
}

void* _CTServerConnectionGetPacketContextActive(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPacketContextActive called");
    return NULL;
}

void* _CTServerConnectionGetPacketContextActiveByServiceType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPacketContextActiveByServiceType called");
    return NULL;
}

void* _CTServerConnectionGetPacketContextActiveByServiceTypeExtended(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPacketContextActiveByServiceTypeExtended called");
    return NULL;
}

void* _CTServerConnectionGetPacketContextCount(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPacketContextCount called");
    return NULL;
}

void* _CTServerConnectionGetPacketContextStatistics(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPacketContextStatistics called");
    return NULL;
}

void* _CTServerConnectionGetPacketDataLoadInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPacketDataLoadInfo called");
    return NULL;
}

void* _CTServerConnectionGetPhonebookEntryCount(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetPhonebookEntryCount called");
    return NULL;
}

void* _CTServerConnectionGetRATSelection(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRATSelection called");
    return NULL;
}

void* _CTServerConnectionGetRadioAccessTechnology(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRadioAccessTechnology called");
    return NULL;
}

void* _CTServerConnectionGetRadioAccessTechnologyForPreferredDataSIM(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRadioAccessTechnologyForPreferredDataSIM called");
    return NULL;
}

void* _CTServerConnectionGetRadioModuleIsDead(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRadioModuleIsDead called");
    return NULL;
}

void* _CTServerConnectionGetRadioState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRadioState called");
    return NULL;
}

void* _CTServerConnectionGetRegistrationCauseCode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRegistrationCauseCode called");
    return NULL;
}

void* _CTServerConnectionGetRegistrationStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRegistrationStatus called");
    return NULL;
}

void* _CTServerConnectionGetRemainingPINAttemptCount(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRemainingPINAttemptCount called");
    return NULL;
}

void* _CTServerConnectionGetRemainingPUKAttemptCount(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRemainingPUKAttemptCount called");
    return NULL;
}

void* _CTServerConnectionGetRemoteBundleInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRemoteBundleInfo called");
    return NULL;
}

void* _CTServerConnectionGetRemoteServerURL(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetRemoteServerURL called");
    return NULL;
}

void* _CTServerConnectionGetResetCellularAccountSettingsIsAllowed(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetResetCellularAccountSettingsIsAllowed called");
    return NULL;
}

void* _CTServerConnectionGetResetState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetResetState called");
    return NULL;
}

void* _CTServerConnectionGetSIMStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetSIMStatus called");
    return NULL;
}

void* _CTServerConnectionGetSIMTrayStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetSIMTrayStatus called");
    return NULL;
}

void* _CTServerConnectionGetShowUsageAlert(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetShowUsageAlert called");
    return NULL;
}

void* _CTServerConnectionGetSignalStrength(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetSignalStrength called");
    return NULL;
}

void* _CTServerConnectionGetSignalStrengthDisplayValue(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetSignalStrengthDisplayValue called");
    return NULL;
}

void* _CTServerConnectionGetSupportedEnhancedLinkQualityMetric(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetSupportedEnhancedLinkQualityMetric called");
    return NULL;
}

void* _CTServerConnectionGetTTYMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetTTYMode called");
    return NULL;
}

void* _CTServerConnectionGetTemperature(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetTemperature called");
    return NULL;
}

void* _CTServerConnectionGetTestModeFunction(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetTestModeFunction called");
    return NULL;
}

void* _CTServerConnectionGetThumperName(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetThumperName called");
    return NULL;
}

void* _CTServerConnectionGetTypeID(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetTypeID called");
    return NULL;
}

void* _CTServerConnectionGetUIConfiguredApns(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetUIConfiguredApns called");
    return NULL;
}

void* _CTServerConnectionGetUpdatedCarrierBundle(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetUpdatedCarrierBundle called");
    return NULL;
}

void* _CTServerConnectionGetUplinkFreq(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetUplinkFreq called");
    return NULL;
}

void* _CTServerConnectionGetVoiceLinkQualityMetric(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetVoiceLinkQualityMetric called");
    return NULL;
}

void* _CTServerConnectionGetVoiceMailAvailable(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetVoiceMailAvailable called");
    return NULL;
}

void* _CTServerConnectionGetVoiceMute(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetVoiceMute called");
    return NULL;
}

void* _CTServerConnectionGetVoicePrivacy(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetVoicePrivacy called");
    return NULL;
}

void* _CTServerConnectionGetXpcConnection(void)
{
    if (verbose) puts("STUB: _CTServerConnectionGetXpcConnection called");
    return NULL;
}

void* _CTServerConnectionHandleReply(void)
{
    if (verbose) puts("STUB: _CTServerConnectionHandleReply called");
    return NULL;
}

void* _CTServerConnectionHideDataIndicator(void)
{
    if (verbose) puts("STUB: _CTServerConnectionHideDataIndicator called");
    return NULL;
}

void* _CTServerConnectionHideRatIndicator(void)
{
    if (verbose) puts("STUB: _CTServerConnectionHideRatIndicator called");
    return NULL;
}

void* _CTServerConnectionICloudAccountReset(void)
{
    if (verbose) puts("STUB: _CTServerConnectionICloudAccountReset called");
    return NULL;
}

void* _CTServerConnectionInjectCellBroadcastMessage(void)
{
    if (verbose) puts("STUB: _CTServerConnectionInjectCellBroadcastMessage called");
    return NULL;
}

void* _CTServerConnectionInjectCellBroadcastMessageWithWac(void)
{
    if (verbose) puts("STUB: _CTServerConnectionInjectCellBroadcastMessageWithWac called");
    return NULL;
}

void* _CTServerConnectionInstallCarrierBundle(void)
{
    if (verbose) puts("STUB: _CTServerConnectionInstallCarrierBundle called");
    return NULL;
}

void* _CTServerConnectionIsAnyCallActiveOrHeld(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsAnyCallActiveOrHeld called");
    return NULL;
}

void* _CTServerConnectionIsCDMAInternationalRoamingSettingAllowed(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsCDMAInternationalRoamingSettingAllowed called");
    return NULL;
}

void* _CTServerConnectionIsDataAttached(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsDataAttached called");
    return NULL;
}

void* _CTServerConnectionIsDataAttachedForPreferredDataSIM(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsDataAttachedForPreferredDataSIM called");
    return NULL;
}

void* _CTServerConnectionIsDefaultConnection(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsDefaultConnection called");
    return NULL;
}

void* _CTServerConnectionIsEmergencyNumber(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsEmergencyNumber called");
    return NULL;
}

void* _CTServerConnectionIsEmergencyNumberWithWhitelistIncluded(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsEmergencyNumberWithWhitelistIncluded called");
    return NULL;
}

void* _CTServerConnectionIsEncryptedIdentitySupported(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsEncryptedIdentitySupported called");
    return NULL;
}

void* _CTServerConnectionIsInHomeCountry(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsInHomeCountry called");
    return NULL;
}

void* _CTServerConnectionIsInHomeCountryForPreferredDataSIM(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsInHomeCountryForPreferredDataSIM called");
    return NULL;
}

void* _CTServerConnectionIsPhoneNumberCredentialValid(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsPhoneNumberCredentialValid called");
    return NULL;
}

void* _CTServerConnectionIsPhoneNumberRegistrationSupported(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsPhoneNumberRegistrationSupported called");
    return NULL;
}

void* _CTServerConnectionIsServiceOverPublicInternet(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsServiceOverPublicInternet called");
    return NULL;
}

void* _CTServerConnectionIsSilenceAssertionBarred(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsSilenceAssertionBarred called");
    return NULL;
}

void* _CTServerConnectionIsUserIdentityModuleRequired(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsUserIdentityModuleRequired called");
    return NULL;
}

void* _CTServerConnectionIsValidEmergencyNumber(void)
{
    if (verbose) puts("STUB: _CTServerConnectionIsValidEmergencyNumber called");
    return NULL;
}

void* _CTServerConnectionKeepAliveAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionKeepAliveAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionLoadManagedCellularProfile(void)
{
    if (verbose) puts("STUB: _CTServerConnectionLoadManagedCellularProfile called");
    return NULL;
}

void* _CTServerConnectionLockPostponementTicket(void)
{
    if (verbose) puts("STUB: _CTServerConnectionLockPostponementTicket called");
    return NULL;
}

void* _CTServerConnectionModifyAttachAPNSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionModifyAttachAPNSettings called");
    return NULL;
}

void* _CTServerConnectionNetworkMonitorCreateLogChannel(void)
{
    if (verbose) puts("STUB: _CTServerConnectionNetworkMonitorCreateLogChannel called");
    return NULL;
}

void* _CTServerConnectionNetworkSelectionMenuAvailable(void)
{
    if (verbose) puts("STUB: _CTServerConnectionNetworkSelectionMenuAvailable called");
    return NULL;
}

void* _CTServerConnectionOTAActivationAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionOTAActivationAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionPINUnlock(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPINUnlock called");
    return NULL;
}

void* _CTServerConnectionPUKUnlock(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPUKUnlock called");
    return NULL;
}

void* _CTServerConnectionPacketContextAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPacketContextAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionPerformInternalVinylOperation(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPerformInternalVinylOperation called");
    return NULL;
}

void* _CTServerConnectionPerformMigration(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPerformMigration called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesAccountStatusChanged(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesAccountStatusChanged called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesAddDevice(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesAddDevice called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesDisableDevice(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesDisableDevice called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesEnableDevice(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesEnableDevice called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesGetDeviceInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesGetDeviceInfo called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesGetDeviceList(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesGetDeviceList called");
    return NULL;
}

void* _CTServerConnectionPhoneServicesRemoveDevice(void)
{
    if (verbose) puts("STUB: _CTServerConnectionPhoneServicesRemoveDevice called");
    return NULL;
}

void* _CTServerConnectionQueryAccountInformation(void)
{
    if (verbose) puts("STUB: _CTServerConnectionQueryAccountInformation called");
    return NULL;
}

void* _CTServerConnectionQueryEnhancedLinkQualityMetric(void)
{
    if (verbose) puts("STUB: _CTServerConnectionQueryEnhancedLinkQualityMetric called");
    return NULL;
}

void* _CTServerConnectionRegisterBlockForNotification(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRegisterBlockForNotification called");
    return NULL;
}

void* _CTServerConnectionRegisterForEvent(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRegisterForEvent called");
    return NULL;
}

void* _CTServerConnectionRegisterForNotification(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRegisterForNotification called");
    return NULL;
}

void* _CTServerConnectionRegisterSilentHoursCallback(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRegisterSilentHoursCallback called");
    return NULL;
}

void* _CTServerConnectionRegisterSupportedEnhancedLinkQualityMetric(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRegisterSupportedEnhancedLinkQualityMetric called");
    return NULL;
}

void* _CTServerConnectionRegistrationNotificationAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRegistrationNotificationAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionReleaseQOS(void)
{
    if (verbose) puts("STUB: _CTServerConnectionReleaseQOS called");
    return NULL;
}

void* _CTServerConnectionRemoveVictimFreq(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRemoveVictimFreq called");
    return NULL;
}

void* _CTServerConnectionRequestQOS(void)
{
    if (verbose) puts("STUB: _CTServerConnectionRequestQOS called");
    return NULL;
}

void* _CTServerConnectionResetCarrierBundle(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResetCarrierBundle called");
    return NULL;
}

void* _CTServerConnectionResetModem(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResetModem called");
    return NULL;
}

void* _CTServerConnectionResetModemWithCrashLogs(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResetModemWithCrashLogs called");
    return NULL;
}

void* _CTServerConnectionResetPacketContextSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResetPacketContextSettings called");
    return NULL;
}

void* _CTServerConnectionResetPacketContextStatistics(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResetPacketContextStatistics called");
    return NULL;
}

void* _CTServerConnectionResetUIConfiguredApns(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResetUIConfiguredApns called");
    return NULL;
}

void* _CTServerConnectionResumePacketContextSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionResumePacketContextSettings called");
    return NULL;
}

void* _CTServerConnectionSIMToolkitDisplayReady(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSIMToolkitDisplayReady called");
    return NULL;
}

void* _CTServerConnectionSavePhonebookEntry(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSavePhonebookEntry called");
    return NULL;
}

void* _CTServerConnectionSelectNetwork(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSelectNetwork called");
    return NULL;
}

void* _CTServerConnectionSelectPhonebook(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSelectPhonebook called");
    return NULL;
}

void* _CTServerConnectionSelectSIMToolkitListItem(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSelectSIMToolkitListItem called");
    return NULL;
}

void* _CTServerConnectionSelectSIMToolkitMenuItem(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSelectSIMToolkitMenuItem called");
    return NULL;
}

void* _CTServerConnectionSendAwdQueriableMetric(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendAwdQueriableMetric called");
    return NULL;
}

void* _CTServerConnectionSendBarrier(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendBarrier called");
    return NULL;
}

void* _CTServerConnectionSendEnhancedLinkQualityTrafficInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendEnhancedLinkQualityTrafficInfo called");
    return NULL;
}

void* _CTServerConnectionSendRemoteRestoreToDefault(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendRemoteRestoreToDefault called");
    return NULL;
}

void* _CTServerConnectionSendSIMToolkitBooleanResponse(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendSIMToolkitBooleanResponse called");
    return NULL;
}

void* _CTServerConnectionSendSIMToolkitResponse(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendSIMToolkitResponse called");
    return NULL;
}

void* _CTServerConnectionSendSIMToolkitStringResponse(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendSIMToolkitStringResponse called");
    return NULL;
}

void* _CTServerConnectionSendUSSDResponse(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendUSSDResponse called");
    return NULL;
}

void* _CTServerConnectionSendUserActivity(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSendUserActivity called");
    return NULL;
}

void* _CTServerConnectionSetActiveAudioSystemConfiguration(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetActiveAudioSystemConfiguration called");
    return NULL;
}

void* _CTServerConnectionSetAudioLoggingEnabled(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetAudioLoggingEnabled called");
    return NULL;
}

void* _CTServerConnectionSetAudioPath(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetAudioPath called");
    return NULL;
}

void* _CTServerConnectionSetBandInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetBandInfo called");
    return NULL;
}

void* _CTServerConnectionSetBasebandSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetBasebandSettings called");
    return NULL;
}

void* _CTServerConnectionSetCDMAInternationalRoaming(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCDMAInternationalRoaming called");
    return NULL;
}

void* _CTServerConnectionSetCampOnlyMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCampOnlyMode called");
    return NULL;
}

void* _CTServerConnectionSetCapability(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCapability called");
    return NULL;
}

void* _CTServerConnectionSetCapabilityExtended(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCapabilityExtended called");
    return NULL;
}

void* _CTServerConnectionSetCellBroadcastSettingForAlertType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellBroadcastSettingForAlertType called");
    return NULL;
}

void* _CTServerConnectionSetCellularDataIsDisallowed(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularDataIsDisallowed called");
    return NULL;
}

void* _CTServerConnectionSetCellularDataIsEnabled(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularDataIsEnabled called");
    return NULL;
}

void* _CTServerConnectionSetCellularDataIsEnabledEx(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularDataIsEnabledEx called");
    return NULL;
}

void* _CTServerConnectionSetCellularUsagePolicy(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularUsagePolicy called");
    return NULL;
}

void* _CTServerConnectionSetCellularUsagePolicyEx(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularUsagePolicyEx called");
    return NULL;
}

void* _CTServerConnectionSetCellularUsagePolicyMirroring(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularUsagePolicyMirroring called");
    return NULL;
}

void* _CTServerConnectionSetCellularUsagePolicyOsVersion(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCellularUsagePolicyOsVersion called");
    return NULL;
}

void* _CTServerConnectionSetChannelMute(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetChannelMute called");
    return NULL;
}

void* _CTServerConnectionSetCivicInfoBasedOnCurrentLocation(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetCivicInfoBasedOnCurrentLocation called");
    return NULL;
}

void* _CTServerConnectionSetDTMFType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetDTMFType called");
    return NULL;
}

void* _CTServerConnectionSetDefaultConnection(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetDefaultConnection called");
    return NULL;
}

void* _CTServerConnectionSetDisallowTelephonyFeature(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetDisallowTelephonyFeature called");
    return NULL;
}

void* _CTServerConnectionSetEMBMSActive(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetEMBMSActive called");
    return NULL;
}

void* _CTServerConnectionSetEMBMSSAIInterestedInfo(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetEMBMSSAIInterestedInfo called");
    return NULL;
}

void* _CTServerConnectionSetEnableOnlyHomeNetwork(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetEnableOnlyHomeNetwork called");
    return NULL;
}

void* _CTServerConnectionSetI2SFormat(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetI2SFormat called");
    return NULL;
}

void* _CTServerConnectionSetInternalManagedState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetInternalManagedState called");
    return NULL;
}

void* _CTServerConnectionSetInternationalDataAccessStatus(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetInternationalDataAccessStatus called");
    return NULL;
}

void* _CTServerConnectionSetMasterMute(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetMasterMute called");
    return NULL;
}

void* _CTServerConnectionSetMaxAllowedDataRate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetMaxAllowedDataRate called");
    return NULL;
}

void* _CTServerConnectionSetMaxTemperature(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetMaxTemperature called");
    return NULL;
}

void* _CTServerConnectionSetMaxTransmitPower(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetMaxTransmitPower called");
    return NULL;
}

void* _CTServerConnectionSetMixState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetMixState called");
    return NULL;
}

void* _CTServerConnectionSetOTAServerOverride(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetOTAServerOverride called");
    return NULL;
}

void* _CTServerConnectionSetPacketContextActive(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPacketContextActive called");
    return NULL;
}

void* _CTServerConnectionSetPacketContextActiveByServiceType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPacketContextActiveByServiceType called");
    return NULL;
}

void* _CTServerConnectionSetPacketNotificationFilter(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPacketNotificationFilter called");
    return NULL;
}

void* _CTServerConnectionSetPacketProtocolActiveByInterface(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPacketProtocolActiveByInterface called");
    return NULL;
}

void* _CTServerConnectionSetPacketProtocolActiveByServiceType(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPacketProtocolActiveByServiceType called");
    return NULL;
}

void* _CTServerConnectionSetPeriodicTemperatureUpdate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPeriodicTemperatureUpdate called");
    return NULL;
}

void* _CTServerConnectionSetPostponementTicket(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetPostponementTicket called");
    return NULL;
}

void* _CTServerConnectionSetRATSelection(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetRATSelection called");
    return NULL;
}

void* _CTServerConnectionSetReliableNetworkFallbackToCellular(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetReliableNetworkFallbackToCellular called");
    return NULL;
}

void* _CTServerConnectionSetShowUsageAlert(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetShowUsageAlert called");
    return NULL;
}

void* _CTServerConnectionSetSourceGain(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetSourceGain called");
    return NULL;
}

void* _CTServerConnectionSetTTYMode(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetTTYMode called");
    return NULL;
}

void* _CTServerConnectionSetTargetQueue(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetTargetQueue called");
    return NULL;
}

void* _CTServerConnectionSetTestModeFunction(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetTestModeFunction called");
    return NULL;
}

void* _CTServerConnectionSetTimeCacheValidityPeriod(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetTimeCacheValidityPeriod called");
    return NULL;
}

void* _CTServerConnectionSetTraceProperty(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetTraceProperty called");
    return NULL;
}

void* _CTServerConnectionSetUIConfiguredApns(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetUIConfiguredApns called");
    return NULL;
}

void* _CTServerConnectionSetUsageAlertParameters(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetUsageAlertParameters called");
    return NULL;
}

void* _CTServerConnectionSetVibratorState(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetVibratorState called");
    return NULL;
}

void* _CTServerConnectionSetVoiceMute(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSetVoiceMute called");
    return NULL;
}

void* _CTServerConnectionShouldDiscourageDisableLteDataRate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionShouldDiscourageDisableLteDataRate called");
    return NULL;
}

void* _CTServerConnectionShouldWarnDisabledLteMayImpactService(void)
{
    if (verbose) puts("STUB: _CTServerConnectionShouldWarnDisabledLteMayImpactService called");
    return NULL;
}

void* _CTServerConnectionSilenceAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSilenceAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionSimRefresh(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSimRefresh called");
    return NULL;
}

void* _CTServerConnectionStartAudioTone(void)
{
    if (verbose) puts("STUB: _CTServerConnectionStartAudioTone called");
    return NULL;
}

void* _CTServerConnectionStopAudioTone(void)
{
    if (verbose) puts("STUB: _CTServerConnectionStopAudioTone called");
    return NULL;
}

void* _CTServerConnectionSuspendPacketContextSettings(void)
{
    if (verbose) puts("STUB: _CTServerConnectionSuspendPacketContextSettings called");
    return NULL;
}

void* _CTServerConnectionTetheringAssertionCreate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionTetheringAssertionCreate called");
    return NULL;
}

void* _CTServerConnectionUIMAuthenticate(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUIMAuthenticate called");
    return NULL;
}

void* _CTServerConnectionUnloadManagedCellularProfile(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUnloadManagedCellularProfile called");
    return NULL;
}

void* _CTServerConnectionUnlockPostponementTicket(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUnlockPostponementTicket called");
    return NULL;
}

void* _CTServerConnectionUnregisterForAllNotifications(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUnregisterForAllNotifications called");
    return NULL;
}

void* _CTServerConnectionUnregisterForEvent(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUnregisterForEvent called");
    return NULL;
}

void* _CTServerConnectionUnregisterForNotification(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUnregisterForNotification called");
    return NULL;
}

void* _CTServerConnectionUnregisterSilentHoursCallback(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUnregisterSilentHoursCallback called");
    return NULL;
}

void* _CTServerConnectionUpdateAccountWithFeatureOrService(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUpdateAccountWithFeatureOrService called");
    return NULL;
}

void* _CTServerConnectionUpdateCarrierPushToken(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUpdateCarrierPushToken called");
    return NULL;
}

void* _CTServerConnectionUpdateProtocolQualityOfService(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUpdateProtocolQualityOfService called");
    return NULL;
}

void* _CTServerConnectionUpdateQualityOfService(void)
{
    if (verbose) puts("STUB: _CTServerConnectionUpdateQualityOfService called");
    return NULL;
}

void* _CTServerConnectionVerifyCarrierEntitlements(void)
{
    if (verbose) puts("STUB: _CTServerConnectionVerifyCarrierEntitlements called");
    return NULL;
}

void* _CTServerEnableFastDormancy(void)
{
    if (verbose) puts("STUB: _CTServerEnableFastDormancy called");
    return NULL;
}

void* _CTServerForceFastDormancy(void)
{
    if (verbose) puts("STUB: _CTServerForceFastDormancy called");
    return NULL;
}

void* _CTServerIsFastDormancyEnabled(void)
{
    if (verbose) puts("STUB: _CTServerIsFastDormancyEnabled called");
    return NULL;
}

void* _CTServerSetSupportsVoiceCall(void)
{
    if (verbose) puts("STUB: _CTServerSetSupportsVoiceCall called");
    return NULL;
}

void* _ConvertBinaryToHex(void)
{
    if (verbose) puts("STUB: _ConvertBinaryToHex called");
    return NULL;
}

void* _CreateDirectoriesToPath(void)
{
    if (verbose) puts("STUB: _CreateDirectoriesToPath called");
    return NULL;
}

void* _GetMobileUserDirectory(void)
{
    if (verbose) puts("STUB: _GetMobileUserDirectory called");
    return NULL;
}

void* _GetMobileUserGID(void)
{
    if (verbose) puts("STUB: _GetMobileUserGID called");
    return NULL;
}

void* _GetMobileUserUID(void)
{
    if (verbose) puts("STUB: _GetMobileUserUID called");
    return NULL;
}

void* _GetWirelessUserDirectory(void)
{
    if (verbose) puts("STUB: _GetWirelessUserDirectory called");
    return NULL;
}

void* _GetWirelessUserGID(void)
{
    if (verbose) puts("STUB: _GetWirelessUserGID called");
    return NULL;
}

void* _GetWirelessUserUID(void)
{
    if (verbose) puts("STUB: _GetWirelessUserUID called");
    return NULL;
}

void* _SynchronizeCommCenterPreferences(void)
{
    if (verbose) puts("STUB: _SynchronizeCommCenterPreferences called");
    return NULL;
}

void* _Z10asCFString24EntitlementAuthTokenType(void)
{
    if (verbose) puts("STUB: _Z10asCFString24EntitlementAuthTokenType called");
    return NULL;
}

void* _Z10createPLMNRKNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEES7_(void)
{
    if (verbose) puts("STUB: _Z10createPLMNRKNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEES7_ called");
    return NULL;
}

void* _Z10createPLMNii(void)
{
    if (verbose) puts("STUB: _Z10createPLMNii called");
    return NULL;
}

void* _Z10createPLMNiib(void)
{
    if (verbose) puts("STUB: _Z10createPLMNiib called");
    return NULL;
}

void* _Z12createCFPLMNRK3MCCRK3MNC(void)
{
    if (verbose) puts("STUB: _Z12createCFPLMNRK3MCCRK3MNC called");
    return NULL;
}

void* _Z15TTYTypeAsString7TTYType(void)
{
    if (verbose) puts("STUB: _Z15TTYTypeAsString7TTYType called");
    return NULL;
}

void* _Z17isEmergencyNumberRKNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _Z17isEmergencyNumberRKNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE called");
    return NULL;
}

void* _Z18EmCallTypeAsString23CTCallEmergencyCallType(void)
{
    if (verbose) puts("STUB: _Z18EmCallTypeAsString23CTCallEmergencyCallType called");
    return NULL;
}

void* _Z22CTGetRootVersionStringv(void)
{
    if (verbose) puts("STUB: _Z22CTGetRootVersionStringv called");
    return NULL;
}

void* _Z22callSourceModeAsString16CCCallSourceMode(void)
{
    if (verbose) puts("STUB: _Z22callSourceModeAsString16CCCallSourceMode called");
    return NULL;
}

void* _Z8asString20EntitlementTopicType(void)
{
    if (verbose) puts("STUB: _Z8asString20EntitlementTopicType called");
    return NULL;
}

void* _Z8asString24EntitlementAuthTokenType(void)
{
    if (verbose) puts("STUB: _Z8asString24EntitlementAuthTokenType called");
    return NULL;
}

void* _Z8asStringN19NetworkNotification16NotificationTypeE(void)
{
    if (verbose) puts("STUB: _Z8asStringN19NetworkNotification16NotificationTypeE called");
    return NULL;
}

void* _Z8formPLMNRK3MCCRK3MNC(void)
{
    if (verbose) puts("STUB: _Z8formPLMNRK3MCCRK3MNC called");
    return NULL;
}

void* _ZN14CSIPhoneNumber13setBaseNumberEPKc(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber13setBaseNumberEPKc called");
    return NULL;
}

void* _ZN14CSIPhoneNumber13setBaseNumberERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber13setBaseNumberERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN14CSIPhoneNumber13setDtmfStringERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber13setDtmfStringERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN14CSIPhoneNumber15setTypeOfNumberE15PhoneNumberType(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber15setTypeOfNumberE15PhoneNumberType called");
    return NULL;
}

void* _ZN14CSIPhoneNumber16setTypeOfAddressEi(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber16setTypeOfAddressEi called");
    return NULL;
}

void* _ZN14CSIPhoneNumber19setCLIRRequestStateE16CLIRRequestState(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber19setCLIRRequestStateE16CLIRRequestState called");
    return NULL;
}

void* _ZN14CSIPhoneNumber20setEmergencyCategoryEj(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber20setEmergencyCategoryEj called");
    return NULL;
}

void* _ZN14CSIPhoneNumber22setIsListedAsEmergencyEb(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber22setIsListedAsEmergencyEb called");
    return NULL;
}

void* _ZN14CSIPhoneNumber23setIsEmSMSTextSupportedEb(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber23setIsEmSMSTextSupportedEb called");
    return NULL;
}

void* _ZN14CSIPhoneNumber27convertLettersToPhoneNumberEv(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumber27convertLettersToPhoneNumberEv called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC1EPKc(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC1EPKc called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE15PhoneNumberType(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE15PhoneNumberType called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC1EiRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC1EiRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC1Ev(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC1Ev called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC2EPKc(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC2EPKc called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE15PhoneNumberType(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE15PhoneNumberType called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC2EiRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC2EiRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN14CSIPhoneNumberC2Ev(void)
{
    if (verbose) puts("STUB: _ZN14CSIPhoneNumberC2Ev called");
    return NULL;
}

void* _ZN19NetworkNotification14_ConvertToBoolEPK10__CFStringRPKv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification14_ConvertToBoolEPK10__CFStringRPKv called");
    return NULL;
}

void* _ZN19NetworkNotification14_ConvertToDateEPK10__CFStringRPKv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification14_ConvertToDateEPK10__CFStringRPKv called");
    return NULL;
}

void* _ZN19NetworkNotification14setValueForKeyEPK10__CFStringS2_NS_24NotificationTrackChangesE(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification14setValueForKeyEPK10__CFStringS2_NS_24NotificationTrackChangesE called");
    return NULL;
}

void* _ZN19NetworkNotification15_GetTypeForNameEPK10__CFString(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification15_GetTypeForNameEPK10__CFString called");
    return NULL;
}

void* _ZN19NetworkNotification16_ConvertToNumberEPK10__CFStringRPKv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification16_ConvertToNumberEPK10__CFStringRPKv called");
    return NULL;
}

void* _ZN19NetworkNotification18notificationStringEv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification18notificationStringEv called");
    return NULL;
}

void* _ZN19NetworkNotification18parsedNotificationEv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification18parsedNotificationEv called");
    return NULL;
}

void* _ZN19NetworkNotification21_GetFinalFormForValueEPK10__CFStringS2_RPKv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification21_GetFinalFormForValueEPK10__CFStringS2_RPKv called");
    return NULL;
}

void* _ZN19NetworkNotification24_parseNotificationStringEv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification24_parseNotificationStringEv called");
    return NULL;
}

void* _ZN19NetworkNotification30_NotificationKeyIsValidForTypeEPK10__CFString(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification30_NotificationKeyIsValidForTypeEPK10__CFString called");
    return NULL;
}

void* _ZN19NetworkNotification3urlEv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification3urlEv called");
    return NULL;
}

void* _ZN19NetworkNotification4nameEv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification4nameEv called");
    return NULL;
}

void* _ZN19NetworkNotification4typeEv(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotification4typeEv called");
    return NULL;
}

void* _ZN19NetworkNotificationC1EPK10__CFString(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotificationC1EPK10__CFString called");
    return NULL;
}

void* _ZN19NetworkNotificationC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotificationC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN19NetworkNotificationC2EPK10__CFString(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotificationC2EPK10__CFString called");
    return NULL;
}

void* _ZN19NetworkNotificationC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotificationC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN19NetworkNotificationD1Ev(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotificationD1Ev called");
    return NULL;
}

void* _ZN19NetworkNotificationD2Ev(void)
{
    if (verbose) puts("STUB: _ZN19NetworkNotificationD2Ev called");
    return NULL;
}

void* _ZN3MCC5clearEv(void)
{
    if (verbose) puts("STUB: _ZN3MCC5clearEv called");
    return NULL;
}

void* _ZN3MCCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MCCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MCCC1Et(void)
{
    if (verbose) puts("STUB: _ZN3MCCC1Et called");
    return NULL;
}

void* _ZN3MCCC1EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MCCC1EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MCCC1Ev(void)
{
    if (verbose) puts("STUB: _ZN3MCCC1Ev called");
    return NULL;
}

void* _ZN3MCCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MCCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MCCC2Et(void)
{
    if (verbose) puts("STUB: _ZN3MCCC2Et called");
    return NULL;
}

void* _ZN3MCCC2EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MCCC2EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MCCC2Ev(void)
{
    if (verbose) puts("STUB: _ZN3MCCC2Ev called");
    return NULL;
}

void* _ZN3MCCaSERKS_(void)
{
    if (verbose) puts("STUB: _ZN3MCCaSERKS_ called");
    return NULL;
}

void* _ZN3MNCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MNCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MNCC1EtRK3MCC(void)
{
    if (verbose) puts("STUB: _ZN3MNCC1EtRK3MCC called");
    return NULL;
}

void* _ZN3MNCC1EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MNCC1EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MNCC1Ev(void)
{
    if (verbose) puts("STUB: _ZN3MNCC1Ev called");
    return NULL;
}

void* _ZN3MNCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MNCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MNCC2EtRK3MCC(void)
{
    if (verbose) puts("STUB: _ZN3MNCC2EtRK3MCC called");
    return NULL;
}

void* _ZN3MNCC2EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN3MNCC2EtRKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN3MNCC2Ev(void)
{
    if (verbose) puts("STUB: _ZN3MNCC2Ev called");
    return NULL;
}

void* _ZN9MCCAndMNCC1ERK3MCCRK3MNC(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC1ERK3MCCRK3MNC called");
    return NULL;
}

void* _ZN9MCCAndMNCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN9MCCAndMNCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC1ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_ called");
    return NULL;
}

void* _ZN9MCCAndMNCC1Ett(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC1Ett called");
    return NULL;
}

void* _ZN9MCCAndMNCC1Ev(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC1Ev called");
    return NULL;
}

void* _ZN9MCCAndMNCC2ERK3MCCRK3MNC(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC2ERK3MCCRK3MNC called");
    return NULL;
}

void* _ZN9MCCAndMNCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZN9MCCAndMNCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC2ERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_ called");
    return NULL;
}

void* _ZN9MCCAndMNCC2Ett(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC2Ett called");
    return NULL;
}

void* _ZN9MCCAndMNCC2Ev(void)
{
    if (verbose) puts("STUB: _ZN9MCCAndMNCC2Ev called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber12getFormattedEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber12getFormattedEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber13getBaseNumberEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber13getBaseNumberEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber13getDtmfStringEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber13getDtmfStringEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber13getFullNumberEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber13getFullNumberEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber15getTypeOfNumberEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber15getTypeOfNumberEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber16getTypeOfAddressEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber16getTypeOfAddressEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber17hasSameBaseNumberERKS_(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber17hasSameBaseNumberERKS_ called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber19getCLIRRequestStateEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber19getCLIRRequestStateEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber19getFullNumberHelperENSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber19getFullNumberHelperENSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber20getEmergencyCategoryEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber20getEmergencyCategoryEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber21getFullOriginalNumberEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber21getFullOriginalNumberEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber22getIsListedAsEmergencyEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber22getIsListedAsEmergencyEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber23getIsEmSMSTextSupportedEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber23getIsEmSMSTextSupportedEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber23hasEmergencyCategorySetEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber23hasEmergencyCategorySetEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber26getBaseNumberNoPauseDigitsEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber26getBaseNumberNoPauseDigitsEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber26getFullNumberNoPauseDigitsEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber26getFullNumberNoPauseDigitsEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber8getIsMMIEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber8getIsMMIEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumber9getIsUSSDEv(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumber9getIsUSSDEv called");
    return NULL;
}

void* _ZNK14CSIPhoneNumbereqERKS_(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumbereqERKS_ called");
    return NULL;
}

void* _ZNK14CSIPhoneNumberneERKS_(void)
{
    if (verbose) puts("STUB: _ZNK14CSIPhoneNumberneERKS_ called");
    return NULL;
}

void* _ZNK19NetworkNotification24copyUnparsedNotificationEv(void)
{
    if (verbose) puts("STUB: _ZNK19NetworkNotification24copyUnparsedNotificationEv called");
    return NULL;
}

void* _ZNK3MCC11getIntValueEv(void)
{
    if (verbose) puts("STUB: _ZNK3MCC11getIntValueEv called");
    return NULL;
}

void* _ZNK3MCC14getStringValueEv(void)
{
    if (verbose) puts("STUB: _ZNK3MCC14getStringValueEv called");
    return NULL;
}

void* _ZNK3MCC5validEv(void)
{
    if (verbose) puts("STUB: _ZNK3MCC5validEv called");
    return NULL;
}

void* _ZNK3MCCeqERKS_(void)
{
    if (verbose) puts("STUB: _ZNK3MCCeqERKS_ called");
    return NULL;
}

void* _ZNK3MCCgtERKS_(void)
{
    if (verbose) puts("STUB: _ZNK3MCCgtERKS_ called");
    return NULL;
}

void* _ZNK3MCCltERKS_(void)
{
    if (verbose) puts("STUB: _ZNK3MCCltERKS_ called");
    return NULL;
}

void* _ZNK3MCCneERKS_(void)
{
    if (verbose) puts("STUB: _ZNK3MCCneERKS_ called");
    return NULL;
}

void* _ZNK3MNC12isThreeDigitEv(void)
{
    if (verbose) puts("STUB: _ZNK3MNC12isThreeDigitEv called");
    return NULL;
}

void* _ZNK3MNC15getIntegerWidthEv(void)
{
    if (verbose) puts("STUB: _ZNK3MNC15getIntegerWidthEv called");
    return NULL;
}

void* _ZNK9MCCAndMNC14getStringValueEv(void)
{
    if (verbose) puts("STUB: _ZNK9MCCAndMNC14getStringValueEv called");
    return NULL;
}

void* _ZNK9MCCAndMNC6getMccEv(void)
{
    if (verbose) puts("STUB: _ZNK9MCCAndMNC6getMccEv called");
    return NULL;
}

void* _ZNK9MCCAndMNC6getMncEv(void)
{
    if (verbose) puts("STUB: _ZNK9MCCAndMNC6getMncEv called");
    return NULL;
}

void* _ZNK9MCCAndMNCeqERKS_(void)
{
    if (verbose) puts("STUB: _ZNK9MCCAndMNCeqERKS_ called");
    return NULL;
}

void* _ZNK9MCCAndMNCltERKS_(void)
{
    if (verbose) puts("STUB: _ZNK9MCCAndMNCltERKS_ called");
    return NULL;
}

void* _ZNK9MCCAndMNCneERKS_(void)
{
    if (verbose) puts("STUB: _ZNK9MCCAndMNCneERKS_ called");
    return NULL;
}

void* _ZlsRNSt3__113basic_ostreamIcNS_11char_traitsIcEEEERK9MCCAndMNC(void)
{
    if (verbose) puts("STUB: _ZlsRNSt3__113basic_ostreamIcNS_11char_traitsIcEEEERK9MCCAndMNC called");
    return NULL;
}

void* _is3DigitMNCWithIMSI(void)
{
    if (verbose) puts("STUB: _is3DigitMNCWithIMSI called");
    return NULL;
}

void* _is3DigitMNCWithMCC(void)
{
    if (verbose) puts("STUB: _is3DigitMNCWithMCC called");
    return NULL;
}

void* asString(void)
{
    if (verbose) puts("STUB: asString called");
    return NULL;
}

void* createUuidBytes(void)
{
    if (verbose) puts("STUB: createUuidBytes called");
    return NULL;
}

void* getClientKeyForEntitlementType(void)
{
    if (verbose) puts("STUB: getClientKeyForEntitlementType called");
    return NULL;
}

void* getEntitlementTypeFromClientKey(void)
{
    if (verbose) puts("STUB: getEntitlementTypeFromClientKey called");
    return NULL;
}

void* kCTDataConnectionServiceTypeFromIndex(void)
{
    if (verbose) puts("STUB: kCTDataConnectionServiceTypeFromIndex called");
    return NULL;
}

void* kCTDataConnectionTypeFromString(void)
{
    if (verbose) puts("STUB: kCTDataConnectionTypeFromString called");
    return NULL;
}

void* radioStateAsString(void)
{
    if (verbose) puts("STUB: radioStateAsString called");
    return NULL;
}
