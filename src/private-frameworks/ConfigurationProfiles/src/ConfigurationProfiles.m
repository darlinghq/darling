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


#include <ConfigurationProfiles/ConfigurationProfiles.h>
#include <stdlib.h>
#include <stdio.h>

#include <ConfigurationProfiles/CPBootstrapToken.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* BZip2Compress(void) {
    if (verbose) puts("STUB: BZip2Compress called");
    return NULL;
}

void* BZip2Decompress(void) {
    if (verbose) puts("STUB: BZip2Decompress called");
    return NULL;
}

void* CPCreateProvisioningDictionaries(void) {
    if (verbose) puts("STUB: CPCreateProvisioningDictionaries called");
    return NULL;
}

void* CPFetchActivationRecord(void) {
    if (verbose) puts("STUB: CPFetchActivationRecord called");
    return NULL;
}

void* CPGDI_AddDetailsRow(void) {
    if (verbose) puts("STUB: CPGDI_AddDetailsRow called");
    return NULL;
}

void* CPGDI_AddDetailsRowWithStyle(void) {
    if (verbose) puts("STUB: CPGDI_AddDetailsRowWithStyle called");
    return NULL;
}

void* CPGDI_GetLocStr(void) {
    if (verbose) puts("STUB: CPGDI_GetLocStr called");
    return NULL;
}

void* CPGDI_GetOptLocStr(void) {
    if (verbose) puts("STUB: CPGDI_GetOptLocStr called");
    return NULL;
}

void* CPGDI_RequestPayloadHidden(void) {
    if (verbose) puts("STUB: CPGDI_RequestPayloadHidden called");
    return NULL;
}

void* CPGDI_SetDetailsTitle(void) {
    if (verbose) puts("STUB: CPGDI_SetDetailsTitle called");
    return NULL;
}

void* CPGDI_SetPayloadLocStringsInfo(void) {
    if (verbose) puts("STUB: CPGDI_SetPayloadLocStringsInfo called");
    return NULL;
}

void* CPGDI_SetSummary(void) {
    if (verbose) puts("STUB: CPGDI_SetSummary called");
    return NULL;
}

void* CPGDI_SetSummaryNote(void) {
    if (verbose) puts("STUB: CPGDI_SetSummaryNote called");
    return NULL;
}

void* CPGetActivationProfile(void) {
    if (verbose) puts("STUB: CPGetActivationProfile called");
    return NULL;
}

void* CPGetActivationRecord(void) {
    if (verbose) puts("STUB: CPGetActivationRecord called");
    return NULL;
}

void* CPGetActivationRecordTrigger(void) {
    if (verbose) puts("STUB: CPGetActivationRecordTrigger called");
    return NULL;
}

void* CPGetCachedActivationRecord(void) {
    if (verbose) puts("STUB: CPGetCachedActivationRecord called");
    return NULL;
}

void* CPGetInstalledConfigurationInfo(void) {
    if (verbose) puts("STUB: CPGetInstalledConfigurationInfo called");
    return NULL;
}

void* CPHasActivationRecord(void) {
    if (verbose) puts("STUB: CPHasActivationRecord called");
    return NULL;
}

void* CPInstallActivationProfile(void) {
    if (verbose) puts("STUB: CPInstallActivationProfile called");
    return NULL;
}

void* CPInstallActivationProfileFromSetupAssistant(void) {
    if (verbose) puts("STUB: CPInstallActivationProfileFromSetupAssistant called");
    return NULL;
}

void* CPInstallActivationProfilePriv(void) {
    if (verbose) puts("STUB: CPInstallActivationProfilePriv called");
    return NULL;
}

void* CPMF_AddArrayIndexToKeyPath(void) {
    if (verbose) puts("STUB: CPMF_AddArrayIndexToKeyPath called");
    return NULL;
}

void* CPMF_AddDictKeyToKeyPath(void) {
    if (verbose) puts("STUB: CPMF_AddDictKeyToKeyPath called");
    return NULL;
}

void* CPMF_GetLocStrFmt(void) {
    if (verbose) puts("STUB: CPMF_GetLocStrFmt called");
    return NULL;
}

void* CPMF_IsKeyMissing(void) {
    if (verbose) puts("STUB: CPMF_IsKeyMissing called");
    return NULL;
}

void* CPMF_SplitKeyPaths(void) {
    if (verbose) puts("STUB: CPMF_SplitKeyPaths called");
    return NULL;
}

void* CPSetActivationProfileState(void) {
    if (verbose) puts("STUB: CPSetActivationProfileState called");
    return NULL;
}

void* CPSetPrimaryAccountGUID(void) {
    if (verbose) puts("STUB: CPSetPrimaryAccountGUID called");
    return NULL;
}

void* CPSetSetupAssistantUserInfo(void) {
    if (verbose) puts("STUB: CPSetSetupAssistantUserInfo called");
    return NULL;
}

void* CPValidateProvisioningDictionaries(void) {
    if (verbose) puts("STUB: CPValidateProvisioningDictionaries called");
    return NULL;
}

void* CP_ADCertificatePayloadUsesRPC(void) {
    if (verbose) puts("STUB: CP_ADCertificatePayloadUsesRPC called");
    return NULL;
}

void* CP_ArchiveNSError(void) {
    if (verbose) puts("STUB: CP_ArchiveNSError called");
    return NULL;
}

void* CP_AreMajorOSUpdatesManaged(void) {
    if (verbose) puts("STUB: CP_AreMajorOSUpdatesManaged called");
    return NULL;
}

void* CP_CacheCloudConfigRecord(void) {
    if (verbose) puts("STUB: CP_CacheCloudConfigRecord called");
    return NULL;
}

void* CP_CaspianProfilesAreInstalled(void) {
    if (verbose) puts("STUB: CP_CaspianProfilesAreInstalled called");
    return NULL;
}

void* CP_CloudConfigHasActivationRecordHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigHasActivationRecordHintFlagFileLocation called");
    return NULL;
}

void* CP_CloudConfigNoActivationRecordHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigNoActivationRecordHintFlagFileLocation called");
    return NULL;
}

void* CP_CloudConfigRecordFoundCacheFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigRecordFoundCacheFileLocation called");
    return NULL;
}

void* CP_CloudConfigRecordInstalledHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigRecordInstalledHintFlagFileLocation called");
    return NULL;
}

void* CP_CloudConfigRecordNotFoundHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigRecordNotFoundHintFlagFileLocation called");
    return NULL;
}

void* CP_CloudConfigRecordObtainedHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigRecordObtainedHintFlagFileLocation called");
    return NULL;
}

void* CP_CloudConfigSetupConfigurationFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigSetupConfigurationFileLocation called");
    return NULL;
}

void* CP_CloudConfigUSetupUserFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigUSetupUserFileLocation called");
    return NULL;
}

void* CP_CloudConfigUserSkippedEnrollmentFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_CloudConfigUserSkippedEnrollmentFlagFileLocation called");
    return NULL;
}

void* CP_ConfigurationProfilesAreInstalled(void) {
    if (verbose) puts("STUB: CP_ConfigurationProfilesAreInstalled called");
    return NULL;
}

void* CP_CreatePersistentStoreDirectory(void) {
    if (verbose) puts("STUB: CP_CreatePersistentStoreDirectory called");
    return NULL;
}

void* CP_DeviceIsEnrolledViaDEP(void) {
    if (verbose) puts("STUB: CP_DeviceIsEnrolledViaDEP called");
    return NULL;
}

void* CP_ExpandPayloadVariables(void) {
    if (verbose) puts("STUB: CP_ExpandPayloadVariables called");
    return NULL;
}

void* CP_ExpandPayloadVariablesDidChange(void) {
    if (verbose) puts("STUB: CP_ExpandPayloadVariablesDidChange called");
    return NULL;
}

void* CP_FDEGetPRKEscrowRedirectionInfo(void) {
    if (verbose) puts("STUB: CP_FDEGetPRKEscrowRedirectionInfo called");
    return NULL;
}

void* CP_FDEPRKEscrowRedirectionInfoForEFI(void) {
    if (verbose) puts("STUB: CP_FDEPRKEscrowRedirectionInfoForEFI called");
    return NULL;
}

void* CP_FDESavePRKForEscrow(void) {
    if (verbose) puts("STUB: CP_FDESavePRKForEscrow called");
    return NULL;
}

void* CP_GetCertificateSignerSummary(void) {
    if (verbose) puts("STUB: CP_GetCertificateSignerSummary called");
    return NULL;
}

void* CP_GetDeviceProfiles(void) {
    if (verbose) puts("STUB: CP_GetDeviceProfiles called");
    return NULL;
}

void* CP_GetManagedPreferencesFromPayload(void) {
    if (verbose) puts("STUB: CP_GetManagedPreferencesFromPayload called");
    return NULL;
}

void* CP_GetManagementIdentityPersistRef(void) {
    if (verbose) puts("STUB: CP_GetManagementIdentityPersistRef called");
    return NULL;
}

void* CP_GetManagementStatus(void) {
    if (verbose) puts("STUB: CP_GetManagementStatus called");
    return NULL;
}

void* CP_GetPathForPID(void) {
    if (verbose) puts("STUB: CP_GetPathForPID called");
    return NULL;
}

void* CP_GetUserProfiles(void) {
    if (verbose) puts("STUB: CP_GetUserProfiles called");
    return NULL;
}

void* CP_GetXPCSuccess(void) {
    if (verbose) puts("STUB: CP_GetXPCSuccess called");
    return NULL;
}

void* CP_ImportCertificatesForPayload(void) {
    if (verbose) puts("STUB: CP_ImportCertificatesForPayload called");
    return NULL;
}

void* CP_IsAccountManaged(void) {
    if (verbose) puts("STUB: CP_IsAccountManaged called");
    return NULL;
}

void* CP_MDMOrgInfoManagingDomain(void) {
    if (verbose) puts("STUB: CP_MDMOrgInfoManagingDomain called");
    return NULL;
}

void* CP_ManagedAccessibilityBundleIDs(void) {
    if (verbose) puts("STUB: CP_ManagedAccessibilityBundleIDs called");
    return NULL;
}

void* CP_ManagedClientLoginAgentCheckin(void) {
    if (verbose) puts("STUB: CP_ManagedClientLoginAgentCheckin called");
    return NULL;
}

void* CP_PasscodePoliciesAreInstalledHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_PasscodePoliciesAreInstalledHintFlagFileLocation called");
    return NULL;
}

void* CP_PreLoginCheckIn(void) {
    if (verbose) puts("STUB: CP_PreLoginCheckIn called");
    return NULL;
}

void* CP_ProfileInstalledFromUserApprovedManagementService(void) {
    if (verbose) puts("STUB: CP_ProfileInstalledFromUserApprovedManagementService called");
    return NULL;
}

void* CP_ProfilesAreInstalledHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_ProfilesAreInstalledHintFlagFileLocation called");
    return NULL;
}

void* CP_ReadSecureSettingsForDevice(void) {
    if (verbose) puts("STUB: CP_ReadSecureSettingsForDevice called");
    return NULL;
}

void* CP_ReadSecureSettingsForUser(void) {
    if (verbose) puts("STUB: CP_ReadSecureSettingsForUser called");
    return NULL;
}

void* CP_RemoveProfilesWithXsanPayloads(void) {
    if (verbose) puts("STUB: CP_RemoveProfilesWithXsanPayloads called");
    return NULL;
}

void* CP_RemoveSystemProfileWithIdentifier(void) {
    if (verbose) puts("STUB: CP_RemoveSystemProfileWithIdentifier called");
    return NULL;
}

void* CP_RemoveUserProfileWithIdentifier(void) {
    if (verbose) puts("STUB: CP_RemoveUserProfileWithIdentifier called");
    return NULL;
}

void* CP_SetCloudConfigInstalledHintInfo(void) {
    if (verbose) puts("STUB: CP_SetCloudConfigInstalledHintInfo called");
    return NULL;
}

void* CP_SetCloudConfigSetupUserInfo(void) {
    if (verbose) puts("STUB: CP_SetCloudConfigSetupUserInfo called");
    return NULL;
}

void* CP_SetXPCSuccess(void) {
    if (verbose) puts("STUB: CP_SetXPCSuccess called");
    return NULL;
}

void* CP_StartupProfileChangeNeedsRestartHintFlagFileLocation(void) {
    if (verbose) puts("STUB: CP_StartupProfileChangeNeedsRestartHintFlagFileLocation called");
    return NULL;
}

void* CP_TransferXPCSuccess(void) {
    if (verbose) puts("STUB: CP_TransferXPCSuccess called");
    return NULL;
}

void* CP_UnarchiveSecureObject(void) {
    if (verbose) puts("STUB: CP_UnarchiveSecureObject called");
    return NULL;
}

void* CP_UnlockUserAccount(void) {
    if (verbose) puts("STUB: CP_UnlockUserAccount called");
    return NULL;
}

void* CloudConfig_SetUserMustEnroll(void) {
    if (verbose) puts("STUB: CloudConfig_SetUserMustEnroll called");
    return NULL;
}

void* CloudConfig_ShouldEnroll(void) {
    if (verbose) puts("STUB: CloudConfig_ShouldEnroll called");
    return NULL;
}

void* DisableMCXDebug(void) {
    if (verbose) puts("STUB: DisableMCXDebug called");
    return NULL;
}

void* GetODProfiles(void) {
    if (verbose) puts("STUB: GetODProfiles called");
    return NULL;
}

void* GetUserDataForODProfile(void) {
    if (verbose) puts("STUB: GetUserDataForODProfile called");
    return NULL;
}

void* HasODProfiles(void) {
    if (verbose) puts("STUB: HasODProfiles called");
    return NULL;
}

void* IsBZip2Compressed(void) {
    if (verbose) puts("STUB: IsBZip2Compressed called");
    return NULL;
}

void* LoadMCXToolsInterface(void) {
    if (verbose) puts("STUB: LoadMCXToolsInterface called");
    return NULL;
}

void* ODDataToProfileInfo(void) {
    if (verbose) puts("STUB: ODDataToProfileInfo called");
    return NULL;
}

void* ProvisioningProfilesAreInstalled(void) {
    if (verbose) puts("STUB: ProvisioningProfilesAreInstalled called");
    return NULL;
}

void* _CP_ALAllowed(void) {
    if (verbose) puts("STUB: _CP_ALAllowed called");
    return NULL;
}

void* _CP_ALBHash(void) {
    if (verbose) puts("STUB: _CP_ALBHash called");
    return NULL;
}

BOOL CP_SupportsBootstrapToken(void) {
    if (verbose) puts("STUB: CP_SupportsBootstrapToken called");
    return NO;
};

NSString* CP_GetBootstrapToken(void* something) {
    return nil;
};

NSString* CP_GetBootstrapTokenWithOptions(NSDictionary<NSString*,NSNumber*> *token, void* unknown) {
    if (verbose) puts("STUB: CP_GetBootstrapTokenWithOptions called");
    return nil;
}
