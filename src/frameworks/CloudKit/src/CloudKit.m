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


#include <CloudKit/CloudKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CKAbsoluteHostTime(void)
{
    if (verbose) puts("STUB: CKAbsoluteHostTime called");
    return NULL;
}

void* CKAcceptablePredicateValueClasses(void)
{
    if (verbose) puts("STUB: CKAcceptablePredicateValueClasses called");
    return NULL;
}

void* CKAcceptableValueClasses(void)
{
    if (verbose) puts("STUB: CKAcceptableValueClasses called");
    return NULL;
}

void* CKAllClientEntitlementKeys(void)
{
    if (verbose) puts("STUB: CKAllClientEntitlementKeys called");
    return NULL;
}

void* CKAppIdentifierFromTeamAppTuple(void)
{
    if (verbose) puts("STUB: CKAppIdentifierFromTeamAppTuple called");
    return NULL;
}

void* CKAutoBugCaptureReasonToString(void)
{
    if (verbose) puts("STUB: CKAutoBugCaptureReasonToString called");
    return NULL;
}

void* CKBoolFromCKTernary(void)
{
    if (verbose) puts("STUB: CKBoolFromCKTernary called");
    return NULL;
}

void* CKBuildVersion(void)
{
    if (verbose) puts("STUB: CKBuildVersion called");
    return NULL;
}

void* CKCFArrayForEach(void)
{
    if (verbose) puts("STUB: CKCFArrayForEach called");
    return NULL;
}

void* CKCFDictionaryForEach(void)
{
    if (verbose) puts("STUB: CKCFDictionaryForEach called");
    return NULL;
}

void* CKCanRetryForError(void)
{
    if (verbose) puts("STUB: CKCanRetryForError called");
    return NULL;
}

void* CKContainerEnvironmentFromString(void)
{
    if (verbose) puts("STUB: CKContainerEnvironmentFromString called");
    return NULL;
}

void* CKContainerEnvironmentString(void)
{
    if (verbose) puts("STUB: CKContainerEnvironmentString called");
    return NULL;
}

void* CKContainerIdentifierIsAppleInternal(void)
{
    if (verbose) puts("STUB: CKContainerIdentifierIsAppleInternal called");
    return NULL;
}

void* CKCreateDirectoryAtPath(void)
{
    if (verbose) puts("STUB: CKCreateDirectoryAtPath called");
    return NULL;
}

void* CKCreateDirectoryAtPathWithAttributes(void)
{
    if (verbose) puts("STUB: CKCreateDirectoryAtPathWithAttributes called");
    return NULL;
}

void* CKCreateGUID(void)
{
    if (verbose) puts("STUB: CKCreateGUID called");
    return NULL;
}

void* CKCurrentProcessIsDaemon(void)
{
    if (verbose) puts("STUB: CKCurrentProcessIsDaemon called");
    return NULL;
}

void* CKCurrentQueueIsMainQueue(void)
{
    if (verbose) puts("STUB: CKCurrentQueueIsMainQueue called");
    return NULL;
}

void* CKCurrentQueueQualityOfService(void)
{
    if (verbose) puts("STUB: CKCurrentQueueQualityOfService called");
    return NULL;
}

void* CKCurrentThreadQualityOfService(void)
{
    if (verbose) puts("STUB: CKCurrentThreadQualityOfService called");
    return NULL;
}

void* CKDPDateReadFrom(void)
{
    if (verbose) puts("STUB: CKDPDateReadFrom called");
    return NULL;
}

void* CKDPIdentifierReadFrom(void)
{
    if (verbose) puts("STUB: CKDPIdentifierReadFrom called");
    return NULL;
}

void* CKDPLocationCoordinateReadFrom(void)
{
    if (verbose) puts("STUB: CKDPLocationCoordinateReadFrom called");
    return NULL;
}

void* CKDPRecordFieldValueEncryptedValueReadFrom(void)
{
    if (verbose) puts("STUB: CKDPRecordFieldValueEncryptedValueReadFrom called");
    return NULL;
}

void* CKDPRecordIdentifierReadFrom(void)
{
    if (verbose) puts("STUB: CKDPRecordIdentifierReadFrom called");
    return NULL;
}

void* CKDPRecordReferenceReadFrom(void)
{
    if (verbose) puts("STUB: CKDPRecordReferenceReadFrom called");
    return NULL;
}

void* CKDPRecordZoneIdentifierReadFrom(void)
{
    if (verbose) puts("STUB: CKDPRecordZoneIdentifierReadFrom called");
    return NULL;
}

void* CKDatabaseScopeFromString(void)
{
    if (verbose) puts("STUB: CKDatabaseScopeFromString called");
    return NULL;
}

void* CKDatabaseScopeString(void)
{
    if (verbose) puts("STUB: CKDatabaseScopeString called");
    return NULL;
}

void* CKDatasAreBothNilOrEqual(void)
{
    if (verbose) puts("STUB: CKDatasAreBothNilOrEqual called");
    return NULL;
}

void* CKDeviceID(void)
{
    if (verbose) puts("STUB: CKDeviceID called");
    return NULL;
}

void* CKEarliestStartDateAfterError(void)
{
    if (verbose) puts("STUB: CKEarliestStartDateAfterError called");
    return NULL;
}

void* CKErrorChainFromError(void)
{
    if (verbose) puts("STUB: CKErrorChainFromError called");
    return NULL;
}

void* CKErrorChainStringFromError(void)
{
    if (verbose) puts("STUB: CKErrorChainStringFromError called");
    return NULL;
}

void* CKErrorCodeForInternalErrorCode(void)
{
    if (verbose) puts("STUB: CKErrorCodeForInternalErrorCode called");
    return NULL;
}

void* CKErrorCodeForNSURLErrorCode(void)
{
    if (verbose) puts("STUB: CKErrorCodeForNSURLErrorCode called");
    return NULL;
}

void* CKErrorIsCode(void)
{
    if (verbose) puts("STUB: CKErrorIsCode called");
    return NULL;
}

void* CKFetchAPSEnvironmentFromServerOrEntitlements(void)
{
    if (verbose) puts("STUB: CKFetchAPSEnvironmentFromServerOrEntitlements called");
    return NULL;
}

void* CKGetCurrentActivities(void)
{
    if (verbose) puts("STUB: CKGetCurrentActivities called");
    return NULL;
}

void* CKGetGlobalQueue(void)
{
    if (verbose) puts("STUB: CKGetGlobalQueue called");
    return NULL;
}

void* CKGetHomeDir(void)
{
    if (verbose) puts("STUB: CKGetHomeDir called");
    return NULL;
}

void* CKGetHomeDirRealPath(void)
{
    if (verbose) puts("STUB: CKGetHomeDirRealPath called");
    return NULL;
}

void* CKGetRealPath(void)
{
    if (verbose) puts("STUB: CKGetRealPath called");
    return NULL;
}

void* CKHexCharFromBytes(void)
{
    if (verbose) puts("STUB: CKHexCharFromBytes called");
    return NULL;
}

void* CKIndexedArrayKey(void)
{
    if (verbose) puts("STUB: CKIndexedArrayKey called");
    return NULL;
}

void* CKIsPCSError(void)
{
    if (verbose) puts("STUB: CKIsPCSError called");
    return NULL;
}

void* CKIsRunningInCloudD(void)
{
    if (verbose) puts("STUB: CKIsRunningInCloudD called");
    return NULL;
}

void* CKIsRunningInTestHost(void)
{
    if (verbose) puts("STUB: CKIsRunningInTestHost called");
    return NULL;
}

void* CKIsValidOperationForScope(void)
{
    if (verbose) puts("STUB: CKIsValidOperationForScope called");
    return NULL;
}

void* CKLinkCheck0fd6bdf95f2efb6e65813fd4cd0f5d9af656d08a(void)
{
    if (verbose) puts("STUB: CKLinkCheck0fd6bdf95f2efb6e65813fd4cd0f5d9af656d08a called");
    return NULL;
}

void* CKLinkCheck5dbf91c3fd1d871f0bcfe60afeb451e3e708d350(void)
{
    if (verbose) puts("STUB: CKLinkCheck5dbf91c3fd1d871f0bcfe60afeb451e3e708d350 called");
    return NULL;
}

void* CKLinkCheck908c3403f5370f9fc0f790c790ce4de0669132c0(void)
{
    if (verbose) puts("STUB: CKLinkCheck908c3403f5370f9fc0f790c790ce4de0669132c0 called");
    return NULL;
}

void* CKLinkCheck92e3e8f8ec1a906754afb22d87eb36301b4f6760(void)
{
    if (verbose) puts("STUB: CKLinkCheck92e3e8f8ec1a906754afb22d87eb36301b4f6760 called");
    return NULL;
}

void* CKMainBundleIsAppleExecutable(void)
{
    if (verbose) puts("STUB: CKMainBundleIsAppleExecutable called");
    return NULL;
}

void* CKNSIndexSet_enumerateInverseRangesInRange_options_usingBlock(void)
{
    if (verbose) puts("STUB: CKNSIndexSet_enumerateInverseRangesInRange_options_usingBlock called");
    return NULL;
}

void* CKNSIndexSet_indexSetWithInverseIndexSet(void)
{
    if (verbose) puts("STUB: CKNSIndexSet_indexSetWithInverseIndexSet called");
    return NULL;
}

void* CKNSQualityOfServiceFromQoSClass(void)
{
    if (verbose) puts("STUB: CKNSQualityOfServiceFromQoSClass called");
    return NULL;
}

void* CKOperationGroupTransferSizeForBytes(void)
{
    if (verbose) puts("STUB: CKOperationGroupTransferSizeForBytes called");
    return NULL;
}

void* CKOperationLoggableProgressArguments(void)
{
    if (verbose) puts("STUB: CKOperationLoggableProgressArguments called");
    return NULL;
}

void* CKOperationProgressCallbackClasses(void)
{
    if (verbose) puts("STUB: CKOperationProgressCallbackClasses called");
    return NULL;
}

void* CKPIDIsStillAlive(void)
{
    if (verbose) puts("STUB: CKPIDIsStillAlive called");
    return NULL;
}

void* CKProcessIndexedArrayKey(void)
{
    if (verbose) puts("STUB: CKProcessIndexedArrayKey called");
    return NULL;
}

void* CKProductName(void)
{
    if (verbose) puts("STUB: CKProductName called");
    return NULL;
}

void* CKProductType(void)
{
    if (verbose) puts("STUB: CKProductType called");
    return NULL;
}

void* CKProductVersion(void)
{
    if (verbose) puts("STUB: CKProductVersion called");
    return NULL;
}

void* CKQoSClassFromNSQualityOfService(void)
{
    if (verbose) puts("STUB: CKQoSClassFromNSQualityOfService called");
    return NULL;
}

void* CKQoSIsBackground(void)
{
    if (verbose) puts("STUB: CKQoSIsBackground called");
    return NULL;
}

void* CKQualityOfServiceOrdering(void)
{
    if (verbose) puts("STUB: CKQualityOfServiceOrdering called");
    return NULL;
}

void* CKRetryAfterSecondsForError(void)
{
    if (verbose) puts("STUB: CKRetryAfterSecondsForError called");
    return NULL;
}

void* CKSDKVersion(void)
{
    if (verbose) puts("STUB: CKSDKVersion called");
    return NULL;
}

void* CKSQLiteJournalSuffixes(void)
{
    if (verbose) puts("STUB: CKSQLiteJournalSuffixes called");
    return NULL;
}

void* CKServerEnvironmentFromString(void)
{
    if (verbose) puts("STUB: CKServerEnvironmentFromString called");
    return NULL;
}

void* CKShareURLSlugForiWorkShareTitle(void)
{
    if (verbose) puts("STUB: CKShareURLSlugForiWorkShareTitle called");
    return NULL;
}

void* CKSharingContainerSlugForContainerID(void)
{
    if (verbose) puts("STUB: CKSharingContainerSlugForContainerID called");
    return NULL;
}

void* CKShortRandomID(void)
{
    if (verbose) puts("STUB: CKShortRandomID called");
    return NULL;
}

void* CKShouldUseNewPredicateValidation(void)
{
    if (verbose) puts("STUB: CKShouldUseNewPredicateValidation called");
    return NULL;
}

void* CKShouldWrapErrorFetchingRecords(void)
{
    if (verbose) puts("STUB: CKShouldWrapErrorFetchingRecords called");
    return NULL;
}

void* CKStringForDiscretionaryNetworkBehavior(void)
{
    if (verbose) puts("STUB: CKStringForDiscretionaryNetworkBehavior called");
    return NULL;
}

void* CKStringForNetworkServiceType(void)
{
    if (verbose) puts("STUB: CKStringForNetworkServiceType called");
    return NULL;
}

void* CKStringForQOS(void)
{
    if (verbose) puts("STUB: CKStringForQOS called");
    return NULL;
}

void* CKStringForTransferSize(void)
{
    if (verbose) puts("STUB: CKStringForTransferSize called");
    return NULL;
}

void* CKStringForXPCActivityState(void)
{
    if (verbose) puts("STUB: CKStringForXPCActivityState called");
    return NULL;
}

void* CKStringFromAccountChangeType(void)
{
    if (verbose) puts("STUB: CKStringFromAccountChangeType called");
    return NULL;
}

void* CKStringFromAccountStatus(void)
{
    if (verbose) puts("STUB: CKStringFromAccountStatus called");
    return NULL;
}

void* CKStringFromApplicationPermissionStatus(void)
{
    if (verbose) puts("STUB: CKStringFromApplicationPermissionStatus called");
    return NULL;
}

void* CKStringFromCKMMCSEncryptionSupport(void)
{
    if (verbose) puts("STUB: CKStringFromCKMMCSEncryptionSupport called");
    return NULL;
}

void* CKStringFromCapabilities(void)
{
    if (verbose) puts("STUB: CKStringFromCapabilities called");
    return NULL;
}

void* CKStringFromDeviceToDeviceEncryptionStatus(void)
{
    if (verbose) puts("STUB: CKStringFromDeviceToDeviceEncryptionStatus called");
    return NULL;
}

void* CKStringFromParticipantAcceptanceStatus(void)
{
    if (verbose) puts("STUB: CKStringFromParticipantAcceptanceStatus called");
    return NULL;
}

void* CKStringFromParticipantInvitationTokenStatus(void)
{
    if (verbose) puts("STUB: CKStringFromParticipantInvitationTokenStatus called");
    return NULL;
}

void* CKStringFromParticipantPermission(void)
{
    if (verbose) puts("STUB: CKStringFromParticipantPermission called");
    return NULL;
}

void* CKStringFromParticipantRole(void)
{
    if (verbose) puts("STUB: CKStringFromParticipantRole called");
    return NULL;
}

void* CKStringFromPartition(void)
{
    if (verbose) puts("STUB: CKStringFromPartition called");
    return NULL;
}

void* CKStringFromServerEnvironment(void)
{
    if (verbose) puts("STUB: CKStringFromServerEnvironment called");
    return NULL;
}

void* CKStringFromShareParticipantVisibility(void)
{
    if (verbose) puts("STUB: CKStringFromShareParticipantVisibility called");
    return NULL;
}

void* CKStringWithArray(void)
{
    if (verbose) puts("STUB: CKStringWithArray called");
    return NULL;
}

void* CKStringWithBytes(void)
{
    if (verbose) puts("STUB: CKStringWithBytes called");
    return NULL;
}

void* CKStringWithData(void)
{
    if (verbose) puts("STUB: CKStringWithData called");
    return NULL;
}

void* CKStringWithDate(void)
{
    if (verbose) puts("STUB: CKStringWithDate called");
    return NULL;
}

void* CKStringWithDictionary(void)
{
    if (verbose) puts("STUB: CKStringWithDictionary called");
    return NULL;
}

void* CKStringWithLimitedArray(void)
{
    if (verbose) puts("STUB: CKStringWithLimitedArray called");
    return NULL;
}

void* CKStringWithNibbles(void)
{
    if (verbose) puts("STUB: CKStringWithNibbles called");
    return NULL;
}

void* CKStringWithObject(void)
{
    if (verbose) puts("STUB: CKStringWithObject called");
    return NULL;
}

void* CKStringWithSet(void)
{
    if (verbose) puts("STUB: CKStringWithSet called");
    return NULL;
}

void* CKStringsAreBothNilOrEqual(void)
{
    if (verbose) puts("STUB: CKStringsAreBothNilOrEqual called");
    return NULL;
}

void* CKSyncEngineRecordModificationTypeOpposite(void)
{
    if (verbose) puts("STUB: CKSyncEngineRecordModificationTypeOpposite called");
    return NULL;
}

void* CKTabIndentAtDepth(void)
{
    if (verbose) puts("STUB: CKTabIndentAtDepth called");
    return NULL;
}

void* CKTernaryFromBOOL(void)
{
    if (verbose) puts("STUB: CKTernaryFromBOOL called");
    return NULL;
}

void* CKTopLevelUnderlyingErrorCodes(void)
{
    if (verbose) puts("STUB: CKTopLevelUnderlyingErrorCodes called");
    return NULL;
}

void* CKTriggerAutoBugCaptureWithFormatSignature(void)
{
    if (verbose) puts("STUB: CKTriggerAutoBugCaptureWithFormatSignature called");
    return NULL;
}

void* CKTriggerClientSideAutoBugCaptureWithFormatSignature(void)
{
    if (verbose) puts("STUB: CKTriggerClientSideAutoBugCaptureWithFormatSignature called");
    return NULL;
}

void* CKValidSharingURLHostnames(void)
{
    if (verbose) puts("STUB: CKValidSharingURLHostnames called");
    return NULL;
}

void* CKValidateIndexedArrayKeys(void)
{
    if (verbose) puts("STUB: CKValidateIndexedArrayKeys called");
    return NULL;
}

void* CKValidateKeyName(void)
{
    if (verbose) puts("STUB: CKValidateKeyName called");
    return NULL;
}

void* CKValidateRecordArrayValue(void)
{
    if (verbose) puts("STUB: CKValidateRecordArrayValue called");
    return NULL;
}

void* CKValueIsAcceptableClass(void)
{
    if (verbose) puts("STUB: CKValueIsAcceptableClass called");
    return NULL;
}

void* CKValueIsAcceptablePredicateClass(void)
{
    if (verbose) puts("STUB: CKValueIsAcceptablePredicateClass called");
    return NULL;
}

void* CKWarnForIncorrectServiceIdentity(void)
{
    if (verbose) puts("STUB: CKWarnForIncorrectServiceIdentity called");
    return NULL;
}

void* CKWarnForInvalidApplicationIdentifier(void)
{
    if (verbose) puts("STUB: CKWarnForInvalidApplicationIdentifier called");
    return NULL;
}

void* CKWeakLinkClass(void)
{
    if (verbose) puts("STUB: CKWeakLinkClass called");
    return NULL;
}

void* CKWeakLinkSymbol(void)
{
    if (verbose) puts("STUB: CKWeakLinkSymbol called");
    return NULL;
}

void* CKXPCSuitableError(void)
{
    if (verbose) puts("STUB: CKXPCSuitableError called");
    return NULL;
}

void* NSTimeIntervalToClosestXPCActivityInterval(void)
{
    if (verbose) puts("STUB: NSTimeIntervalToClosestXPCActivityInterval called");
    return NULL;
}

void* _CKCheckArgument(void)
{
    if (verbose) puts("STUB: _CKCheckArgument called");
    return NULL;
}

void* _CKSimulateCrash(void)
{
    if (verbose) puts("STUB: _CKSimulateCrash called");
    return NULL;
}

void* ck_call_or_dispatch_async_if_not_key(void)
{
    if (verbose) puts("STUB: ck_call_or_dispatch_async_if_not_key called");
    return NULL;
}

void* ck_call_or_dispatch_sync_if_not_key(void)
{
    if (verbose) puts("STUB: ck_call_or_dispatch_sync_if_not_key called");
    return NULL;
}

void* ck_log_asl_level_to_type(void)
{
    if (verbose) puts("STUB: ck_log_asl_level_to_type called");
    return NULL;
}

void* ck_log_facilities_initailize_static(void)
{
    if (verbose) puts("STUB: ck_log_facilities_initailize_static called");
    return NULL;
}

void* ck_log_get_asl_level(void)
{
    if (verbose) puts("STUB: ck_log_get_asl_level called");
    return NULL;
}

void* mmapFileAtPath(void)
{
    if (verbose) puts("STUB: mmapFileAtPath called");
    return NULL;
}

void* mmapFileDescriptor(void)
{
    if (verbose) puts("STUB: mmapFileDescriptor called");
    return NULL;
}

void* setCKIsRunningInCloudD(void)
{
    if (verbose) puts("STUB: setCKIsRunningInCloudD called");
    return NULL;
}

void* setCKIsRunningInTestHost(void)
{
    if (verbose) puts("STUB: setCKIsRunningInTestHost called");
    return NULL;
}

void* stringForCKErrorCode(void)
{
    if (verbose) puts("STUB: stringForCKErrorCode called");
    return NULL;
}
