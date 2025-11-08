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


#include <CoreWLAN/CoreWLAN.h>
#include <stdlib.h>
#include <stdio.h>

NSString *const kCWScanKeyMerge = @"SCAN_MERGE";

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CWAddNetworkProfile(void)
{
    if (verbose) puts("STUB: CWAddNetworkProfile called");
    return NULL;
}

void* CWAddNetworkProfileToBottom(void)
{
    if (verbose) puts("STUB: CWAddNetworkProfileToBottom called");
    return NULL;
}

void* CWAddPasspointNetworkProfile(void)
{
    if (verbose) puts("STUB: CWAddPasspointNetworkProfile called");
    return NULL;
}

void* CWAddPasspointNetworkProfileToBottom(void)
{
    if (verbose) puts("STUB: CWAddPasspointNetworkProfileToBottom called");
    return NULL;
}

void* CWAuthorizeRight(void)
{
    if (verbose) puts("STUB: CWAuthorizeRight called");
    return NULL;
}

void* CWClearRecoveryNetworksFromNVRAM(void)
{
    if (verbose) puts("STUB: CWClearRecoveryNetworksFromNVRAM called");
    return NULL;
}

void* CWConfigurationAddNetworkProfile(void)
{
    if (verbose) puts("STUB: CWConfigurationAddNetworkProfile called");
    return NULL;
}

void* CWConfigurationRemoveNetworkProfile(void)
{
    if (verbose) puts("STUB: CWConfigurationRemoveNetworkProfile called");
    return NULL;
}

void* CWConfigurationSetPreferredNetworkProfile(void)
{
    if (verbose) puts("STUB: CWConfigurationSetPreferredNetworkProfile called");
    return NULL;
}

void* CWCurrentTimestamp(void)
{
    if (verbose) puts("STUB: CWCurrentTimestamp called");
    return NULL;
}

void* CWDescriptionForAWDLSuspendReason(void)
{
    if (verbose) puts("STUB: CWDescriptionForAWDLSuspendReason called");
    return NULL;
}

void* CWDescriptionForAutoJoinDeferralReason(void)
{
    if (verbose) puts("STUB: CWDescriptionForAutoJoinDeferralReason called");
    return NULL;
}

void* CWDescriptionForAutoJoinTrigger(void)
{
    if (verbose) puts("STUB: CWDescriptionForAutoJoinTrigger called");
    return NULL;
}

void* CWDescriptionForXPCErrorCode(void)
{
    if (verbose) puts("STUB: CWDescriptionForXPCErrorCode called");
    return NULL;
}

void* CWDumpTemporaryLog(void)
{
    if (verbose) puts("STUB: CWDumpTemporaryLog called");
    return NULL;
}

void* CWEAPIdentityCreateData(void)
{
    if (verbose) puts("STUB: CWEAPIdentityCreateData called");
    return NULL;
}

void* CWEAPIdentityCreateFromData(void)
{
    if (verbose) puts("STUB: CWEAPIdentityCreateFromData called");
    return NULL;
}

void* CWFindNetworkProfile(void)
{
    if (verbose) puts("STUB: CWFindNetworkProfile called");
    return NULL;
}

void* CWFindNetworkProfileWithID(void)
{
    if (verbose) puts("STUB: CWFindNetworkProfileWithID called");
    return NULL;
}

void* CWFindPasspointNetworkProfile(void)
{
    if (verbose) puts("STUB: CWFindPasspointNetworkProfile called");
    return NULL;
}

void* CWHashedSSIDUsingDeviceID(void)
{
    if (verbose) puts("STUB: CWHashedSSIDUsingDeviceID called");
    return NULL;
}

void* CWHashedWPAPassword(void)
{
    if (verbose) puts("STUB: CWHashedWPAPassword called");
    return NULL;
}

void* CWHexadecimalStringFromData(void)
{
    if (verbose) puts("STUB: CWHexadecimalStringFromData called");
    return NULL;
}

void* CWIEEE80211GASStatusToString(void)
{
    if (verbose) puts("STUB: CWIEEE80211GASStatusToString called");
    return NULL;
}

void* CWInterfaceStartAutoJoin(void)
{
    if (verbose) puts("STUB: CWInterfaceStartAutoJoin called");
    return NULL;
}

void* CWInterfaceStartHostAPMode(void)
{
    if (verbose) puts("STUB: CWInterfaceStartHostAPMode called");
    return NULL;
}

void* CWInterfaceStopHostAPMode(void)
{
    if (verbose) puts("STUB: CWInterfaceStopHostAPMode called");
    return NULL;
}

void* CWInterfaceSystemMode8021XAssociate(void)
{
    if (verbose) puts("STUB: CWInterfaceSystemMode8021XAssociate called");
    return NULL;
}

void* CWInterfaceSystemModePSKAssociate(void)
{
    if (verbose) puts("STUB: CWInterfaceSystemModePSKAssociate called");
    return NULL;
}

void* CWInternetSharingCopyConfiguration(void)
{
    if (verbose) puts("STUB: CWInternetSharingCopyConfiguration called");
    return NULL;
}

void* CWInternetSharingSetConfiguration(void)
{
    if (verbose) puts("STUB: CWInternetSharingSetConfiguration called");
    return NULL;
}

void* CWIsInstallEnvironment(void)
{
    if (verbose) puts("STUB: CWIsInstallEnvironment called");
    return NULL;
}

void* CWIsWiFiCloudSyncEngineRunning(void)
{
    if (verbose) puts("STUB: CWIsWiFiCloudSyncEngineRunning called");
    return NULL;
}

void* CWKeychainCopyEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainCopyEAPIdentity called");
    return NULL;
}

void* CWKeychainCopyEAPIdentityList(void)
{
    if (verbose) puts("STUB: CWKeychainCopyEAPIdentityList called");
    return NULL;
}

void* CWKeychainCopyEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainCopyEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainCopyPasspointEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainCopyPasspointEAPIdentity called");
    return NULL;
}

void* CWKeychainCopyPasspointEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainCopyPasspointEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainCopyPassword(void)
{
    if (verbose) puts("STUB: CWKeychainCopyPassword called");
    return NULL;
}

void* CWKeychainCopyWiFiEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainCopyWiFiEAPIdentity called");
    return NULL;
}

void* CWKeychainCopyWiFiPasspointEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainCopyWiFiPasspointEAPIdentity called");
    return NULL;
}

void* CWKeychainDeleteEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeleteEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainDeleteHostAPModePassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeleteHostAPModePassword called");
    return NULL;
}

void* CWKeychainDeletePasspointEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeletePasspointEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainDeletePassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeletePassword called");
    return NULL;
}

void* CWKeychainDeleteWiFiEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeleteWiFiEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainDeleteWiFiPasspointEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeleteWiFiPasspointEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainDeleteWiFiPassword(void)
{
    if (verbose) puts("STUB: CWKeychainDeleteWiFiPassword called");
    return NULL;
}

void* CWKeychainDeleteWiFiPassword_10_8(void)
{
    if (verbose) puts("STUB: CWKeychainDeleteWiFiPassword_10_8 called");
    return NULL;
}

void* CWKeychainFindHostAPModePassword(void)
{
    if (verbose) puts("STUB: CWKeychainFindHostAPModePassword called");
    return NULL;
}

void* CWKeychainFindPassword(void)
{
    if (verbose) puts("STUB: CWKeychainFindPassword called");
    return NULL;
}

void* CWKeychainFindWiFiEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainFindWiFiEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainFindWiFiPasspointEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainFindWiFiPasspointEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainFindWiFiPassword(void)
{
    if (verbose) puts("STUB: CWKeychainFindWiFiPassword called");
    return NULL;
}

void* CWKeychainFindWiFiPassword_10_8(void)
{
    if (verbose) puts("STUB: CWKeychainFindWiFiPassword_10_8 called");
    return NULL;
}

void* CWKeychainSetEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainSetEAPIdentity called");
    return NULL;
}

void* CWKeychainSetEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainSetHostAPModePassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetHostAPModePassword called");
    return NULL;
}

void* CWKeychainSetPasspointEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainSetPasspointEAPIdentity called");
    return NULL;
}

void* CWKeychainSetPasspointEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetPasspointEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainSetPassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetPassword called");
    return NULL;
}

void* CWKeychainSetWiFiEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainSetWiFiEAPIdentity called");
    return NULL;
}

void* CWKeychainSetWiFiEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetWiFiEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainSetWiFiPasspointEAPIdentity(void)
{
    if (verbose) puts("STUB: CWKeychainSetWiFiPasspointEAPIdentity called");
    return NULL;
}

void* CWKeychainSetWiFiPasspointEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetWiFiPasspointEAPUsernameAndPassword called");
    return NULL;
}

void* CWKeychainSetWiFiPassword(void)
{
    if (verbose) puts("STUB: CWKeychainSetWiFiPassword called");
    return NULL;
}

void* CWKeychainSetWiFiPassword_10_8(void)
{
    if (verbose) puts("STUB: CWKeychainSetWiFiPassword_10_8 called");
    return NULL;
}

void* CWKeychainUpdateWiFiKeychainItems(void)
{
    if (verbose) puts("STUB: CWKeychainUpdateWiFiKeychainItems called");
    return NULL;
}

void* CWLookupLocalizedAccessNetworkType(void)
{
    if (verbose) puts("STUB: CWLookupLocalizedAccessNetworkType called");
    return NULL;
}

void* CWLookupLocalizedNetworkAuthTypeIndicator(void)
{
    if (verbose) puts("STUB: CWLookupLocalizedNetworkAuthTypeIndicator called");
    return NULL;
}

void* CWLookupLocalizedVenueGroupAndType(void)
{
    if (verbose) puts("STUB: CWLookupLocalizedVenueGroupAndType called");
    return NULL;
}

void* CWMachineSerialNumber(void)
{
    if (verbose) puts("STUB: CWMachineSerialNumber called");
    return NULL;
}

void* CWMergeNetworks(void)
{
    if (verbose) puts("STUB: CWMergeNetworks called");
    return NULL;
}

void* CWPreAuthorizeRight(void)
{
    if (verbose) puts("STUB: CWPreAuthorizeRight called");
    return NULL;
}

void* CWReadRecoveryNetworksFromNVRAM(void)
{
    if (verbose) puts("STUB: CWReadRecoveryNetworksFromNVRAM called");
    return NULL;
}

void* CWRemoveCloudWiFiProfile(void)
{
    if (verbose) puts("STUB: CWRemoveCloudWiFiProfile called");
    return NULL;
}

void* CWRemoveNetworkProfile(void)
{
    if (verbose) puts("STUB: CWRemoveNetworkProfile called");
    return NULL;
}

void* CWRemoveNetworkProfileWithID(void)
{
    if (verbose) puts("STUB: CWRemoveNetworkProfileWithID called");
    return NULL;
}

void* CWRemovePasspointNetworkProfile(void)
{
    if (verbose) puts("STUB: CWRemovePasspointNetworkProfile called");
    return NULL;
}

void* CWSynchronizeCloudWiFiProfiles(void)
{
    if (verbose) puts("STUB: CWSynchronizeCloudWiFiProfiles called");
    return NULL;
}

void* CWSystemKeychainDeleteEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainDeleteEAPUsernameAndPassword called");
    return NULL;
}

void* CWSystemKeychainDeleteHostAPModePassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainDeleteHostAPModePassword called");
    return NULL;
}

void* CWSystemKeychainDeletePassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainDeletePassword called");
    return NULL;
}

void* CWSystemKeychainFindPassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainFindPassword called");
    return NULL;
}

void* CWSystemKeychainSetEAPIdentity(void)
{
    if (verbose) puts("STUB: CWSystemKeychainSetEAPIdentity called");
    return NULL;
}

void* CWSystemKeychainSetEAPUsernameAndPassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainSetEAPUsernameAndPassword called");
    return NULL;
}

void* CWSystemKeychainSetHostAPModePassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainSetHostAPModePassword called");
    return NULL;
}

void* CWSystemKeychainSetPassword(void)
{
    if (verbose) puts("STUB: CWSystemKeychainSetPassword called");
    return NULL;
}

void* CWUpdateNetworkProfile(void)
{
    if (verbose) puts("STUB: CWUpdateNetworkProfile called");
    return NULL;
}

void* CWUpdateNetworkProfileWithID(void)
{
    if (verbose) puts("STUB: CWUpdateNetworkProfileWithID called");
    return NULL;
}

void* CWUpdatePasspointNetworkProfile(void)
{
    if (verbose) puts("STUB: CWUpdatePasspointNetworkProfile called");
    return NULL;
}

void* CWValidLengthComputerNameSSIDWithEncoding(void)
{
    if (verbose) puts("STUB: CWValidLengthComputerNameSSIDWithEncoding called");
    return NULL;
}

void* CWValidLengthSSIDWithEncoding(void)
{
    if (verbose) puts("STUB: CWValidLengthSSIDWithEncoding called");
    return NULL;
}

void* WiFiDeviceClientCopyCurrentNetwork(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientCopyCurrentNetwork called");
    return NULL;
}

void* WiFiDeviceClientCopyProperty(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientCopyProperty called");
    return NULL;
}

void* WiFiDeviceClientDisassociate(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientDisassociate called");
    return NULL;
}

void* WiFiDeviceClientGetInterfaceName(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientGetInterfaceName called");
    return NULL;
}

void* WiFiDeviceClientGetPower(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientGetPower called");
    return NULL;
}

void* WiFiDeviceClientIsInterfaceAWDL(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientIsInterfaceAWDL called");
    return NULL;
}

void* WiFiDeviceClientRegisterDeviceAvailableCallback(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientRegisterDeviceAvailableCallback called");
    return NULL;
}

void* WiFiDeviceClientRegisterExtendedLinkCallback(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientRegisterExtendedLinkCallback called");
    return NULL;
}

void* WiFiDeviceClientRegisterPowerCallback(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientRegisterPowerCallback called");
    return NULL;
}

void* WiFiDeviceClientRegisterVirtualInterfaceStateChangeCallback(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientRegisterVirtualInterfaceStateChangeCallback called");
    return NULL;
}

void* WiFiDeviceClientScanAsync(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientScanAsync called");
    return NULL;
}

void* WiFiDeviceClientTrafficRegistration(void)
{
    if (verbose) puts("STUB: WiFiDeviceClientTrafficRegistration called");
    return NULL;
}

void* WiFiGetNetworkChannel(void)
{
    if (verbose) puts("STUB: WiFiGetNetworkChannel called");
    return NULL;
}

void* WiFiManagerClientCopyDevices(void)
{
    if (verbose) puts("STUB: WiFiManagerClientCopyDevices called");
    return NULL;
}

void* WiFiManagerClientCopyInterfaces(void)
{
    if (verbose) puts("STUB: WiFiManagerClientCopyInterfaces called");
    return NULL;
}

void* WiFiManagerClientCopyNetworks(void)
{
    if (verbose) puts("STUB: WiFiManagerClientCopyNetworks called");
    return NULL;
}

void* WiFiManagerClientCreate(void)
{
    if (verbose) puts("STUB: WiFiManagerClientCreate called");
    return NULL;
}

void* WiFiManagerClientDisableNetwork(void)
{
    if (verbose) puts("STUB: WiFiManagerClientDisableNetwork called");
    return NULL;
}

void* WiFiManagerClientGetDevice(void)
{
    if (verbose) puts("STUB: WiFiManagerClientGetDevice called");
    return NULL;
}

void* WiFiManagerClientGetThermalIndex(void)
{
    if (verbose) puts("STUB: WiFiManagerClientGetThermalIndex called");
    return NULL;
}

void* WiFiManagerClientGetWoWCapability(void)
{
    if (verbose) puts("STUB: WiFiManagerClientGetWoWCapability called");
    return NULL;
}

void* WiFiManagerClientGetWoWState(void)
{
    if (verbose) puts("STUB: WiFiManagerClientGetWoWState called");
    return NULL;
}

void* WiFiManagerClientRegisterDeviceAttachmentCallback(void)
{
    if (verbose) puts("STUB: WiFiManagerClientRegisterDeviceAttachmentCallback called");
    return NULL;
}

void* WiFiManagerClientRegisterWowStateChangedCallback(void)
{
    if (verbose) puts("STUB: WiFiManagerClientRegisterWowStateChangedCallback called");
    return NULL;
}

void* WiFiManagerClientScheduleWithRunLoop(void)
{
    if (verbose) puts("STUB: WiFiManagerClientScheduleWithRunLoop called");
    return NULL;
}

void* WiFiManagerClientSetNetworkProperty(void)
{
    if (verbose) puts("STUB: WiFiManagerClientSetNetworkProperty called");
    return NULL;
}

void* WiFiManagerClientSetPower(void)
{
    if (verbose) puts("STUB: WiFiManagerClientSetPower called");
    return NULL;
}

void* WiFiManagerClientSetThermalIndex(void)
{
    if (verbose) puts("STUB: WiFiManagerClientSetThermalIndex called");
    return NULL;
}

void* WiFiManagerClientSetWoWState(void)
{
    if (verbose) puts("STUB: WiFiManagerClientSetWoWState called");
    return NULL;
}

void* WiFiManagerClientTemporarilyDisableNetwork(void)
{
    if (verbose) puts("STUB: WiFiManagerClientTemporarilyDisableNetwork called");
    return NULL;
}

void* WiFiManagerClientUnscheduleFromRunLoop(void)
{
    if (verbose) puts("STUB: WiFiManagerClientUnscheduleFromRunLoop called");
    return NULL;
}

void* WiFiManagerClientUpdateNetwork(void)
{
    if (verbose) puts("STUB: WiFiManagerClientUpdateNetwork called");
    return NULL;
}

void* WiFiManagerCopyDeviceList(void)
{
    if (verbose) puts("STUB: WiFiManagerCopyDeviceList called");
    return NULL;
}

void* WiFiNetworkCopyBSSIDData(void)
{
    if (verbose) puts("STUB: WiFiNetworkCopyBSSIDData called");
    return NULL;
}

void* WiFiNetworkCopyHashedWPAPassword(void)
{
    if (verbose) puts("STUB: WiFiNetworkCopyHashedWPAPassword called");
    return NULL;
}

void* WiFiNetworkCopyRecord(void)
{
    if (verbose) puts("STUB: WiFiNetworkCopyRecord called");
    return NULL;
}

void* WiFiNetworkCopyShareablePassword(void)
{
    if (verbose) puts("STUB: WiFiNetworkCopyShareablePassword called");
    return NULL;
}

void* WiFiNetworkCreate(void)
{
    if (verbose) puts("STUB: WiFiNetworkCreate called");
    return NULL;
}

void* WiFiNetworkCreateCopy(void)
{
    if (verbose) puts("STUB: WiFiNetworkCreateCopy called");
    return NULL;
}

void* WiFiNetworkGetChannel(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetChannel called");
    return NULL;
}

void* WiFiNetworkGetIntProperty(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetIntProperty called");
    return NULL;
}

void* WiFiNetworkGetOperatingBand(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetOperatingBand called");
    return NULL;
}

void* WiFiNetworkGetProperty(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetProperty called");
    return NULL;
}

void* WiFiNetworkGetSSID(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetSSID called");
    return NULL;
}

void* WiFiNetworkGetSSIDData(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetSSIDData called");
    return NULL;
}

void* WiFiNetworkGetShareMode(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetShareMode called");
    return NULL;
}

void* WiFiNetworkGetUserRole(void)
{
    if (verbose) puts("STUB: WiFiNetworkGetUserRole called");
    return NULL;
}

void* WiFiNetworkIsAdHoc(void)
{
    if (verbose) puts("STUB: WiFiNetworkIsAdHoc called");
    return NULL;
}

void* WiFiNetworkIsEAP(void)
{
    if (verbose) puts("STUB: WiFiNetworkIsEAP called");
    return NULL;
}

void* WiFiNetworkIsHotspot(void)
{
    if (verbose) puts("STUB: WiFiNetworkIsHotspot called");
    return NULL;
}

void* WiFiNetworkIsWEP(void)
{
    if (verbose) puts("STUB: WiFiNetworkIsWEP called");
    return NULL;
}

void* WiFiNetworkIsWPA(void)
{
    if (verbose) puts("STUB: WiFiNetworkIsWPA called");
    return NULL;
}

void* WiFiNetworkMerge(void)
{
    if (verbose) puts("STUB: WiFiNetworkMerge called");
    return NULL;
}

void* WiFiNetworkSetProperty(void)
{
    if (verbose) puts("STUB: WiFiNetworkSetProperty called");
    return NULL;
}

void* WiFiXPCEventProtocolInterface(void)
{
    if (verbose) puts("STUB: WiFiXPCEventProtocolInterface called");
    return NULL;
}

void* WiFiXPCRequestProtocolInterface(void)
{
    if (verbose) puts("STUB: WiFiXPCRequestProtocolInterface called");
    return NULL;
}

void* _CWCopyWiFiKeychainItem_10_9(void)
{
    if (verbose) puts("STUB: _CWCopyWiFiKeychainItem_10_9 called");
    return NULL;
}

void* _CWDeleteWiFiKeychainItem_10_9(void)
{
    if (verbose) puts("STUB: _CWDeleteWiFiKeychainItem_10_9 called");
    return NULL;
}

void* _CWFindWiFiKeychainItemPassword_10_9(void)
{
    if (verbose) puts("STUB: _CWFindWiFiKeychainItemPassword_10_9 called");
    return NULL;
}

void* _CWSetWiFiKeychainItem_10_9(void)
{
    if (verbose) puts("STUB: _CWSetWiFiKeychainItem_10_9 called");
    return NULL;
}

void* _EAPOLClientConfigurationCopyLoginWindowProfiles(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationCopyLoginWindowProfiles called");
    return NULL;
}

void* _EAPOLClientConfigurationCreate(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationCreate called");
    return NULL;
}

void* _EAPOLClientConfigurationGetDefaultAuthenticationProperties(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationGetDefaultAuthenticationProperties called");
    return NULL;
}

void* _EAPOLClientConfigurationGetProfileWithID(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationGetProfileWithID called");
    return NULL;
}

void* _EAPOLClientConfigurationGetProfileWithWLANDomain(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationGetProfileWithWLANDomain called");
    return NULL;
}

void* _EAPOLClientConfigurationGetProfileWithWLANSSID(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationGetProfileWithWLANSSID called");
    return NULL;
}

void* _EAPOLClientConfigurationGetSystemProfile(void)
{
    if (verbose) puts("STUB: _EAPOLClientConfigurationGetSystemProfile called");
    return NULL;
}

void* _EAPOLClientItemIDCopyIdentity(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDCopyIdentity called");
    return NULL;
}

void* _EAPOLClientItemIDCopyPasswordItem(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDCopyPasswordItem called");
    return NULL;
}

void* _EAPOLClientItemIDCreateWithProfile(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDCreateWithProfile called");
    return NULL;
}

void* _EAPOLClientItemIDCreateWithProfileID(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDCreateWithProfileID called");
    return NULL;
}

void* _EAPOLClientItemIDCreateWithWLANDomain(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDCreateWithWLANDomain called");
    return NULL;
}

void* _EAPOLClientItemIDCreateWithWLANSSID(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDCreateWithWLANSSID called");
    return NULL;
}

void* _EAPOLClientItemIDGetProfileID(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDGetProfileID called");
    return NULL;
}

void* _EAPOLClientItemIDGetWLANSSID(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDGetWLANSSID called");
    return NULL;
}

void* _EAPOLClientItemIDRemovePasswordItem(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDRemovePasswordItem called");
    return NULL;
}

void* _EAPOLClientItemIDSetIdentity(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDSetIdentity called");
    return NULL;
}

void* _EAPOLClientItemIDSetPasswordItem(void)
{
    if (verbose) puts("STUB: _EAPOLClientItemIDSetPasswordItem called");
    return NULL;
}

void* _EAPOLClientProfileCreate(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileCreate called");
    return NULL;
}

void* _EAPOLClientProfileCreatePropertyList(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileCreatePropertyList called");
    return NULL;
}

void* _EAPOLClientProfileCreateWithPropertyList(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileCreateWithPropertyList called");
    return NULL;
}

void* _EAPOLClientProfileGetAuthenticationProperties(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileGetAuthenticationProperties called");
    return NULL;
}

void* _EAPOLClientProfileGetID(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileGetID called");
    return NULL;
}

void* _EAPOLClientProfileGetUserDefinedName(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileGetUserDefinedName called");
    return NULL;
}

void* _EAPOLClientProfileGetWLANDomain(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileGetWLANDomain called");
    return NULL;
}

void* _EAPOLClientProfileGetWLANSSIDAndSecurityType(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileGetWLANSSIDAndSecurityType called");
    return NULL;
}

void* _EAPOLClientProfileSetAuthenticationProperties(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileSetAuthenticationProperties called");
    return NULL;
}

void* _EAPOLClientProfileSetWLANDomain(void)
{
    if (verbose) puts("STUB: _EAPOLClientProfileSetWLANDomain called");
    return NULL;
}

void* _EAPOLControlAnyInterfaceKeyCreate(void)
{
    if (verbose) puts("STUB: _EAPOLControlAnyInterfaceKeyCreate called");
    return NULL;
}

void* _EAPOLControlCopyLoginWindowClientItemID(void)
{
    if (verbose) puts("STUB: _EAPOLControlCopyLoginWindowClientItemID called");
    return NULL;
}

void* _EAPOLControlCopyLoginWindowConfiguration(void)
{
    if (verbose) puts("STUB: _EAPOLControlCopyLoginWindowConfiguration called");
    return NULL;
}

void* _EAPOLControlCopyLoginWindowProfile(void)
{
    if (verbose) puts("STUB: _EAPOLControlCopyLoginWindowProfile called");
    return NULL;
}

void* _EAPOLControlCopyStateAndStatus(void)
{
    if (verbose) puts("STUB: _EAPOLControlCopyStateAndStatus called");
    return NULL;
}

void* _EAPOLControlDidUserCancel(void)
{
    if (verbose) puts("STUB: _EAPOLControlDidUserCancel called");
    return NULL;
}

void* _EAPOLControlKeyCopyInterface(void)
{
    if (verbose) puts("STUB: _EAPOLControlKeyCopyInterface called");
    return NULL;
}

void* _EAPOLControlKeyCreate(void)
{
    if (verbose) puts("STUB: _EAPOLControlKeyCreate called");
    return NULL;
}

void* _EAPOLControlProvideUserInput(void)
{
    if (verbose) puts("STUB: _EAPOLControlProvideUserInput called");
    return NULL;
}

void* _EAPOLControlStart(void)
{
    if (verbose) puts("STUB: _EAPOLControlStart called");
    return NULL;
}

void* _EAPOLControlStartSystem(void)
{
    if (verbose) puts("STUB: _EAPOLControlStartSystem called");
    return NULL;
}

void* _EAPOLControlStartSystemWithClientItemID(void)
{
    if (verbose) puts("STUB: _EAPOLControlStartSystemWithClientItemID called");
    return NULL;
}

void* _EAPOLControlStartWithClientItemID(void)
{
    if (verbose) puts("STUB: _EAPOLControlStartWithClientItemID called");
    return NULL;
}

void* _EAPOLControlStop(void)
{
    if (verbose) puts("STUB: _EAPOLControlStop called");
    return NULL;
}

void* _EAPSecCertificateCopyAttributesDictionary(void)
{
    if (verbose) puts("STUB: _EAPSecCertificateCopyAttributesDictionary called");
    return NULL;
}

void* _EAPSecIdentityHandleCreate(void)
{
    if (verbose) puts("STUB: _EAPSecIdentityHandleCreate called");
    return NULL;
}

void* _EAPSecIdentityHandleCreateSecIdentity(void)
{
    if (verbose) puts("STUB: _EAPSecIdentityHandleCreateSecIdentity called");
    return NULL;
}

void* _EAPSecIdentityListCreate(void)
{
    if (verbose) puts("STUB: _EAPSecIdentityListCreate called");
    return NULL;
}

void* commitDynamicConfiguration(void)
{
    if (verbose) puts("STUB: commitDynamicConfiguration called");
    return NULL;
}

void* commitGlobalConfiguration(void)
{
    if (verbose) puts("STUB: commitGlobalConfiguration called");
    return NULL;
}

void* commitNetworkConfiguration(void)
{
    if (verbose) puts("STUB: commitNetworkConfiguration called");
    return NULL;
}

void* copyActiveWLANNetworkServices(void)
{
    if (verbose) puts("STUB: copyActiveWLANNetworkServices called");
    return NULL;
}

void* copyPrimaryActiveWLANNetworkService(void)
{
    if (verbose) puts("STUB: copyPrimaryActiveWLANNetworkService called");
    return NULL;
}

void* copyPrimaryTopLevelWLANNetworkService(void)
{
    if (verbose) puts("STUB: copyPrimaryTopLevelWLANNetworkService called");
    return NULL;
}

void* copyPrimaryWLANNetworkService(void)
{
    if (verbose) puts("STUB: copyPrimaryWLANNetworkService called");
    return NULL;
}

void* copyTopLevelWLANNetworkServices(void)
{
    if (verbose) puts("STUB: copyTopLevelWLANNetworkServices called");
    return NULL;
}

void* copyWLANNetworkInterface(void)
{
    if (verbose) puts("STUB: copyWLANNetworkInterface called");
    return NULL;
}

void* copyWLANNetworkInterfaceForService(void)
{
    if (verbose) puts("STUB: copyWLANNetworkInterfaceForService called");
    return NULL;
}

void* copyWLANNetworkServices(void)
{
    if (verbose) puts("STUB: copyWLANNetworkServices called");
    return NULL;
}

void* correctEthernetAddressFormat(void)
{
    if (verbose) puts("STUB: correctEthernetAddressFormat called");
    return NULL;
}

void* currentNetworkSetAvailable(void)
{
    if (verbose) puts("STUB: currentNetworkSetAvailable called");
    return NULL;
}

void* dynamicConfigurationForInterface(void)
{
    if (verbose) puts("STUB: dynamicConfigurationForInterface called");
    return NULL;
}

void* findSSIDInBeacon(void)
{
    if (verbose) puts("STUB: findSSIDInBeacon called");
    return NULL;
}

void* generateUUID(void)
{
    if (verbose) puts("STUB: generateUUID called");
    return NULL;
}

void* globalConfiguration(void)
{
    if (verbose) puts("STUB: globalConfiguration called");
    return NULL;
}

void* globalConfigurationForConfiguration(void)
{
    if (verbose) puts("STUB: globalConfigurationForConfiguration called");
    return NULL;
}

void* interfaceConfigurationForConfiguration(void)
{
    if (verbose) puts("STUB: interfaceConfigurationForConfiguration called");
    return NULL;
}

void* knownNetworkEntryForNetworkProfile(void)
{
    if (verbose) puts("STUB: knownNetworkEntryForNetworkProfile called");
    return NULL;
}

void* legacySecurityTypeForSecurityType(void)
{
    if (verbose) puts("STUB: legacySecurityTypeForSecurityType called");
    return NULL;
}

void* locStringForErrorCode(void)
{
    if (verbose) puts("STUB: locStringForErrorCode called");
    return NULL;
}

void* networkHiddenOrBroadcast(void)
{
    if (verbose) puts("STUB: networkHiddenOrBroadcast called");
    return NULL;
}

void* readCurrentNetworkFromNVRAM(void)
{
    if (verbose) puts("STUB: readCurrentNetworkFromNVRAM called");
    return NULL;
}

void* readRecoveryNetworksFromNVRAM(void)
{
    if (verbose) puts("STUB: readRecoveryNetworksFromNVRAM called");
    return NULL;
}

void* roamingProfileTypeForSchemaString(void)
{
    if (verbose) puts("STUB: roamingProfileTypeForSchemaString called");
    return NULL;
}

void* schemaStringForRoamingProfileType(void)
{
    if (verbose) puts("STUB: schemaStringForRoamingProfileType called");
    return NULL;
}

void* schemaStringForSecurityType(void)
{
    if (verbose) puts("STUB: schemaStringForSecurityType called");
    return NULL;
}

void* schemaStringForShareMode(void)
{
    if (verbose) puts("STUB: schemaStringForShareMode called");
    return NULL;
}

void* schemaStringForUserRole(void)
{
    if (verbose) puts("STUB: schemaStringForUserRole called");
    return NULL;
}

void* securityTypeForLegacySecurityType(void)
{
    if (verbose) puts("STUB: securityTypeForLegacySecurityType called");
    return NULL;
}

void* securityTypeForSchemaString(void)
{
    if (verbose) puts("STUB: securityTypeForSchemaString called");
    return NULL;
}

void* shareModeForSchemaString(void)
{
    if (verbose) puts("STUB: shareModeForSchemaString called");
    return NULL;
}

void* stringFromData(void)
{
    if (verbose) puts("STUB: stringFromData called");
    return NULL;
}

void* updateAPModeConfiguration(void)
{
    if (verbose) puts("STUB: updateAPModeConfiguration called");
    return NULL;
}

void* userRoleForSchemaString(void)
{
    if (verbose) puts("STUB: userRoleForSchemaString called");
    return NULL;
}

void* wlanNetworkInterfaceAvailable(void)
{
    if (verbose) puts("STUB: wlanNetworkInterfaceAvailable called");
    return NULL;
}

void* wow_get_parameters(void)
{
    if (verbose) puts("STUB: wow_get_parameters called");
    return NULL;
}

void* wow_get_wake_condition(void)
{
    if (verbose) puts("STUB: wow_get_wake_condition called");
    return NULL;
}

void* wow_preference_enabled(void)
{
    if (verbose) puts("STUB: wow_preference_enabled called");
    return NULL;
}

void* wow_set_parameters(void)
{
    if (verbose) puts("STUB: wow_set_parameters called");
    return NULL;
}

void* wow_set_wake_condition(void)
{
    if (verbose) puts("STUB: wow_set_wake_condition called");
    return NULL;
}

void* wps_cancel(void)
{
    if (verbose) puts("STUB: wps_cancel called");
    return NULL;
}

void* wps_generate_pin(void)
{
    if (verbose) puts("STUB: wps_generate_pin called");
    return NULL;
}

void* wps_requires_pin(void)
{
    if (verbose) puts("STUB: wps_requires_pin called");
    return NULL;
}

void* wps_start(void)
{
    if (verbose) puts("STUB: wps_start called");
    return NULL;
}

void* wps_validate_checksum(void)
{
    if (verbose) puts("STUB: wps_validate_checksum called");
    return NULL;
}

void* writeCurrentNetworkToNVRAM(void)
{
    if (verbose) puts("STUB: writeCurrentNetworkToNVRAM called");
    return NULL;
}

void* writeRecoveryNetworksToNVRAM(void)
{
    if (verbose) puts("STUB: writeRecoveryNetworksToNVRAM called");
    return NULL;
}
