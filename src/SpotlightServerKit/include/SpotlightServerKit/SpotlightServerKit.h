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


#ifndef _SpotlightServerKit_H_
#define _SpotlightServerKit_H_

#import <Foundation/Foundation.h>

#import <SpotlightServerKit/MDSNotificationTokenRecord.h>
#import <SpotlightServerKit/MDSObject.h>
#import <SpotlightServerKit/MDSMachPortNotificationRecord.h>
#import <SpotlightServerKit/MDSMachPortObject.h>
#import <SpotlightServerKit/MDSMachSubsystem.h>
#import <SpotlightServerKit/MDSTwoFaceMachPortObject.h>
#import <SpotlightServerKit/MDSInternalToken.h>
#import <SpotlightServerKit/MDSToken.h>
#import <SpotlightServerKit/MDSObjectToken.h>
#import <SpotlightServerKit/MDSNotificationCenter.h>
#import <SpotlightServerKit/MDSReadCopyUpdate.h>

void* BWA_Create(void);
void* BWA_DataAllocate(void);
void* BWA_DataDeallocate(void);
void* CFDataCreateWithContentsOfURL(void);
void* CF_MaxRange(void);
void* CF_RangeIntersection(void);
void* CF_RangeUnion(void);
void* GetSystemBuildVersionString(void);
void* MDAddNanoseconds(void);
void* MDArrayCreateMutable(void);
void* MDArrayIterate(void);
void* MDCopyMachPortDescription(void);
void* MDDictionaryCreateMutable(void);
void* MDDictionaryIterate(void);
void* MDElapsedTime(void);
void* MDGetCurrentTimeSpec(void);
void* MDGetPhysicalRAMSize(void);
void* MDGetProcessIOPolicy(void);
void* MDGetThreadIOPolicy(void);
void* MDHash(void);
void* MDHashQuick32(void);
void* MDHashQuick32Pair(void);
void* MDHashQuick64(void);
void* MDHashQuickPtrAsInt(void);
void* MDKillTask(void);
void* MDMachAbsoluteTimeDiff(void);
void* MDNumberCreateDouble(void);
void* MDNumberCreateInt32(void);
void* MDNumberCreateInt64(void);
void* MDNumberGetDouble(void);
void* MDNumberGetInt32(void);
void* MDNumberGetInt64(void);
void* MDPowerInfoCollect(void);
void* MDPowerInfoDifference(void);
void* MDPowerInfoEnd(void);
void* MDPowerInfoSum(void);
void* MDRunningInRosetta(void);
void* MDRunningIntel32(void);
void* MDRunningIntel64(void);
void* MDRunningOnEffectivePPC(void);
void* MDRunningOnPhysicalIntel(void);
void* MDRunningOnPhysicalPPC(void);
void* MDSGetSystemLoadAdvisory(void);
void* MDSLogLevelCreateLogArrayDictionary(void);
void* MDSLogLoadLogLevelArrayFromDefaults(void);
void* MDSLogLoadLogLevelsFromDefaults(void);
void* MDSLogParseLogArrayDictionary(void);
void* MDSLogSaveLogLevelArrayToDefaults(void);
void* MDSLogSaveLogLevelsToDefaults(void);
void* MDSLogSetLoggingOutputFunction(void);
void* MDSLogSubsumeNSLog(void);
void* MDSMessageTracerEvent(void);
void* MDSObjectRegistryResolveObjectWithPort(void);
void* MDSQueueRegistryCopyQueueForToken(void);
void* MDSQueueRegistryRegisterQueue(void);
void* MDSQueueRegistryRetireToken(void);
void* MDSRegisterSystemLoadAdvisory(void);
void* MDSTokenRegistryAddTokenForKey(void);
void* MDSTokenRegistryAddTokenForUUID(void);
void* MDSTokenRegistryCopyTokenWithID(void);
void* MDSTokenRegistryCopyTokensForKey(void);
void* MDSTokenRegistryCreateKeyForClass(void);
void* MDSTokenRegistryLookupKeyForClass(void);
void* MDSTokenRegistryRemoveTokenWithID(void);
void* MDSTokenRegistryResolveTokenWithID(void);
void* MDSUnregisterSystemLoadAdvisory(void);
void* MDSecondsToMachAbsoluteTime(void);
void* MDSetCreateMutable(void);
void* MDSetIterate(void);
void* MDSetProcessIOPolicy(void);
void* MDSetProcessVNodeRapidAging(void);
void* MDSetThreadIOPolicy(void);
void* MDSetThreadPriority(void);
void* MDSetThreadVNodeRapidAging(void);
void* MDTimeSinceBoot(void);
void* XPCUtil_CopyCFFromArray(void);
void* XPCUtil_CopyCFFromDictionary(void);
void* XPCUtil_CopyCFFromObject(void);
void* XPCUtil_CopyXPCObjectFromCF(void);
void* XPCUtil_PackCFIntoArray(void);
void* XPCUtil_PackCFIntoDictionary(void);
void* _MDIsAppleInternal(void);
void* _MDSLogInit(void);
void* _MDSTokenRegistryInitialize(void);
void* _mdsLogPrint(void);
void* _mdsLogPrintObjC(void);
void* _mdsLogWrite(void);
void* _mdsLogWriteObjC(void);
void* bsearch_aux(void);
void* cStringFromCFString(void);
void* directPostAction(void);
void* findExtension(void);
void* findNameAndExtension(void);
void* isValidUUID(void);
void* kCFContainerReleaseCallBack(void);
void* kCFContainerRetainCallBack(void);
void* mdBufferSprintf(void);
void* mdsMachSubsystemGeneralBlock(void);
void* mdsObjectGeneralBlock(void);
void* mds_after(void);
void* mds_async(void);
void* mds_async2(void);
void* mds_concurrent(void);
void* mds_create_oneshot_timer(void);
void* mds_create_timer(void);
void* mds_dispatch_nocopy_cancelable_wrapper(void);
void* mds_dispatch_nocopy_wrapper(void);
void* mds_execute_block(void);
void* mds_group_async(void);
void* mds_group_async2(void);
void* mds_propagating_async(void);
void* mds_propagating_async2(void);
void* mds_qos_async(void);
void* mds_qos_async2(void);
void* mds_qos_sync(void);
void* mds_qos_sync2(void);
void* mds_sync(void);
void* mds_sync2(void);
void* path_bundle_index(void);
void* str_common(void);
void* str_prefix(void);
void* strcmp_nocase(void);
void* strcmp_prefix(void);
void* token_after(void);
void* token_after2(void);
void* token_async(void);
void* token_async2(void);
void* token_group_async(void);
void* token_group_async2(void);
void* token_propagating_async(void);
void* token_propagating_async2(void);
void* token_qos_async(void);
void* token_qos_async2(void);
void* token_qos_sync(void);
void* token_qos_sync2(void);
void* token_sync(void);
void* token_sync2(void);

#endif
