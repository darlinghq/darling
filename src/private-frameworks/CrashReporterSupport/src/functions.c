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

#include <stdio.h>
#include <stdlib.h>
static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* CRAddBinaryImageHints(void) { if (verbose) puts("STUB: CRAddBinaryImageHints called"); return NULL; }
void* CRAutoSubmitPreferenceIsLocked(void) { if (verbose) puts("STUB: CRAutoSubmitPreferenceIsLocked called"); return NULL; }
void* CRCanSubmitLog(void) { if (verbose) puts("STUB: CRCanSubmitLog called"); return NULL; }
void* CRCopyCrashHistory(void) { if (verbose) puts("STUB: CRCopyCrashHistory called"); return NULL; }
void* CRCopySanitizedPath(void) { if (verbose) puts("STUB: CRCopySanitizedPath called"); return NULL; }
void* CRDisableCrashReporting(void) { if (verbose) puts("STUB: CRDisableCrashReporting called"); return NULL; }
void* CRDisplayProblemReport(void) { if (verbose) puts("STUB: CRDisplayProblemReport called"); return NULL; }
void* CREnableCrashReporting(void) { if (verbose) puts("STUB: CREnableCrashReporting called"); return NULL; }
void* CRGetHostUUID(void) { if (verbose) puts("STUB: CRGetHostUUID called"); return NULL; }
void* CRGetUserUUID(void) { if (verbose) puts("STUB: CRGetUserUUID called"); return NULL; }
void* CRHasBeenAppleInternal(void) { if (verbose) puts("STUB: CRHasBeenAppleInternal called"); return NULL; }
void* CRHasBeenAppleInternalRecently(void) { if (verbose) puts("STUB: CRHasBeenAppleInternalRecently called"); return NULL; }
void* CRIsAppleInternal(void) { if (verbose) puts("STUB: CRIsAppleInternal called"); return NULL; }
void* CRIsAutoSubmitEnabled(void) { if (verbose) puts("STUB: CRIsAutoSubmitEnabled called"); return NULL; }
void* CRIsSeedBuild(void) { if (verbose) puts("STUB: CRIsSeedBuild called"); return NULL; }
void* CRIsThirdPartyApplicationDataSubmissionEnabled(void) { if (verbose) puts("STUB: CRIsThirdPartyApplicationDataSubmissionEnabled called"); return NULL; }
void* CROpenProblemReport(void) { if (verbose) puts("STUB: CROpenProblemReport called"); return NULL; }
void* CRPrimeIsAppleInternal(void) { if (verbose) puts("STUB: CRPrimeIsAppleInternal called"); return NULL; }
void* CRSaveProblemReport(void) { if (verbose) puts("STUB: CRSaveProblemReport called"); return NULL; }
void* CRSubmitProblemReport(void) { if (verbose) puts("STUB: CRSubmitProblemReport called"); return NULL; }
void* CopyExtensionForProblemType(void) { if (verbose) puts("STUB: CopyExtensionForProblemType called"); return NULL; }
void* CopyUpdatedWhitelistRanges(void) { if (verbose) puts("STUB: CopyUpdatedWhitelistRanges called"); return NULL; }
void* OSAWriteLogForSubmission(void) { if (verbose) puts("STUB: OSAWriteLogForSubmission called"); return NULL; }
void* _CRAppendProblemReport(void) { if (verbose) puts("STUB: _CRAppendProblemReport called"); return NULL; }
void* _CRAsyncLoggingFinish(void) { if (verbose) puts("STUB: _CRAsyncLoggingFinish called"); return NULL; }
void* _CRAutoSubmitEnabledIsSet(void) { if (verbose) puts("STUB: _CRAutoSubmitEnabledIsSet called"); return NULL; }
void* _CRCleanUpLegacyLinksInDirectory(void) { if (verbose) puts("STUB: _CRCleanUpLegacyLinksInDirectory called"); return NULL; }
void* _CRCopyAppleEmailAddress(void) { if (verbose) puts("STUB: _CRCopyAppleEmailAddress called"); return NULL; }
void* _CRCopyCrashHistoryForProcess(void) { if (verbose) puts("STUB: _CRCopyCrashHistoryForProcess called"); return NULL; }
void* _CRCopyCrashHistoryURLForProcess(void) { if (verbose) puts("STUB: _CRCopyCrashHistoryURLForProcess called"); return NULL; }
void* _CRCopyDiagnosticMessagesHistoryValue(void) { if (verbose) puts("STUB: _CRCopyDiagnosticMessagesHistoryValue called"); return NULL; }
void* _CRCopyExecutablePathAndNameForPIDDetectingInconsistencies(void) { if (verbose) puts("STUB: _CRCopyExecutablePathAndNameForPIDDetectingInconsistencies called"); return NULL; }
void* _CRCopyExecutablePathForPID(void) { if (verbose) puts("STUB: _CRCopyExecutablePathForPID called"); return NULL; }
void* _CRCopyProcessNameForPID(void) { if (verbose) puts("STUB: _CRCopyProcessNameForPID called"); return NULL; }
void* _CRCopySanitizedProcessName(void) { if (verbose) puts("STUB: _CRCopySanitizedProcessName called"); return NULL; }
void* _CRCopyStringXAttr(void) { if (verbose) puts("STUB: _CRCopyStringXAttr called"); return NULL; }
void* _CRCopyUTF8String(void) { if (verbose) puts("STUB: _CRCopyUTF8String called"); return NULL; }
void* _CRCreateAssociatedReportPlistURL(void) { if (verbose) puts("STUB: _CRCreateAssociatedReportPlistURL called"); return NULL; }
void* _CRCreateCrashHistoryDirectory(void) { if (verbose) puts("STUB: _CRCreateCrashHistoryDirectory called"); return NULL; }
void* _CRCreateDiagnosticMessagesQuery(void) { if (verbose) puts("STUB: _CRCreateDiagnosticMessagesQuery called"); return NULL; }
void* _CRCreateDiagnosticMessagesSubmissionQuery(void) { if (verbose) puts("STUB: _CRCreateDiagnosticMessagesSubmissionQuery called"); return NULL; }
void* _CRCreateDictionaryWithPath(void) { if (verbose) puts("STUB: _CRCreateDictionaryWithPath called"); return NULL; }
void* _CRCreateFinalWhitelist(void) { if (verbose) puts("STUB: _CRCreateFinalWhitelist called"); return NULL; }
void* _CRCreateInternalUUID(void) { if (verbose) puts("STUB: _CRCreateInternalUUID called"); return NULL; }
void* _CRCreateProblemReportDirectory(void) { if (verbose) puts("STUB: _CRCreateProblemReportDirectory called"); return NULL; }
void* _CRCreateProblemReportFromContentsOfURL(void) { if (verbose) puts("STUB: _CRCreateProblemReportFromContentsOfURL called"); return NULL; }
void* _CRCreateTimeStamp(void) { if (verbose) puts("STUB: _CRCreateTimeStamp called"); return NULL; }
void* _CRCreateUnfilteredASLQuery(void) { if (verbose) puts("STUB: _CRCreateUnfilteredASLQuery called"); return NULL; }
void* _CRCreateWhitelistRanges(void) { if (verbose) puts("STUB: _CRCreateWhitelistRanges called"); return NULL; }
void* _CRDisableSyslogging(void) { if (verbose) puts("STUB: _CRDisableSyslogging called"); return NULL; }
void* _CREnableAsyncLogging(void) { if (verbose) puts("STUB: _CREnableAsyncLogging called"); return NULL; }
void* _CRExecuteInLoggingQueue(void) { if (verbose) puts("STUB: _CRExecuteInLoggingQueue called"); return NULL; }
void* _CRGetAnonHostUUID(void) { if (verbose) puts("STUB: _CRGetAnonHostUUID called"); return NULL; }
void* _CRGetDiagnosticMessagesHistoryBooleanValue(void) { if (verbose) puts("STUB: _CRGetDiagnosticMessagesHistoryBooleanValue called"); return NULL; }
void* _CRGetDiagnosticMessagesHistoryUInt64Value(void) { if (verbose) puts("STUB: _CRGetDiagnosticMessagesHistoryUInt64Value called"); return NULL; }
void* _CRGetHostName(void) { if (verbose) puts("STUB: _CRGetHostName called"); return NULL; }
void* _CRGetOSVersionDictionary(void) { if (verbose) puts("STUB: _CRGetOSVersionDictionary called"); return NULL; }
void* _CRGetOSVersionString(void) { if (verbose) puts("STUB: _CRGetOSVersionString called"); return NULL; }
void* _CRGetPrimaryMACAddress(void) { if (verbose) puts("STUB: _CRGetPrimaryMACAddress called"); return NULL; }
void* _CRIsAppleInternal(void) { if (verbose) puts("STUB: _CRIsAppleInternal called"); return NULL; }
void* _CRIsObviouslyAppleInternal(void) { if (verbose) puts("STUB: _CRIsObviouslyAppleInternal called"); return NULL; }
void* _CRIsRunningInRootLaunchdContext(void) { if (verbose) puts("STUB: _CRIsRunningInRootLaunchdContext called"); return NULL; }
void* _CRIsSeedBuild(void) { if (verbose) puts("STUB: _CRIsSeedBuild called"); return NULL; }
void* _CRMakeDirectory(void) { if (verbose) puts("STUB: _CRMakeDirectory called"); return NULL; }
void* _CROpenProblemReport(void) { if (verbose) puts("STUB: _CROpenProblemReport called"); return NULL; }
void* _CRProcessWillBeForceQuit(void) { if (verbose) puts("STUB: _CRProcessWillBeForceQuit called"); return NULL; }
void* _CRReopenApplicationForProblemReport(void) { if (verbose) puts("STUB: _CRReopenApplicationForProblemReport called"); return NULL; }
void* _CRSafeLog(void) { if (verbose) puts("STUB: _CRSafeLog called"); return NULL; }
void* _CRSecureOpen(void) { if (verbose) puts("STUB: _CRSecureOpen called"); return NULL; }
void* _CRSetAnonHostUUID(void) { if (verbose) puts("STUB: _CRSetAnonHostUUID called"); return NULL; }
void* _CRSetCrashExceptionPort(void) { if (verbose) puts("STUB: _CRSetCrashExceptionPort called"); return NULL; }
void* _CRSetDiagnosticMessagesHistoryValue(void) { if (verbose) puts("STUB: _CRSetDiagnosticMessagesHistoryValue called"); return NULL; }
void* _CRSetIsAutoSubmitEnabled(void) { if (verbose) puts("STUB: _CRSetIsAutoSubmitEnabled called"); return NULL; }
void* _CRSetIsThirdPartyApplicationDataSubmissionEnabled(void) { if (verbose) puts("STUB: _CRSetIsThirdPartyApplicationDataSubmissionEnabled called"); return NULL; }
void* _CRSetXAttr(void) { if (verbose) puts("STUB: _CRSetXAttr called"); return NULL; }
void* _CRStashCrashExceptionPortInfo(void) { if (verbose) puts("STUB: _CRStashCrashExceptionPortInfo called"); return NULL; }
void* _CRThirdPartyApplicationDataSubmissionEnabledIsSet(void) { if (verbose) puts("STUB: _CRThirdPartyApplicationDataSubmissionEnabledIsSet called"); return NULL; }
void* _CRWriteCrashHistoryForProcess(void) { if (verbose) puts("STUB: _CRWriteCrashHistoryForProcess called"); return NULL; }
void* _CRWritePropertyListWithPathAndMode(void) { if (verbose) puts("STUB: _CRWritePropertyListWithPathAndMode called"); return NULL; }
void* _CreateASLQueryWithDomainList(void) { if (verbose) puts("STUB: _CreateASLQueryWithDomainList called"); return NULL; }
void* _canModifyPreferences(void) { if (verbose) puts("STUB: _canModifyPreferences called"); return NULL; }
void* _createAutoSubmitSettingObject(void) { if (verbose) puts("STUB: _createAutoSubmitSettingObject called"); return NULL; }
void* _migrateLegacyAutoSubmitEnabledSetting(void) { if (verbose) puts("STUB: _migrateLegacyAutoSubmitEnabledSetting called"); return NULL; }
void* _setPreference(void) { if (verbose) puts("STUB: _setPreference called"); return NULL; }
void* _updateHasBeenAppleInternalCookiePath(void) { if (verbose) puts("STUB: _updateHasBeenAppleInternalCookiePath called"); return NULL; }
