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

void* CRAddBinaryImageHints(void);
void* CRAutoSubmitPreferenceIsLocked(void);
void* CRCanSubmitLog(void);
void* CRCopyCrashHistory(void);
void* CRCopySanitizedPath(void);
void* CRDisableCrashReporting(void);
void* CRDisplayProblemReport(void);
void* CREnableCrashReporting(void);
void* CRGetHostUUID(void);
void* CRGetUserUUID(void);
void* CRHasBeenAppleInternal(void);
void* CRHasBeenAppleInternalRecently(void);
void* CRIsAppleInternal(void);
void* CRIsAutoSubmitEnabled(void);
void* CRIsSeedBuild(void);
void* CRIsThirdPartyApplicationDataSubmissionEnabled(void);
void* CROpenProblemReport(void);
void* CRPrimeIsAppleInternal(void);
void* CRSaveProblemReport(void);
void* CRSubmitProblemReport(void);
void* CopyExtensionForProblemType(void);
void* CopyUpdatedWhitelistRanges(void);
void* OSAWriteLogForSubmission(void);
void* _CRAppendProblemReport(void);
void* _CRAsyncLoggingFinish(void);
void* _CRAutoSubmitEnabledIsSet(void);
void* _CRCleanUpLegacyLinksInDirectory(void);
void* _CRCopyAppleEmailAddress(void);
void* _CRCopyCrashHistoryForProcess(void);
void* _CRCopyCrashHistoryURLForProcess(void);
void* _CRCopyDiagnosticMessagesHistoryValue(void);
void* _CRCopyExecutablePathAndNameForPIDDetectingInconsistencies(void);
void* _CRCopyExecutablePathForPID(void);
void* _CRCopyProcessNameForPID(void);
void* _CRCopySanitizedProcessName(void);
void* _CRCopyStringXAttr(void);
void* _CRCopyUTF8String(void);
void* _CRCreateAssociatedReportPlistURL(void);
void* _CRCreateCrashHistoryDirectory(void);
void* _CRCreateDiagnosticMessagesQuery(void);
void* _CRCreateDiagnosticMessagesSubmissionQuery(void);
void* _CRCreateDictionaryWithPath(void);
void* _CRCreateFinalWhitelist(void);
void* _CRCreateInternalUUID(void);
void* _CRCreateProblemReportDirectory(void);
void* _CRCreateProblemReportFromContentsOfURL(void);
void* _CRCreateTimeStamp(void);
void* _CRCreateUnfilteredASLQuery(void);
void* _CRCreateWhitelistRanges(void);
void* _CRDisableSyslogging(void);
void* _CREnableAsyncLogging(void);
void* _CRExecuteInLoggingQueue(void);
void* _CRGetAnonHostUUID(void);
void* _CRGetDiagnosticMessagesHistoryBooleanValue(void);
void* _CRGetDiagnosticMessagesHistoryUInt64Value(void);
void* _CRGetHostName(void);
void* _CRGetOSVersionDictionary(void);
void* _CRGetOSVersionString(void);
void* _CRGetPrimaryMACAddress(void);
void* _CRIsAppleInternal(void);
void* _CRIsObviouslyAppleInternal(void);
void* _CRIsRunningInRootLaunchdContext(void);
void* _CRIsSeedBuild(void);
void* _CRMakeDirectory(void);
void* _CROpenProblemReport(void);
void* _CRProcessWillBeForceQuit(void);
void* _CRReopenApplicationForProblemReport(void);
void* _CRSafeLog(void);
void* _CRSecureOpen(void);
void* _CRSetAnonHostUUID(void);
void* _CRSetCrashExceptionPort(void);
void* _CRSetDiagnosticMessagesHistoryValue(void);
void* _CRSetIsAutoSubmitEnabled(void);
void* _CRSetIsThirdPartyApplicationDataSubmissionEnabled(void);
void* _CRSetXAttr(void);
void* _CRStashCrashExceptionPortInfo(void);
void* _CRThirdPartyApplicationDataSubmissionEnabledIsSet(void);
void* _CRWriteCrashHistoryForProcess(void);
void* _CRWritePropertyListWithPathAndMode(void);
void* _CreateASLQueryWithDomainList(void);
void* _canModifyPreferences(void);
void* _createAutoSubmitSettingObject(void);
void* _migrateLegacyAutoSubmitEnabledSetting(void);
void* _setPreference(void);
void* _updateHasBeenAppleInternalCookiePath(void);

#import <Foundation/Foundation.h>
#import <CrashReporterSupport/OSALogWriter.h>
