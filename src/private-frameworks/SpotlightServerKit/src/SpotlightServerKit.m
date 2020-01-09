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


#include <SpotlightServerKit/SpotlightServerKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* BWA_Create(void) {
    if (verbose) puts("STUB: BWA_Create called");
    return NULL;
}

void* BWA_DataAllocate(void) {
    if (verbose) puts("STUB: BWA_DataAllocate called");
    return NULL;
}

void* BWA_DataDeallocate(void) {
    if (verbose) puts("STUB: BWA_DataDeallocate called");
    return NULL;
}

void* CFDataCreateWithContentsOfURL(void) {
    if (verbose) puts("STUB: CFDataCreateWithContentsOfURL called");
    return NULL;
}

void* CF_MaxRange(void) {
    if (verbose) puts("STUB: CF_MaxRange called");
    return NULL;
}

void* CF_RangeIntersection(void) {
    if (verbose) puts("STUB: CF_RangeIntersection called");
    return NULL;
}

void* CF_RangeUnion(void) {
    if (verbose) puts("STUB: CF_RangeUnion called");
    return NULL;
}

void* GetSystemBuildVersionString(void) {
    if (verbose) puts("STUB: GetSystemBuildVersionString called");
    return NULL;
}

void* MDAddNanoseconds(void) {
    if (verbose) puts("STUB: MDAddNanoseconds called");
    return NULL;
}

void* MDArrayCreateMutable(void) {
    if (verbose) puts("STUB: MDArrayCreateMutable called");
    return NULL;
}

void* MDArrayIterate(void) {
    if (verbose) puts("STUB: MDArrayIterate called");
    return NULL;
}

void* MDCopyMachPortDescription(void) {
    if (verbose) puts("STUB: MDCopyMachPortDescription called");
    return NULL;
}

void* MDDictionaryCreateMutable(void) {
    if (verbose) puts("STUB: MDDictionaryCreateMutable called");
    return NULL;
}

void* MDDictionaryIterate(void) {
    if (verbose) puts("STUB: MDDictionaryIterate called");
    return NULL;
}

void* MDElapsedTime(void) {
    if (verbose) puts("STUB: MDElapsedTime called");
    return NULL;
}

void* MDGetCurrentTimeSpec(void) {
    if (verbose) puts("STUB: MDGetCurrentTimeSpec called");
    return NULL;
}

void* MDGetPhysicalRAMSize(void) {
    if (verbose) puts("STUB: MDGetPhysicalRAMSize called");
    return NULL;
}

void* MDGetProcessIOPolicy(void) {
    if (verbose) puts("STUB: MDGetProcessIOPolicy called");
    return NULL;
}

void* MDGetThreadIOPolicy(void) {
    if (verbose) puts("STUB: MDGetThreadIOPolicy called");
    return NULL;
}

void* MDHash(void) {
    if (verbose) puts("STUB: MDHash called");
    return NULL;
}

void* MDHashQuick32(void) {
    if (verbose) puts("STUB: MDHashQuick32 called");
    return NULL;
}

void* MDHashQuick32Pair(void) {
    if (verbose) puts("STUB: MDHashQuick32Pair called");
    return NULL;
}

void* MDHashQuick64(void) {
    if (verbose) puts("STUB: MDHashQuick64 called");
    return NULL;
}

void* MDHashQuickPtrAsInt(void) {
    if (verbose) puts("STUB: MDHashQuickPtrAsInt called");
    return NULL;
}

void* MDKillTask(void) {
    if (verbose) puts("STUB: MDKillTask called");
    return NULL;
}

void* MDMachAbsoluteTimeDiff(void) {
    if (verbose) puts("STUB: MDMachAbsoluteTimeDiff called");
    return NULL;
}

void* MDNumberCreateDouble(void) {
    if (verbose) puts("STUB: MDNumberCreateDouble called");
    return NULL;
}

void* MDNumberCreateInt32(void) {
    if (verbose) puts("STUB: MDNumberCreateInt32 called");
    return NULL;
}

void* MDNumberCreateInt64(void) {
    if (verbose) puts("STUB: MDNumberCreateInt64 called");
    return NULL;
}

void* MDNumberGetDouble(void) {
    if (verbose) puts("STUB: MDNumberGetDouble called");
    return NULL;
}

void* MDNumberGetInt32(void) {
    if (verbose) puts("STUB: MDNumberGetInt32 called");
    return NULL;
}

void* MDNumberGetInt64(void) {
    if (verbose) puts("STUB: MDNumberGetInt64 called");
    return NULL;
}

void* MDPowerInfoCollect(void) {
    if (verbose) puts("STUB: MDPowerInfoCollect called");
    return NULL;
}

void* MDPowerInfoDifference(void) {
    if (verbose) puts("STUB: MDPowerInfoDifference called");
    return NULL;
}

void* MDPowerInfoEnd(void) {
    if (verbose) puts("STUB: MDPowerInfoEnd called");
    return NULL;
}

void* MDPowerInfoSum(void) {
    if (verbose) puts("STUB: MDPowerInfoSum called");
    return NULL;
}

void* MDRunningInRosetta(void) {
    if (verbose) puts("STUB: MDRunningInRosetta called");
    return NULL;
}

void* MDRunningIntel32(void) {
    if (verbose) puts("STUB: MDRunningIntel32 called");
    return NULL;
}

void* MDRunningIntel64(void) {
    if (verbose) puts("STUB: MDRunningIntel64 called");
    return NULL;
}

void* MDRunningOnEffectivePPC(void) {
    if (verbose) puts("STUB: MDRunningOnEffectivePPC called");
    return NULL;
}

void* MDRunningOnPhysicalIntel(void) {
    if (verbose) puts("STUB: MDRunningOnPhysicalIntel called");
    return NULL;
}

void* MDRunningOnPhysicalPPC(void) {
    if (verbose) puts("STUB: MDRunningOnPhysicalPPC called");
    return NULL;
}

void* MDSGetSystemLoadAdvisory(void) {
    if (verbose) puts("STUB: MDSGetSystemLoadAdvisory called");
    return NULL;
}

void* MDSLogLevelCreateLogArrayDictionary(void) {
    if (verbose) puts("STUB: MDSLogLevelCreateLogArrayDictionary called");
    return NULL;
}

void* MDSLogLoadLogLevelArrayFromDefaults(void) {
    if (verbose) puts("STUB: MDSLogLoadLogLevelArrayFromDefaults called");
    return NULL;
}

void* MDSLogLoadLogLevelsFromDefaults(void) {
    if (verbose) puts("STUB: MDSLogLoadLogLevelsFromDefaults called");
    return NULL;
}

void* MDSLogParseLogArrayDictionary(void) {
    if (verbose) puts("STUB: MDSLogParseLogArrayDictionary called");
    return NULL;
}

void* MDSLogSaveLogLevelArrayToDefaults(void) {
    if (verbose) puts("STUB: MDSLogSaveLogLevelArrayToDefaults called");
    return NULL;
}

void* MDSLogSaveLogLevelsToDefaults(void) {
    if (verbose) puts("STUB: MDSLogSaveLogLevelsToDefaults called");
    return NULL;
}

void* MDSLogSetLoggingOutputFunction(void) {
    if (verbose) puts("STUB: MDSLogSetLoggingOutputFunction called");
    return NULL;
}

void* MDSLogSubsumeNSLog(void) {
    if (verbose) puts("STUB: MDSLogSubsumeNSLog called");
    return NULL;
}

void* MDSMessageTracerEvent(void) {
    if (verbose) puts("STUB: MDSMessageTracerEvent called");
    return NULL;
}

void* MDSObjectRegistryResolveObjectWithPort(void) {
    if (verbose) puts("STUB: MDSObjectRegistryResolveObjectWithPort called");
    return NULL;
}

void* MDSQueueRegistryCopyQueueForToken(void) {
    if (verbose) puts("STUB: MDSQueueRegistryCopyQueueForToken called");
    return NULL;
}

void* MDSQueueRegistryRegisterQueue(void) {
    if (verbose) puts("STUB: MDSQueueRegistryRegisterQueue called");
    return NULL;
}

void* MDSQueueRegistryRetireToken(void) {
    if (verbose) puts("STUB: MDSQueueRegistryRetireToken called");
    return NULL;
}

void* MDSRegisterSystemLoadAdvisory(void) {
    if (verbose) puts("STUB: MDSRegisterSystemLoadAdvisory called");
    return NULL;
}

void* MDSTokenRegistryAddTokenForKey(void) {
    if (verbose) puts("STUB: MDSTokenRegistryAddTokenForKey called");
    return NULL;
}

void* MDSTokenRegistryAddTokenForUUID(void) {
    if (verbose) puts("STUB: MDSTokenRegistryAddTokenForUUID called");
    return NULL;
}

void* MDSTokenRegistryCopyTokenWithID(void) {
    if (verbose) puts("STUB: MDSTokenRegistryCopyTokenWithID called");
    return NULL;
}

void* MDSTokenRegistryCopyTokensForKey(void) {
    if (verbose) puts("STUB: MDSTokenRegistryCopyTokensForKey called");
    return NULL;
}

void* MDSTokenRegistryCreateKeyForClass(void) {
    if (verbose) puts("STUB: MDSTokenRegistryCreateKeyForClass called");
    return NULL;
}

void* MDSTokenRegistryLookupKeyForClass(void) {
    if (verbose) puts("STUB: MDSTokenRegistryLookupKeyForClass called");
    return NULL;
}

void* MDSTokenRegistryRemoveTokenWithID(void) {
    if (verbose) puts("STUB: MDSTokenRegistryRemoveTokenWithID called");
    return NULL;
}

void* MDSTokenRegistryResolveTokenWithID(void) {
    if (verbose) puts("STUB: MDSTokenRegistryResolveTokenWithID called");
    return NULL;
}

void* MDSUnregisterSystemLoadAdvisory(void) {
    if (verbose) puts("STUB: MDSUnregisterSystemLoadAdvisory called");
    return NULL;
}

void* MDSecondsToMachAbsoluteTime(void) {
    if (verbose) puts("STUB: MDSecondsToMachAbsoluteTime called");
    return NULL;
}

void* MDSetCreateMutable(void) {
    if (verbose) puts("STUB: MDSetCreateMutable called");
    return NULL;
}

void* MDSetIterate(void) {
    if (verbose) puts("STUB: MDSetIterate called");
    return NULL;
}

void* MDSetProcessIOPolicy(void) {
    if (verbose) puts("STUB: MDSetProcessIOPolicy called");
    return NULL;
}

void* MDSetProcessVNodeRapidAging(void) {
    if (verbose) puts("STUB: MDSetProcessVNodeRapidAging called");
    return NULL;
}

void* MDSetThreadIOPolicy(void) {
    if (verbose) puts("STUB: MDSetThreadIOPolicy called");
    return NULL;
}

void* MDSetThreadPriority(void) {
    if (verbose) puts("STUB: MDSetThreadPriority called");
    return NULL;
}

void* MDSetThreadVNodeRapidAging(void) {
    if (verbose) puts("STUB: MDSetThreadVNodeRapidAging called");
    return NULL;
}

void* MDTimeSinceBoot(void) {
    if (verbose) puts("STUB: MDTimeSinceBoot called");
    return NULL;
}

void* XPCUtil_CopyCFFromArray(void) {
    if (verbose) puts("STUB: XPCUtil_CopyCFFromArray called");
    return NULL;
}

void* XPCUtil_CopyCFFromDictionary(void) {
    if (verbose) puts("STUB: XPCUtil_CopyCFFromDictionary called");
    return NULL;
}

void* XPCUtil_CopyCFFromObject(void) {
    if (verbose) puts("STUB: XPCUtil_CopyCFFromObject called");
    return NULL;
}

void* XPCUtil_CopyXPCObjectFromCF(void) {
    if (verbose) puts("STUB: XPCUtil_CopyXPCObjectFromCF called");
    return NULL;
}

void* XPCUtil_PackCFIntoArray(void) {
    if (verbose) puts("STUB: XPCUtil_PackCFIntoArray called");
    return NULL;
}

void* XPCUtil_PackCFIntoDictionary(void) {
    if (verbose) puts("STUB: XPCUtil_PackCFIntoDictionary called");
    return NULL;
}

void* _MDIsAppleInternal(void) {
    if (verbose) puts("STUB: _MDIsAppleInternal called");
    return NULL;
}

void* _MDSLogInit(void) {
    if (verbose) puts("STUB: _MDSLogInit called");
    return NULL;
}

void* _MDSTokenRegistryInitialize(void) {
    if (verbose) puts("STUB: _MDSTokenRegistryInitialize called");
    return NULL;
}

void* _mdsLogPrint(void) {
    if (verbose) puts("STUB: _mdsLogPrint called");
    return NULL;
}

void* _mdsLogPrintObjC(void) {
    if (verbose) puts("STUB: _mdsLogPrintObjC called");
    return NULL;
}

void* _mdsLogWrite(void) {
    if (verbose) puts("STUB: _mdsLogWrite called");
    return NULL;
}

void* _mdsLogWriteObjC(void) {
    if (verbose) puts("STUB: _mdsLogWriteObjC called");
    return NULL;
}

void* bsearch_aux(void) {
    if (verbose) puts("STUB: bsearch_aux called");
    return NULL;
}

void* cStringFromCFString(void) {
    if (verbose) puts("STUB: cStringFromCFString called");
    return NULL;
}

void* directPostAction(void) {
    if (verbose) puts("STUB: directPostAction called");
    return NULL;
}

void* findExtension(void) {
    if (verbose) puts("STUB: findExtension called");
    return NULL;
}

void* findNameAndExtension(void) {
    if (verbose) puts("STUB: findNameAndExtension called");
    return NULL;
}

void* isValidUUID(void) {
    if (verbose) puts("STUB: isValidUUID called");
    return NULL;
}

void* kCFContainerReleaseCallBack(void) {
    if (verbose) puts("STUB: kCFContainerReleaseCallBack called");
    return NULL;
}

void* kCFContainerRetainCallBack(void) {
    if (verbose) puts("STUB: kCFContainerRetainCallBack called");
    return NULL;
}

void* mdBufferSprintf(void) {
    if (verbose) puts("STUB: mdBufferSprintf called");
    return NULL;
}

void* mdsMachSubsystemGeneralBlock(void) {
    if (verbose) puts("STUB: mdsMachSubsystemGeneralBlock called");
    return NULL;
}

void* mdsObjectGeneralBlock(void) {
    if (verbose) puts("STUB: mdsObjectGeneralBlock called");
    return NULL;
}

void* mds_after(void) {
    if (verbose) puts("STUB: mds_after called");
    return NULL;
}

void* mds_async(void) {
    if (verbose) puts("STUB: mds_async called");
    return NULL;
}

void* mds_async2(void) {
    if (verbose) puts("STUB: mds_async2 called");
    return NULL;
}

void* mds_concurrent(void) {
    if (verbose) puts("STUB: mds_concurrent called");
    return NULL;
}

void* mds_create_oneshot_timer(void) {
    if (verbose) puts("STUB: mds_create_oneshot_timer called");
    return NULL;
}

void* mds_create_timer(void) {
    if (verbose) puts("STUB: mds_create_timer called");
    return NULL;
}

void* mds_dispatch_nocopy_cancelable_wrapper(void) {
    if (verbose) puts("STUB: mds_dispatch_nocopy_cancelable_wrapper called");
    return NULL;
}

void* mds_dispatch_nocopy_wrapper(void) {
    if (verbose) puts("STUB: mds_dispatch_nocopy_wrapper called");
    return NULL;
}

void* mds_execute_block(void) {
    if (verbose) puts("STUB: mds_execute_block called");
    return NULL;
}

void* mds_group_async(void) {
    if (verbose) puts("STUB: mds_group_async called");
    return NULL;
}

void* mds_group_async2(void) {
    if (verbose) puts("STUB: mds_group_async2 called");
    return NULL;
}

void* mds_propagating_async(void) {
    if (verbose) puts("STUB: mds_propagating_async called");
    return NULL;
}

void* mds_propagating_async2(void) {
    if (verbose) puts("STUB: mds_propagating_async2 called");
    return NULL;
}

void* mds_qos_async(void) {
    if (verbose) puts("STUB: mds_qos_async called");
    return NULL;
}

void* mds_qos_async2(void) {
    if (verbose) puts("STUB: mds_qos_async2 called");
    return NULL;
}

void* mds_qos_sync(void) {
    if (verbose) puts("STUB: mds_qos_sync called");
    return NULL;
}

void* mds_qos_sync2(void) {
    if (verbose) puts("STUB: mds_qos_sync2 called");
    return NULL;
}

void* mds_sync(void) {
    if (verbose) puts("STUB: mds_sync called");
    return NULL;
}

void* mds_sync2(void) {
    if (verbose) puts("STUB: mds_sync2 called");
    return NULL;
}

void* path_bundle_index(void) {
    if (verbose) puts("STUB: path_bundle_index called");
    return NULL;
}

void* str_common(void) {
    if (verbose) puts("STUB: str_common called");
    return NULL;
}

void* str_prefix(void) {
    if (verbose) puts("STUB: str_prefix called");
    return NULL;
}

void* strcmp_nocase(void) {
    if (verbose) puts("STUB: strcmp_nocase called");
    return NULL;
}

void* strcmp_prefix(void) {
    if (verbose) puts("STUB: strcmp_prefix called");
    return NULL;
}

void* token_after(void) {
    if (verbose) puts("STUB: token_after called");
    return NULL;
}

void* token_after2(void) {
    if (verbose) puts("STUB: token_after2 called");
    return NULL;
}

void* token_async(void) {
    if (verbose) puts("STUB: token_async called");
    return NULL;
}

void* token_async2(void) {
    if (verbose) puts("STUB: token_async2 called");
    return NULL;
}

void* token_group_async(void) {
    if (verbose) puts("STUB: token_group_async called");
    return NULL;
}

void* token_group_async2(void) {
    if (verbose) puts("STUB: token_group_async2 called");
    return NULL;
}

void* token_propagating_async(void) {
    if (verbose) puts("STUB: token_propagating_async called");
    return NULL;
}

void* token_propagating_async2(void) {
    if (verbose) puts("STUB: token_propagating_async2 called");
    return NULL;
}

void* token_qos_async(void) {
    if (verbose) puts("STUB: token_qos_async called");
    return NULL;
}

void* token_qos_async2(void) {
    if (verbose) puts("STUB: token_qos_async2 called");
    return NULL;
}

void* token_qos_sync(void) {
    if (verbose) puts("STUB: token_qos_sync called");
    return NULL;
}

void* token_qos_sync2(void) {
    if (verbose) puts("STUB: token_qos_sync2 called");
    return NULL;
}

void* token_sync(void) {
    if (verbose) puts("STUB: token_sync called");
    return NULL;
}

void* token_sync2(void) {
    if (verbose) puts("STUB: token_sync2 called");
    return NULL;
}
