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


#include <SpotlightIndex/SpotlightIndex.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* RLEOIDArrayAddOid(void) {
    if (verbose) puts("STUB: RLEOIDArrayAddOid called");
    return NULL;
}

void* RLEOIDArrayAddOid_locked(void) {
    if (verbose) puts("STUB: RLEOIDArrayAddOid_locked called");
    return NULL;
}

void* RLEOIDArrayAddOids(void) {
    if (verbose) puts("STUB: RLEOIDArrayAddOids called");
    return NULL;
}

void* RLEOIDArrayAddSortedOids(void) {
    if (verbose) puts("STUB: RLEOIDArrayAddSortedOids called");
    return NULL;
}

void* RLEOIDArrayClear(void) {
    if (verbose) puts("STUB: RLEOIDArrayClear called");
    return NULL;
}

void* RLEOIDArrayContainsOid(void) {
    if (verbose) puts("STUB: RLEOIDArrayContainsOid called");
    return NULL;
}

void* RLEOIDArrayContainsOid_locked(void) {
    if (verbose) puts("STUB: RLEOIDArrayContainsOid_locked called");
    return NULL;
}

void* RLEOIDArrayCreate(void) {
    if (verbose) puts("STUB: RLEOIDArrayCreate called");
    return NULL;
}

void* RLEOIDArrayCreateMutable(void) {
    if (verbose) puts("STUB: RLEOIDArrayCreateMutable called");
    return NULL;
}

void* RLEOIDArrayCreateMutableCopy(void) {
    if (verbose) puts("STUB: RLEOIDArrayCreateMutableCopy called");
    return NULL;
}

void* RLEOIDArrayDeserialize(void) {
    if (verbose) puts("STUB: RLEOIDArrayDeserialize called");
    return NULL;
}

void* RLEOIDArrayFlatten(void) {
    if (verbose) puts("STUB: RLEOIDArrayFlatten called");
    return NULL;
}

void* RLEOIDArrayGetAllOids(void) {
    if (verbose) puts("STUB: RLEOIDArrayGetAllOids called");
    return NULL;
}

void* RLEOIDArrayIsEmpty(void) {
    if (verbose) puts("STUB: RLEOIDArrayIsEmpty called");
    return NULL;
}

void* RLEOIDArrayRemoveOid(void) {
    if (verbose) puts("STUB: RLEOIDArrayRemoveOid called");
    return NULL;
}

void* RLEOIDArrayRemoveOids(void) {
    if (verbose) puts("STUB: RLEOIDArrayRemoveOids called");
    return NULL;
}

void* RLEOIDArrayRemoveOidsWithExistence(void) {
    if (verbose) puts("STUB: RLEOIDArrayRemoveOidsWithExistence called");
    return NULL;
}

void* RLEOIDArrayRemoveSortedOids(void) {
    if (verbose) puts("STUB: RLEOIDArrayRemoveSortedOids called");
    return NULL;
}

void* RLEOIDArraySerialize(void) {
    if (verbose) puts("STUB: RLEOIDArraySerialize called");
    return NULL;
}

void* RLEOIDArrayUnflatten(void) {
    if (verbose) puts("STUB: RLEOIDArrayUnflatten called");
    return NULL;
}

void* RLEOIDArrayValidate(void) {
    if (verbose) puts("STUB: RLEOIDArrayValidate called");
    return NULL;
}

void* SIAddCorrection(void) {
    if (verbose) puts("STUB: SIAddCorrection called");
    return NULL;
}

void* SIBulkDeleteAttributes(void) {
    if (verbose) puts("STUB: SIBulkDeleteAttributes called");
    return NULL;
}

void* SIBulkGetAttributeNames(void) {
    if (verbose) puts("STUB: SIBulkGetAttributeNames called");
    return NULL;
}

void* SIBulkGetAttributes(void) {
    if (verbose) puts("STUB: SIBulkGetAttributes called");
    return NULL;
}

void* SIBulkGetPath(void) {
    if (verbose) puts("STUB: SIBulkGetPath called");
    return NULL;
}

void* SIBulkSetAttributes(void) {
    if (verbose) puts("STUB: SIBulkSetAttributes called");
    return NULL;
}

void* SICalculateSize(void) {
    if (verbose) puts("STUB: SICalculateSize called");
    return NULL;
}

void* SICancel(void) {
    if (verbose) puts("STUB: SICancel called");
    return NULL;
}

void* SICloseIndex(void) {
    if (verbose) puts("STUB: SICloseIndex called");
    return NULL;
}

void* SICopyCorrections(void) {
    if (verbose) puts("STUB: SICopyCorrections called");
    return NULL;
}

void* SICopyProperties(void) {
    if (verbose) puts("STUB: SICopyProperties called");
    return NULL;
}

void* SICopyProperty(void) {
    if (verbose) puts("STUB: SICopyProperty called");
    return NULL;
}

void* SICorrectionCommit(void) {
    if (verbose) puts("STUB: SICorrectionCommit called");
    return NULL;
}

void* SICorrectionCreate(void) {
    if (verbose) puts("STUB: SICorrectionCreate called");
    return NULL;
}

void* SICorrectionDestory(void) {
    if (verbose) puts("STUB: SICorrectionDestory called");
    return NULL;
}

void* SICrashStringInit(void) {
    if (verbose) puts("STUB: SICrashStringInit called");
    return NULL;
}

void* SICreateIndex(void) {
    if (verbose) puts("STUB: SICreateIndex called");
    return NULL;
}

void* SICreateNewIndex(void) {
    if (verbose) puts("STUB: SICreateNewIndex called");
    return NULL;
}

void* SIDeleteAttributes(void) {
    if (verbose) puts("STUB: SIDeleteAttributes called");
    return NULL;
}

void* SIDeleteCSAttributes(void) {
    if (verbose) puts("STUB: SIDeleteCSAttributes called");
    return NULL;
}

void* SIDeserialize(void) {
    if (verbose) puts("STUB: SIDeserialize called");
    return NULL;
}

void* SIExecuteQuery(void) {
    if (verbose) puts("STUB: SIExecuteQuery called");
    return NULL;
}

void* SIFlushAndSuspendIndex(void) {
    if (verbose) puts("STUB: SIFlushAndSuspendIndex called");
    return NULL;
}

void* SIGetAccumulatedSizeForGroup(void) {
    if (verbose) puts("STUB: SIGetAccumulatedSizeForGroup called");
    return NULL;
}

void* SIGetAccumulatedStorageSizeForBundleId(void) {
    if (verbose) puts("STUB: SIGetAccumulatedStorageSizeForBundleId called");
    return NULL;
}

void* SIGetAttributes(void) {
    if (verbose) puts("STUB: SIGetAttributes called");
    return NULL;
}

void* SIGetCSAttributes(void) {
    if (verbose) puts("STUB: SIGetCSAttributes called");
    return NULL;
}

void* SIGetExceptionPort(void) {
    if (verbose) puts("STUB: SIGetExceptionPort called");
    return NULL;
}

void* SIGetLargestOid(void) {
    if (verbose) puts("STUB: SIGetLargestOid called");
    return NULL;
}

void* SIGetMaxTransactionID(void) {
    if (verbose) puts("STUB: SIGetMaxTransactionID called");
    return NULL;
}

void* SIGetPermCacheStatistics(void) {
    if (verbose) puts("STUB: SIGetPermCacheStatistics called");
    return NULL;
}

void* SIGetStoreCookie(void) {
    if (verbose) puts("STUB: SIGetStoreCookie called");
    return NULL;
}

void* SIHasInFlightUpdates(void) {
    if (verbose) puts("STUB: SIHasInFlightUpdates called");
    return NULL;
}

void* SIHasPersistentStoreCookie(void) {
    if (verbose) puts("STUB: SIHasPersistentStoreCookie called");
    return NULL;
}

void* SIHealthCheckFiles(void) {
    if (verbose) puts("STUB: SIHealthCheckFiles called");
    return NULL;
}

void* SIIndexInactive(void) {
    if (verbose) puts("STUB: SIIndexInactive called");
    return NULL;
}

void* SIIndexingFullyCaughtUp(void) {
    if (verbose) puts("STUB: SIIndexingFullyCaughtUp called");
    return NULL;
}

void* SIInitialIndexingEnded(void) {
    if (verbose) puts("STUB: SIInitialIndexingEnded called");
    return NULL;
}

void* SIInitialIndexingStarted(void) {
    if (verbose) puts("STUB: SIInitialIndexingStarted called");
    return NULL;
}

void* SIMatch(void) {
    if (verbose) puts("STUB: SIMatch called");
    return NULL;
}

void* SIMatcherCreate(void) {
    if (verbose) puts("STUB: SIMatcherCreate called");
    return NULL;
}

void* SIMatcherDispose(void) {
    if (verbose) puts("STUB: SIMatcherDispose called");
    return NULL;
}

void* SIMigrateIndex(void) {
    if (verbose) puts("STUB: SIMigrateIndex called");
    return NULL;
}

void* SIMoveDirectories(void) {
    if (verbose) puts("STUB: SIMoveDirectories called");
    return NULL;
}

void* SIMoveFiles(void) {
    if (verbose) puts("STUB: SIMoveFiles called");
    return NULL;
}

void* SINotePermChange(void) {
    if (verbose) puts("STUB: SINotePermChange called");
    return NULL;
}

void* SIOpenIndex(void) {
    if (verbose) puts("STUB: SIOpenIndex called");
    return NULL;
}

void* SIOpenIndexAtPathWithCallbacks(void) {
    if (verbose) puts("STUB: SIOpenIndexAtPathWithCallbacks called");
    return NULL;
}

void* SIPathFilterCreate(void) {
    if (verbose) puts("STUB: SIPathFilterCreate called");
    return NULL;
}

void* SIPathFilterEvaluate(void) {
    if (verbose) puts("STUB: SIPathFilterEvaluate called");
    return NULL;
}

void* SIPathFilterFilter(void) {
    if (verbose) puts("STUB: SIPathFilterFilter called");
    return NULL;
}

void* SIPersistentIDStoreCheckAccessAndScopesForOIDs(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreCheckAccessAndScopesForOIDs called");
    return NULL;
}

void* SIPersistentIDStoreCheckSandboxForOIDs(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreCheckSandboxForOIDs called");
    return NULL;
}

void* SIPersistentIDStoreClose(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreClose called");
    return NULL;
}

void* SIPersistentIDStoreCreate(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreCreate called");
    return NULL;
}

void* SIPersistentIDStoreGetCredential(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetCredential called");
    return NULL;
}

void* SIPersistentIDStoreGetFastPathForOid(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetFastPathForOid called");
    return NULL;
}

void* SIPersistentIDStoreGetOIDForCPath(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetOIDForCPath called");
    return NULL;
}

void* SIPersistentIDStoreGetOidPathForOid(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetOidPathForOid called");
    return NULL;
}

void* SIPersistentIDStoreGetOidPathForPath(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetOidPathForPath called");
    return NULL;
}

void* SIPersistentIDStoreGetParentForOid(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetParentForOid called");
    return NULL;
}

void* SIPersistentIDStoreGetRealPathForOid(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetRealPathForOid called");
    return NULL;
}

void* SIPersistentIDStoreGetTimeMachinePath(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreGetTimeMachinePath called");
    return NULL;
}

void* SIPersistentIDStoreIsLocal(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreIsLocal called");
    return NULL;
}

void* SIPersistentIDStoreRename(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreRename called");
    return NULL;
}

void* SIPersistentIDStoreSetRootPath(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreSetRootPath called");
    return NULL;
}

void* SIPersistentIDStoreSupportsDirMap(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreSupportsDirMap called");
    return NULL;
}

void* SIPersistentIDStoreSync(void) {
    if (verbose) puts("STUB: SIPersistentIDStoreSync called");
    return NULL;
}

void* SIPossibleMoves(void) {
    if (verbose) puts("STUB: SIPossibleMoves called");
    return NULL;
}

void* SIPossibleMovesBulk(void) {
    if (verbose) puts("STUB: SIPossibleMovesBulk called");
    return NULL;
}

void* SIPreHeatIndex(void) {
    if (verbose) puts("STUB: SIPreHeatIndex called");
    return NULL;
}

void* SIPreHeatPermissions(void) {
    if (verbose) puts("STUB: SIPreHeatPermissions called");
    return NULL;
}

void* SIProcessTerminating(void) {
    if (verbose) puts("STUB: SIProcessTerminating called");
    return NULL;
}

void* SIQueryAddResults(void) {
    if (verbose) puts("STUB: SIQueryAddResults called");
    return NULL;
}

void* SIQueryCreate(void) {
    if (verbose) puts("STUB: SIQueryCreate called");
    return NULL;
}

void* SIQueryCreateWithItems(void) {
    if (verbose) puts("STUB: SIQueryCreateWithItems called");
    return NULL;
}

void* SIQueryCreateWithItemsAndCompletionString(void) {
    if (verbose) puts("STUB: SIQueryCreateWithItemsAndCompletionString called");
    return NULL;
}

void* SIQueryRelease(void) {
    if (verbose) puts("STUB: SIQueryRelease called");
    return NULL;
}

void* SIQueryResolveFriendlyFieldNames(void) {
    if (verbose) puts("STUB: SIQueryResolveFriendlyFieldNames called");
    return NULL;
}

void* SIQueryRetain(void) {
    if (verbose) puts("STUB: SIQueryRetain called");
    return NULL;
}

void* SIQuerySetBatchingParameters(void) {
    if (verbose) puts("STUB: SIQuerySetBatchingParameters called");
    return NULL;
}

void* SIQuerySetDominantRankingQueryCount(void) {
    if (verbose) puts("STUB: SIQuerySetDominantRankingQueryCount called");
    return NULL;
}

void* SIQuerySetGetFriendlyFieldNamesMapCallback(void) {
    if (verbose) puts("STUB: SIQuerySetGetFriendlyFieldNamesMapCallback called");
    return NULL;
}

void* SIQuerySetStrongRankingQueryCount(void) {
    if (verbose) puts("STUB: SIQuerySetStrongRankingQueryCount called");
    return NULL;
}

void* SIReaderWriterLockCreate(void) {
    if (verbose) puts("STUB: SIReaderWriterLockCreate called");
    return NULL;
}

void* SIReaderWriterLockDestroy(void) {
    if (verbose) puts("STUB: SIReaderWriterLockDestroy called");
    return NULL;
}

void* SIReaderWriterLockReadLock(void) {
    if (verbose) puts("STUB: SIReaderWriterLockReadLock called");
    return NULL;
}

void* SIReaderWriterLockReadUnlock(void) {
    if (verbose) puts("STUB: SIReaderWriterLockReadUnlock called");
    return NULL;
}

void* SIReaderWriterLockWriteLock(void) {
    if (verbose) puts("STUB: SIReaderWriterLockWriteLock called");
    return NULL;
}

void* SIReaderWriterLockWriteUnlock(void) {
    if (verbose) puts("STUB: SIReaderWriterLockWriteUnlock called");
    return NULL;
}

void* SIRecomputeSizes(void) {
    if (verbose) puts("STUB: SIRecomputeSizes called");
    return NULL;
}

void* SIRestoreCacheData(void) {
    if (verbose) puts("STUB: SIRestoreCacheData called");
    return NULL;
}

void* SIResultBatchFree(void) {
    if (verbose) puts("STUB: SIResultBatchFree called");
    return NULL;
}

void* SIResultQueueCancel(void) {
    if (verbose) puts("STUB: SIResultQueueCancel called");
    return NULL;
}

void* SIResultQueueCreate(void) {
    if (verbose) puts("STUB: SIResultQueueCreate called");
    return NULL;
}

void* SIResultQueueDequeue(void) {
    if (verbose) puts("STUB: SIResultQueueDequeue called");
    return NULL;
}

void* SIResultQueueGetEnqueuedSize(void) {
    if (verbose) puts("STUB: SIResultQueueGetEnqueuedSize called");
    return NULL;
}

void* SIResultQueueGetLastPollTime(void) {
    if (verbose) puts("STUB: SIResultQueueGetLastPollTime called");
    return NULL;
}

void* SIResultQueueRelease(void) {
    if (verbose) puts("STUB: SIResultQueueRelease called");
    return NULL;
}

void* SIResultQueueRetain(void) {
    if (verbose) puts("STUB: SIResultQueueRetain called");
    return NULL;
}

void* SIResultQueueSetWakeupTrigger(void) {
    if (verbose) puts("STUB: SIResultQueueSetWakeupTrigger called");
    return NULL;
}

void* SIResumeForUnlock(void) {
    if (verbose) puts("STUB: SIResumeForUnlock called");
    return NULL;
}

void* SIResumeIndex(void) {
    if (verbose) puts("STUB: SIResumeIndex called");
    return NULL;
}

void* SISerialize(void) {
    if (verbose) puts("STUB: SISerialize called");
    return NULL;
}

void* SISerializeCacheData(void) {
    if (verbose) puts("STUB: SISerializeCacheData called");
    return NULL;
}

void* SISetAttributes(void) {
    if (verbose) puts("STUB: SISetAttributes called");
    return NULL;
}

void* SISetCSAttributes(void) {
    if (verbose) puts("STUB: SISetCSAttributes called");
    return NULL;
}

void* SISetCodedAttributes(void) {
    if (verbose) puts("STUB: SISetCodedAttributes called");
    return NULL;
}

void* SISetCompletionRankingWeights(void) {
    if (verbose) puts("STUB: SISetCompletionRankingWeights called");
    return NULL;
}

void* SISetDefaultSchemaData(void) {
    if (verbose) puts("STUB: SISetDefaultSchemaData called");
    return NULL;
}

void* SISetLogging(void) {
    if (verbose) puts("STUB: SISetLogging called");
    return NULL;
}

void* SISetPowerState(void) {
    if (verbose) puts("STUB: SISetPowerState called");
    return NULL;
}

void* SISetProperty(void) {
    if (verbose) puts("STUB: SISetProperty called");
    return NULL;
}

void* SISetScanCount(void) {
    if (verbose) puts("STUB: SISetScanCount called");
    return NULL;
}

void* SISetStoreCookie(void) {
    if (verbose) puts("STUB: SISetStoreCookie called");
    return NULL;
}

void* SISetTransactionCount(void) {
    if (verbose) puts("STUB: SISetTransactionCount called");
    return NULL;
}

void* SIShutdownIndex(void) {
    if (verbose) puts("STUB: SIShutdownIndex called");
    return NULL;
}

void* SISupportsFastScoping(void) {
    if (verbose) puts("STUB: SISupportsFastScoping called");
    return NULL;
}

void* SISyncIndex(void) {
    if (verbose) puts("STUB: SISyncIndex called");
    return NULL;
}

void* SISyncIndexForLikelyShutdown(void) {
    if (verbose) puts("STUB: SISyncIndexForLikelyShutdown called");
    return NULL;
}

void* SISyncIndexVaccuumIfNeeded(void) {
    if (verbose) puts("STUB: SISyncIndexVaccuumIfNeeded called");
    return NULL;
}

void* SISynchedOp(void) {
    if (verbose) puts("STUB: SISynchedOp called");
    return NULL;
}

void* SISynchedOpPropagatingPriority(void) {
    if (verbose) puts("STUB: SISynchedOpPropagatingPriority called");
    return NULL;
}

void* SITokenizerCreate(void) {
    if (verbose) puts("STUB: SITokenizerCreate called");
    return NULL;
}

void* SITokenizerDispose(void) {
    if (verbose) puts("STUB: SITokenizerDispose called");
    return NULL;
}

void* SITokenizerGetTokenRanges(void) {
    if (verbose) puts("STUB: SITokenizerGetTokenRanges called");
    return NULL;
}

void* SITokenizerGetTokens(void) {
    if (verbose) puts("STUB: SITokenizerGetTokens called");
    return NULL;
}

void* SITransferAttributes(void) {
    if (verbose) puts("STUB: SITransferAttributes called");
    return NULL;
}

void* SIUserCtxAllowPermissionCheck(void) {
    if (verbose) puts("STUB: SIUserCtxAllowPermissionCheck called");
    return NULL;
}

void* SIUserCtxCreate(void) {
    if (verbose) puts("STUB: SIUserCtxCreate called");
    return NULL;
}

void* SIUserCtxCreateWithLanguages(void) {
    if (verbose) puts("STUB: SIUserCtxCreateWithLanguages called");
    return NULL;
}

void* SIUserCtxGetArgumentDictionary(void) {
    if (verbose) puts("STUB: SIUserCtxGetArgumentDictionary called");
    return NULL;
}

void* SIUserCtxGetAuditToken(void) {
    if (verbose) puts("STUB: SIUserCtxGetAuditToken called");
    return NULL;
}

void* SIUserCtxGetCredential(void) {
    if (verbose) puts("STUB: SIUserCtxGetCredential called");
    return NULL;
}

void* SIUserCtxGetLanguageArray(void) {
    if (verbose) puts("STUB: SIUserCtxGetLanguageArray called");
    return NULL;
}

void* SIUserCtxGetSchema(void) {
    if (verbose) puts("STUB: SIUserCtxGetSchema called");
    return NULL;
}

void* SIUserCtxHasCredential(void) {
    if (verbose) puts("STUB: SIUserCtxHasCredential called");
    return NULL;
}

void* SIUserCtxIsSandboxed(void) {
    if (verbose) puts("STUB: SIUserCtxIsSandboxed called");
    return NULL;
}

void* SIUserCtxRemoveCredential(void) {
    if (verbose) puts("STUB: SIUserCtxRemoveCredential called");
    return NULL;
}

void* SIUserCtxSetArgumentDictionary(void) {
    if (verbose) puts("STUB: SIUserCtxSetArgumentDictionary called");
    return NULL;
}

void* SIValidIndex(void) {
    if (verbose) puts("STUB: SIValidIndex called");
    return NULL;
}

void* SIWaitResults(void) {
    if (verbose) puts("STUB: SIWaitResults called");
    return NULL;
}

void* SI_PersistentIDStoreGetTypeID(void) {
    if (verbose) puts("STUB: SI_PersistentIDStoreGetTypeID called");
    return NULL;
}

void* _SICheckIndexForDuplicateOids(void) {
    if (verbose) puts("STUB: _SICheckIndexForDuplicateOids called");
    return NULL;
}

void* _SICleanupStrings(void) {
    if (verbose) puts("STUB: _SICleanupStrings called");
    return NULL;
}

void* _SIConsistencyCheck(void) {
    if (verbose) puts("STUB: _SIConsistencyCheck called");
    return NULL;
}

void* _SICooldownIndexFilesForMerge(void) {
    if (verbose) puts("STUB: _SICooldownIndexFilesForMerge called");
    return NULL;
}

void* _SICopyPropertyIDMap(void) {
    if (verbose) puts("STUB: _SICopyPropertyIDMap called");
    return NULL;
}

void* _SIDirectoryStoreIterate(void) {
    if (verbose) puts("STUB: _SIDirectoryStoreIterate called");
    return NULL;
}

void* _SIFlushAndSyncIndex(void) {
    if (verbose) puts("STUB: _SIFlushAndSyncIndex called");
    return NULL;
}

void* _SIGetAllOids(void) {
    if (verbose) puts("STUB: _SIGetAllOids called");
    return NULL;
}

void* _SIGetErrorString(void) {
    if (verbose) puts("STUB: _SIGetErrorString called");
    return NULL;
}

void* _SIGetLastUsedDate(void) {
    if (verbose) puts("STUB: _SIGetLastUsedDate called");
    return NULL;
}

void* _SIGetRecoverTimeStamp(void) {
    if (verbose) puts("STUB: _SIGetRecoverTimeStamp called");
    return NULL;
}

void* _SIIssueCacheCleanup(void) {
    if (verbose) puts("STUB: _SIIssueCacheCleanup called");
    return NULL;
}

void* _SIIssueFlush(void) {
    if (verbose) puts("STUB: _SIIssueFlush called");
    return NULL;
}

void* _SIIssueFullMerge(void) {
    if (verbose) puts("STUB: _SIIssueFullMerge called");
    return NULL;
}

void* _SIIssueFullMergeWithGroup(void) {
    if (verbose) puts("STUB: _SIIssueFullMergeWithGroup called");
    return NULL;
}

void* _SIIssueMerge(void) {
    if (verbose) puts("STUB: _SIIssueMerge called");
    return NULL;
}

void* _SIIssueRecovery(void) {
    if (verbose) puts("STUB: _SIIssueRecovery called");
    return NULL;
}

void* _SIIssueSchedulerDump(void) {
    if (verbose) puts("STUB: _SIIssueSchedulerDump called");
    return NULL;
}

void* _SIIssueSizeRepair(void) {
    if (verbose) puts("STUB: _SIIssueSizeRepair called");
    return NULL;
}

void* _SIIssueSplit(void) {
    if (verbose) puts("STUB: _SIIssueSplit called");
    return NULL;
}

void* _SIIssueVerify(void) {
    if (verbose) puts("STUB: _SIIssueVerify called");
    return NULL;
}

void* _SIMinimizeDiskSpace(void) {
    if (verbose) puts("STUB: _SIMinimizeDiskSpace called");
    return NULL;
}

void* _SIOpenIndexFilesForMerge(void) {
    if (verbose) puts("STUB: _SIOpenIndexFilesForMerge called");
    return NULL;
}

void* _SIPreHeatPermissions(void) {
    if (verbose) puts("STUB: _SIPreHeatPermissions called");
    return NULL;
}

void* _SIQuerySetTMNameSpaces(void) {
    if (verbose) puts("STUB: _SIQuerySetTMNameSpaces called");
    return NULL;
}

void* _SIReverseStoreIterate(void) {
    if (verbose) puts("STUB: _SIReverseStoreIterate called");
    return NULL;
}

void* _SIScanQueuesDrained(void) {
    if (verbose) puts("STUB: _SIScanQueuesDrained called");
    return NULL;
}

void* _SIScheduleBackgroundOperation(void) {
    if (verbose) puts("STUB: _SIScheduleBackgroundOperation called");
    return NULL;
}

void* _SIScheduleDiskOperation(void) {
    if (verbose) puts("STUB: _SIScheduleDiskOperation called");
    return NULL;
}

void* _SIScheduleOperation(void) {
    if (verbose) puts("STUB: _SIScheduleOperation called");
    return NULL;
}

void* _SISchedulerCopyDump(void) {
    if (verbose) puts("STUB: _SISchedulerCopyDump called");
    return NULL;
}

void* _SISetContactsIndex(void) {
    if (verbose) puts("STUB: _SISetContactsIndex called");
    return NULL;
}

void* _SISetLastUsedDate(void) {
    if (verbose) puts("STUB: _SISetLastUsedDate called");
    return NULL;
}

void* _SIStartPreheatScheduler(void) {
    if (verbose) puts("STUB: _SIStartPreheatScheduler called");
    return NULL;
}

void* _SITermIndexIterate(void) {
    if (verbose) puts("STUB: _SITermIndexIterate called");
    return NULL;
}

void* qsort_oids(void) {
    if (verbose) puts("STUB: qsort_oids called");
    return NULL;
}

void* si_textcache_fds_for_oids(void) {
    if (verbose) puts("STUB: si_textcache_fds_for_oids called");
    return NULL;
}
