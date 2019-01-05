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


#ifndef _ConfigurationProfiles_H_
#define _ConfigurationProfiles_H_

#import <Foundation/Foundation.h>

#import <ConfigurationProfiles/CPProfileDomainServiceHostXPCProtocol.h>
#import <ConfigurationProfiles/CPProfileDomainServiceRequestProtocol.h>
#import <ConfigurationProfiles/CPProfileDomainServiceXPCProtocol.h>
#import <ConfigurationProfiles/CPProfileMissingFieldHelperProtocol.h>
#import <ConfigurationProfiles/MCTeslaConfigurationFetchInterface.h>
#import <ConfigurationProfiles/NSXPCConnectionDelegate.h>
#import <ConfigurationProfiles/NSXPCListenerDelegate.h>
#import <ConfigurationProfiles/CPProfile.h>
#import <ConfigurationProfiles/MCXPlugInKitManagementCache.h>
#import <ConfigurationProfiles/MCXPlugInKitManagement.h>
#import <ConfigurationProfiles/CPProfileManager.h>
#import <ConfigurationProfiles/CPProfileMissingFieldHelper.h>
#import <ConfigurationProfiles/CPProfileAdditionalWarningsHelper.h>
#import <ConfigurationProfiles/CPMCXTools.h>
#import <ConfigurationProfiles/CPProvisioningProfile.h>
#import <ConfigurationProfiles/CPProvisioningManager.h>
#import <ConfigurationProfiles/CPProfileDomainServiceConnectionHandler.h>
#import <ConfigurationProfiles/CPProfileDomainService.h>
#import <ConfigurationProfiles/CPIdentificationPayloads.h>
#import <ConfigurationProfiles/CPCompositorWatcher.h>
#import <ConfigurationProfiles/MCTeslaConfiguration.h>

void* BZip2Compress(void);
void* BZip2Decompress(void);
void* CPCreateProvisioningDictionaries(void);
void* CPFetchActivationRecord(void);
void* CPGDI_AddDetailsRow(void);
void* CPGDI_AddDetailsRowWithStyle(void);
void* CPGDI_GetLocStr(void);
void* CPGDI_GetOptLocStr(void);
void* CPGDI_RequestPayloadHidden(void);
void* CPGDI_SetDetailsTitle(void);
void* CPGDI_SetPayloadLocStringsInfo(void);
void* CPGDI_SetSummary(void);
void* CPGDI_SetSummaryNote(void);
void* CPGetActivationProfile(void);
void* CPGetActivationRecord(void);
void* CPGetActivationRecordTrigger(void);
void* CPGetCachedActivationRecord(void);
void* CPGetInstalledConfigurationInfo(void);
void* CPHasActivationRecord(void);
void* CPInstallActivationProfile(void);
void* CPInstallActivationProfileFromSetupAssistant(void);
void* CPInstallActivationProfilePriv(void);
void* CPMF_AddArrayIndexToKeyPath(void);
void* CPMF_AddDictKeyToKeyPath(void);
void* CPMF_GetLocStrFmt(void);
void* CPMF_IsKeyMissing(void);
void* CPMF_SplitKeyPaths(void);
void* CPSetActivationProfileState(void);
void* CPSetPrimaryAccountGUID(void);
void* CPSetSetupAssistantUserInfo(void);
void* CPValidateProvisioningDictionaries(void);
void* CP_ADCertificatePayloadUsesRPC(void);
void* CP_ArchiveNSError(void);
void* CP_AreMajorOSUpdatesManaged(void);
void* CP_CacheCloudConfigRecord(void);
void* CP_CaspianProfilesAreInstalled(void);
void* CP_CloudConfigHasActivationRecordHintFlagFileLocation(void);
void* CP_CloudConfigNoActivationRecordHintFlagFileLocation(void);
void* CP_CloudConfigRecordFoundCacheFileLocation(void);
void* CP_CloudConfigRecordInstalledHintFlagFileLocation(void);
void* CP_CloudConfigRecordNotFoundHintFlagFileLocation(void);
void* CP_CloudConfigRecordObtainedHintFlagFileLocation(void);
void* CP_CloudConfigSetupConfigurationFileLocation(void);
void* CP_CloudConfigUSetupUserFileLocation(void);
void* CP_CloudConfigUserSkippedEnrollmentFlagFileLocation(void);
void* CP_ConfigurationProfilesAreInstalled(void);
void* CP_CreatePersistentStoreDirectory(void);
void* CP_DeviceIsEnrolledViaDEP(void);
void* CP_ExpandPayloadVariables(void);
void* CP_ExpandPayloadVariablesDidChange(void);
void* CP_FDEGetPRKEscrowRedirectionInfo(void);
void* CP_FDEPRKEscrowRedirectionInfoForEFI(void);
void* CP_FDESavePRKForEscrow(void);
void* CP_GetCertificateSignerSummary(void);
void* CP_GetDeviceProfiles(void);
void* CP_GetManagedPreferencesFromPayload(void);
void* CP_GetManagementIdentityPersistRef(void);
void* CP_GetManagementStatus(void);
void* CP_GetPathForPID(void);
void* CP_GetUserProfiles(void);
void* CP_GetXPCSuccess(void);
void* CP_ImportCertificatesForPayload(void);
void* CP_IsAccountManaged(void);
void* CP_MDMOrgInfoManagingDomain(void);
void* CP_ManagedAccessibilityBundleIDs(void);
void* CP_ManagedClientLoginAgentCheckin(void);
void* CP_PasscodePoliciesAreInstalledHintFlagFileLocation(void);
void* CP_PreLoginCheckIn(void);
void* CP_ProfileInstalledFromUserApprovedManagementService(void);
void* CP_ProfilesAreInstalledHintFlagFileLocation(void);
void* CP_ReadSecureSettingsForDevice(void);
void* CP_ReadSecureSettingsForUser(void);
void* CP_RemoveProfilesWithXsanPayloads(void);
void* CP_RemoveSystemProfileWithIdentifier(void);
void* CP_RemoveUserProfileWithIdentifier(void);
void* CP_SetCloudConfigInstalledHintInfo(void);
void* CP_SetCloudConfigSetupUserInfo(void);
void* CP_SetXPCSuccess(void);
void* CP_StartupProfileChangeNeedsRestartHintFlagFileLocation(void);
void* CP_TransferXPCSuccess(void);
void* CP_UnarchiveSecureObject(void);
void* CP_UnlockUserAccount(void);
void* CloudConfig_SetUserMustEnroll(void);
void* CloudConfig_ShouldEnroll(void);
void* DisableMCXDebug(void);
void* GetODProfiles(void);
void* GetUserDataForODProfile(void);
void* HasODProfiles(void);
void* IsBZip2Compressed(void);
void* LoadMCXToolsInterface(void);
void* ODDataToProfileInfo(void);
void* ProvisioningProfilesAreInstalled(void);
void* _CP_ALAllowed(void);
void* _CP_ALBHash(void);

#endif
