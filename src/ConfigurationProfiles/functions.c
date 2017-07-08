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


#include <stdlib.h>
static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* BZip2Compress(void) { if (verbose) puts("STUB: BZip2Compress called"); return NULL; }
void* BZip2Decompress(void) { if (verbose) puts("STUB: BZip2Decompress called"); return NULL; }
void* CPCreateProvisioningDictionaries(void) { if (verbose) puts("STUB: CPCreateProvisioningDictionaries called"); return NULL; }
void* CPFetchActivationRecord(void) { if (verbose) puts("STUB: CPFetchActivationRecord called"); return NULL; }
void* CPGDI_AddDetailsRow(void) { if (verbose) puts("STUB: CPGDI_AddDetailsRow called"); return NULL; }
void* CPGDI_AddDetailsRowWithStyle(void) { if (verbose) puts("STUB: CPGDI_AddDetailsRowWithStyle called"); return NULL; }
void* CPGDI_GetLocStr(void) { if (verbose) puts("STUB: CPGDI_GetLocStr called"); return NULL; }
void* CPGDI_GetOptLocStr(void) { if (verbose) puts("STUB: CPGDI_GetOptLocStr called"); return NULL; }
void* CPGDI_RequestPayloadHidden(void) { if (verbose) puts("STUB: CPGDI_RequestPayloadHidden called"); return NULL; }
void* CPGDI_SetDetailsTitle(void) { if (verbose) puts("STUB: CPGDI_SetDetailsTitle called"); return NULL; }
void* CPGDI_SetSummary(void) { if (verbose) puts("STUB: CPGDI_SetSummary called"); return NULL; }
void* CPGDI_SetSummaryNote(void) { if (verbose) puts("STUB: CPGDI_SetSummaryNote called"); return NULL; }
void* CPGetActivationProfile(void) { if (verbose) puts("STUB: CPGetActivationProfile called"); return NULL; }
void* CPGetActivationRecord(void) { if (verbose) puts("STUB: CPGetActivationRecord called"); return NULL; }
void* CPGetActivationRecordTrigger(void) { if (verbose) puts("STUB: CPGetActivationRecordTrigger called"); return NULL; }
void* CPGetCachedActivationRecord(void) { if (verbose) puts("STUB: CPGetCachedActivationRecord called"); return NULL; }
void* CPGetInstalledConfigurationInfo(void) { if (verbose) puts("STUB: CPGetInstalledConfigurationInfo called"); return NULL; }
void* CPHasActivationRecord(void) { if (verbose) puts("STUB: CPHasActivationRecord called"); return NULL; }
void* CPInstallActivationProfile(void) { if (verbose) puts("STUB: CPInstallActivationProfile called"); return NULL; }
void* CPInstallActivationProfileFromSetupAssistant(void) { if (verbose) puts("STUB: CPInstallActivationProfileFromSetupAssistant called"); return NULL; }
void* CPMF_AddArrayIndexToKeyPath(void) { if (verbose) puts("STUB: CPMF_AddArrayIndexToKeyPath called"); return NULL; }
void* CPMF_AddDictKeyToKeyPath(void) { if (verbose) puts("STUB: CPMF_AddDictKeyToKeyPath called"); return NULL; }
void* CPMF_GetLocStrFmt(void) { if (verbose) puts("STUB: CPMF_GetLocStrFmt called"); return NULL; }
void* CPMF_IsKeyMissing(void) { if (verbose) puts("STUB: CPMF_IsKeyMissing called"); return NULL; }
void* CPMF_SplitKeyPaths(void) { if (verbose) puts("STUB: CPMF_SplitKeyPaths called"); return NULL; }
void* CPSetActivationProfileState(void) { if (verbose) puts("STUB: CPSetActivationProfileState called"); return NULL; }
void* CPSetPrimaryAccountGUID(void) { if (verbose) puts("STUB: CPSetPrimaryAccountGUID called"); return NULL; }
void* CPSetSetupAssistantUserInfo(void) { if (verbose) puts("STUB: CPSetSetupAssistantUserInfo called"); return NULL; }
void* CPValidateProvisioningDictionaries(void) { if (verbose) puts("STUB: CPValidateProvisioningDictionaries called"); return NULL; }
void* CP_AreMajorOSUpdatesManaged(void) { if (verbose) puts("STUB: CP_AreMajorOSUpdatesManaged called"); return NULL; }
void* CP_CaspianProfilesAreInstalled(void) { if (verbose) puts("STUB: CP_CaspianProfilesAreInstalled called"); return NULL; }
void* CP_DeviceIsEnrolledViaDEP(void) { if (verbose) puts("STUB: CP_DeviceIsEnrolledViaDEP called"); return NULL; }
void* CP_ExpandPayloadVariables(void) { if (verbose) puts("STUB: CP_ExpandPayloadVariables called"); return NULL; }
void* CP_ExpandPayloadVariablesDidChange(void) { if (verbose) puts("STUB: CP_ExpandPayloadVariablesDidChange called"); return NULL; }
void* CP_GetCertificateSignerSummary(void) { if (verbose) puts("STUB: CP_GetCertificateSignerSummary called"); return NULL; }
void* CP_GetExternalPluginPaths(void) { if (verbose) puts("STUB: CP_GetExternalPluginPaths called"); return NULL; }
void* CP_ManagedClientLoginAgentCheckin(void) { if (verbose) puts("STUB: CP_ManagedClientLoginAgentCheckin called"); return NULL; }
void* CP_RemoveProfilesWithXsanPayloads(void) { if (verbose) puts("STUB: CP_RemoveProfilesWithXsanPayloads called"); return NULL; }
void* CloudConfig_SetUserMustEnroll(void) { if (verbose) puts("STUB: CloudConfig_SetUserMustEnroll called"); return NULL; }
void* CloudConfig_ShouldEnroll(void) { if (verbose) puts("STUB: CloudConfig_ShouldEnroll called"); return NULL; }
void* DisableMCXDebug(void) { if (verbose) puts("STUB: DisableMCXDebug called"); return NULL; }
void* GetODProfiles(void) { if (verbose) puts("STUB: GetODProfiles called"); return NULL; }
void* GetUserDataForODProfile(void) { if (verbose) puts("STUB: GetUserDataForODProfile called"); return NULL; }
void* HasODProfiles(void) { if (verbose) puts("STUB: HasODProfiles called"); return NULL; }
void* IsBZip2Compressed(void) { if (verbose) puts("STUB: IsBZip2Compressed called"); return NULL; }
void* LoadConfigurationProfileInstallerUI(void) { if (verbose) puts("STUB: LoadConfigurationProfileInstallerUI called"); return NULL; }
void* ODDataToProfileInfo(void) { if (verbose) puts("STUB: ODDataToProfileInfo called"); return NULL; }
void* ProvisioningProfilesAreInstalled(void) { if (verbose) puts("STUB: ProvisioningProfilesAreInstalled called"); return NULL; }
