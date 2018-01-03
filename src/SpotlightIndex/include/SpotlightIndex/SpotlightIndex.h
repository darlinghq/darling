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


#ifndef _SpotlightIndex_H_
#define _SpotlightIndex_H_

#import <Foundation/Foundation.h>


void* RLEOIDArrayAddOid(void);
void* RLEOIDArrayAddOid_locked(void);
void* RLEOIDArrayAddOids(void);
void* RLEOIDArrayAddSortedOids(void);
void* RLEOIDArrayClear(void);
void* RLEOIDArrayContainsOid(void);
void* RLEOIDArrayContainsOid_locked(void);
void* RLEOIDArrayCreate(void);
void* RLEOIDArrayCreateMutable(void);
void* RLEOIDArrayCreateMutableCopy(void);
void* RLEOIDArrayDeserialize(void);
void* RLEOIDArrayFlatten(void);
void* RLEOIDArrayGetAllOids(void);
void* RLEOIDArrayIsEmpty(void);
void* RLEOIDArrayRemoveOid(void);
void* RLEOIDArrayRemoveOids(void);
void* RLEOIDArrayRemoveOidsWithExistence(void);
void* RLEOIDArrayRemoveSortedOids(void);
void* RLEOIDArraySerialize(void);
void* RLEOIDArrayUnflatten(void);
void* RLEOIDArrayValidate(void);
void* SIAddCorrection(void);
void* SIBulkDeleteAttributes(void);
void* SIBulkGetAttributeNames(void);
void* SIBulkGetAttributes(void);
void* SIBulkGetPath(void);
void* SIBulkSetAttributes(void);
void* SICalculateSize(void);
void* SICancel(void);
void* SICloseIndex(void);
void* SICopyCorrections(void);
void* SICopyProperties(void);
void* SICopyProperty(void);
void* SICorrectionCommit(void);
void* SICorrectionCreate(void);
void* SICorrectionDestory(void);
void* SICrashStringInit(void);
void* SICreateIndex(void);
void* SICreateNewIndex(void);
void* SIDeleteAttributes(void);
void* SIDeleteCSAttributes(void);
void* SIDeserialize(void);
void* SIExecuteQuery(void);
void* SIFlushAndSuspendIndex(void);
void* SIGetAccumulatedSizeForGroup(void);
void* SIGetAccumulatedStorageSizeForBundleId(void);
void* SIGetAttributes(void);
void* SIGetCSAttributes(void);
void* SIGetExceptionPort(void);
void* SIGetLargestOid(void);
void* SIGetMaxTransactionID(void);
void* SIGetPermCacheStatistics(void);
void* SIGetStoreCookie(void);
void* SIHasInFlightUpdates(void);
void* SIHasPersistentStoreCookie(void);
void* SIHealthCheckFiles(void);
void* SIIndexInactive(void);
void* SIIndexingFullyCaughtUp(void);
void* SIInitialIndexingEnded(void);
void* SIInitialIndexingStarted(void);
void* SIMatch(void);
void* SIMatcherCreate(void);
void* SIMatcherDispose(void);
void* SIMigrateIndex(void);
void* SIMoveDirectories(void);
void* SIMoveFiles(void);
void* SINotePermChange(void);
void* SIOpenIndex(void);
void* SIOpenIndexAtPathWithCallbacks(void);
void* SIPathFilterCreate(void);
void* SIPathFilterEvaluate(void);
void* SIPathFilterFilter(void);
void* SIPersistentIDStoreCheckAccessAndScopesForOIDs(void);
void* SIPersistentIDStoreCheckSandboxForOIDs(void);
void* SIPersistentIDStoreClose(void);
void* SIPersistentIDStoreCreate(void);
void* SIPersistentIDStoreGetCredential(void);
void* SIPersistentIDStoreGetFastPathForOid(void);
void* SIPersistentIDStoreGetOIDForCPath(void);
void* SIPersistentIDStoreGetOidPathForOid(void);
void* SIPersistentIDStoreGetOidPathForPath(void);
void* SIPersistentIDStoreGetParentForOid(void);
void* SIPersistentIDStoreGetRealPathForOid(void);
void* SIPersistentIDStoreGetTimeMachinePath(void);
void* SIPersistentIDStoreIsLocal(void);
void* SIPersistentIDStoreRename(void);
void* SIPersistentIDStoreSetRootPath(void);
void* SIPersistentIDStoreSupportsDirMap(void);
void* SIPersistentIDStoreSync(void);
void* SIPossibleMoves(void);
void* SIPossibleMovesBulk(void);
void* SIPreHeatIndex(void);
void* SIPreHeatPermissions(void);
void* SIProcessTerminating(void);
void* SIQueryAddResults(void);
void* SIQueryCreate(void);
void* SIQueryCreateWithItems(void);
void* SIQueryCreateWithItemsAndCompletionString(void);
void* SIQueryRelease(void);
void* SIQueryResolveFriendlyFieldNames(void);
void* SIQueryRetain(void);
void* SIQuerySetBatchingParameters(void);
void* SIQuerySetDominantRankingQueryCount(void);
void* SIQuerySetGetFriendlyFieldNamesMapCallback(void);
void* SIQuerySetStrongRankingQueryCount(void);
void* SIReaderWriterLockCreate(void);
void* SIReaderWriterLockDestroy(void);
void* SIReaderWriterLockReadLock(void);
void* SIReaderWriterLockReadUnlock(void);
void* SIReaderWriterLockWriteLock(void);
void* SIReaderWriterLockWriteUnlock(void);
void* SIRecomputeSizes(void);
void* SIRestoreCacheData(void);
void* SIResultBatchFree(void);
void* SIResultQueueCancel(void);
void* SIResultQueueCreate(void);
void* SIResultQueueDequeue(void);
void* SIResultQueueGetEnqueuedSize(void);
void* SIResultQueueGetLastPollTime(void);
void* SIResultQueueRelease(void);
void* SIResultQueueRetain(void);
void* SIResultQueueSetWakeupTrigger(void);
void* SIResumeForUnlock(void);
void* SIResumeIndex(void);
void* SISerialize(void);
void* SISerializeCacheData(void);
void* SISetAttributes(void);
void* SISetCSAttributes(void);
void* SISetCodedAttributes(void);
void* SISetCompletionRankingWeights(void);
void* SISetDefaultSchemaData(void);
void* SISetLogging(void);
void* SISetPowerState(void);
void* SISetProperty(void);
void* SISetScanCount(void);
void* SISetStoreCookie(void);
void* SISetTransactionCount(void);
void* SIShutdownIndex(void);
void* SISupportsFastScoping(void);
void* SISyncIndex(void);
void* SISyncIndexForLikelyShutdown(void);
void* SISyncIndexVaccuumIfNeeded(void);
void* SISynchedOp(void);
void* SISynchedOpPropagatingPriority(void);
void* SITokenizerCreate(void);
void* SITokenizerDispose(void);
void* SITokenizerGetTokenRanges(void);
void* SITokenizerGetTokens(void);
void* SITransferAttributes(void);
void* SIUserCtxAllowPermissionCheck(void);
void* SIUserCtxCreate(void);
void* SIUserCtxCreateWithLanguages(void);
void* SIUserCtxGetArgumentDictionary(void);
void* SIUserCtxGetAuditToken(void);
void* SIUserCtxGetCredential(void);
void* SIUserCtxGetLanguageArray(void);
void* SIUserCtxGetSchema(void);
void* SIUserCtxHasCredential(void);
void* SIUserCtxIsSandboxed(void);
void* SIUserCtxRemoveCredential(void);
void* SIUserCtxSetArgumentDictionary(void);
void* SIValidIndex(void);
void* SIWaitResults(void);
void* SI_PersistentIDStoreGetTypeID(void);
void* _SICheckIndexForDuplicateOids(void);
void* _SICleanupStrings(void);
void* _SIConsistencyCheck(void);
void* _SICooldownIndexFilesForMerge(void);
void* _SICopyPropertyIDMap(void);
void* _SIDirectoryStoreIterate(void);
void* _SIFlushAndSyncIndex(void);
void* _SIGetAllOids(void);
void* _SIGetErrorString(void);
void* _SIGetLastUsedDate(void);
void* _SIGetRecoverTimeStamp(void);
void* _SIIssueCacheCleanup(void);
void* _SIIssueFlush(void);
void* _SIIssueFullMerge(void);
void* _SIIssueFullMergeWithGroup(void);
void* _SIIssueMerge(void);
void* _SIIssueRecovery(void);
void* _SIIssueSchedulerDump(void);
void* _SIIssueSizeRepair(void);
void* _SIIssueSplit(void);
void* _SIIssueVerify(void);
void* _SIMinimizeDiskSpace(void);
void* _SIOpenIndexFilesForMerge(void);
void* _SIPreHeatPermissions(void);
void* _SIQuerySetTMNameSpaces(void);
void* _SIReverseStoreIterate(void);
void* _SIScanQueuesDrained(void);
void* _SIScheduleBackgroundOperation(void);
void* _SIScheduleDiskOperation(void);
void* _SIScheduleOperation(void);
void* _SISchedulerCopyDump(void);
void* _SISetContactsIndex(void);
void* _SISetLastUsedDate(void);
void* _SIStartPreheatScheduler(void);
void* _SITermIndexIterate(void);
void* qsort_oids(void);
void* si_textcache_fds_for_oids(void);

#endif
