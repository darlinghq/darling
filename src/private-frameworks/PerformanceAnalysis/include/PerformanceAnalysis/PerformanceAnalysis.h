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

#import <Foundation/Foundation.h>

void* MachToNano(void);
void* PACFBundleCopyStringForKey(void);
void* PACancelOperation(void);
void* PACommonAverageFromArray(void);
void* PACommonStandardDeviationFromArray(void);
void* PACurrentRealTimeInSeconds(void);
void* PADetailedPerfLoggingIsEnabled(void);
void* PADetailedPerfLoggingWhiteListAddEntry(void);
void* PADetailedPerfLoggingWhiteListClearList(void);
void* PADetailedPerfLoggingWhiteListRemoveEntry(void);
void* PAMachTimeForMachAbsTime(void);
void* PAMachTimeForMachAbsTimeWithTimebase(void);
void* PAMachTimeForNSNumberOfMachAbsTime(void);
void* PAMachTimeGetCurrent(void);
void* PAMachTimeToMachAbsTimeWithTimebase(void);
void* PAMarkOperationEnd(void);
void* PAMarkOperationEndAndGetIntervalStats(void);
void* PAMarkOperationEndAndTruncateByDurationInSec(void);
void* PAMarkOperationEndAndTruncateByDurationInSecAndGetIntervalStats(void);
void* PAMarkOperationEvent(void);
void* PAMarkOperationIntervalEnd(void);
void* PAMarkOperationIntervalEndAndStart(void);
void* PAMarkOperationIntervalStart(void);
void* PAMarkOperationStart(void);
void* PAMarkOperationStartAndSetContextProcessingConfiguration(void);
void* PANSDictionaryCopyStringForKey(void);
void* PAPerfLoggingGetLogHandle(void);
void* PAPerfLoggingIntervalPrintHeader(void);
void* PAPerfLoggingIsEnabled(void);
void* PAPerfLoggingMachAbsoluteTimeToSeconds(void);
void* PAPerfLoggingOnlyReportFramerate(void);
void* PAPerfLoggingProcessWSUpdateInfoDictionary(void);
void* PAPerfLoggingSecondsToMachAbsoluteTime(void);
void* PAPerfLoggingSetDataGatheringEnabled(void);
void* PAPerfLoggingTimeoutIsEnabled(void);
void* PASetContextProcessingConfigurationForOperation(void);
void* PASetContextProcessingConfigurationForOperationIntervalType(void);
void* PAXPCCancelOperation(void);
void* PAXPCGetAPDConnection(void);
void* PAXPCMarkOperationEnd(void);
void* PAXPCMarkOperationEndAndTruncateByDurationInSec(void);
void* PAXPCMarkOperationEndAndTruncateByDurationInSecNoSysLogLogging(void);
void* PAXPCMarkOperationEndNoSysLogLogging(void);
void* PAXPCMarkOperationEvent(void);
void* PAXPCMarkOperationIntervalEnd(void);
void* PAXPCMarkOperationIntervalEndAndStart(void);
void* PAXPCMarkOperationIntervalStart(void);
void* PAXPCMarkOperationStart(void);
void* PAXPCMarkOperationStartAndForceDetailedLogging(void);
void* PA_MachAbsTimeToNanosecondsWithTimebase(void);
void* PA_NanosecondsToMachAbsTimeWithTimebase(void);
void* _PALogTimestamp(void);
void* _PAPrintToMutableData(void);
void* formattedAddress(void);
void* formattedBytes(void);
void* formattedBytesDouble(void);
void* formattedSignedBytes(void);
void* levelFormattedString(void);
void* nameForProcInfo(void);
void* newProcessStructures(void);
void* pagesToBytes(void);
void* pagesToBytesDefault(void);
void* pointerForIndex(void);
void* sizeOfRAM(void);
void* stringForIndex(void);
void* stringIndexForNSString(void);
void* stringNameForPAInternalTransitionType(void);
void* uuidForBytes(void);
void* uuidForString(void);

#import <PerformanceAnalysis/MallocTotalDiff.h>
#import <PerformanceAnalysis/MallocDataCategorySummaryDiff.h>
#import <PerformanceAnalysis/PurgeTypeTotalDiff.h>
#import <PerformanceAnalysis/ZoneTotalDiff.h>
#import <PerformanceAnalysis/AllocationTotalDiff.h>
#import <PerformanceAnalysis/MallocTotal.h>
#import <PerformanceAnalysis/MallocDataCategorySummary.h>
#import <PerformanceAnalysis/PurgeTypeTotal.h>
#import <PerformanceAnalysis/ZoneTotal.h>
#import <PerformanceAnalysis/AllocationTotal.h>
#import <PerformanceAnalysis/PAPLDistributedNotificationHandler.h>
#import <PerformanceAnalysis/PAPerfLoggingOperationCompletionNotificationIntervalData.h>
#import <PerformanceAnalysis/PAPerfLoggingOperationCompletionNotification.h>
#import <PerformanceAnalysis/PAProcess.h>
#import <PerformanceAnalysis/PAMemRegion.h>
#import <PerformanceAnalysis/PASystem.h>
#import <PerformanceAnalysis/PAMemRange.h>
#import <PerformanceAnalysis/PAMappedFileRegion.h>
#import <PerformanceAnalysis/PAImage.h>
#import <PerformanceAnalysis/PAImageSegment.h>
#import <PerformanceAnalysis/PAImageSection.h>
#import <PerformanceAnalysis/PAPageResidency.h>
#import <PerformanceAnalysis/PAPageResidencyDiff.h>
#import <PerformanceAnalysis/PAMallocData.h>
#import <PerformanceAnalysis/PAMemAllocationCategory.h>
#import <PerformanceAnalysis/PAPerfLoggingClientOperation.h>
#import <PerformanceAnalysis/PAMemAllocation.h>
#import <PerformanceAnalysis/PASampleNode.h>
#import <PerformanceAnalysis/PARangeContainer.h>
#import <PerformanceAnalysis/PADataArchiveKey.h>
#import <PerformanceAnalysis/PADataArchiveHandle.h>
#import <PerformanceAnalysis/PADataArchive.h>
#import <PerformanceAnalysis/PAAbandonedMemory.h>
#import <PerformanceAnalysis/PAImageSegmentRegion.h>
#import <PerformanceAnalysis/PAVideoCard.h>
#import <PerformanceAnalysis/PAVideoCardSet.h>
#import <PerformanceAnalysis/PAMallocStackTree.h>
#import <PerformanceAnalysis/PAMallocStackTreeFrame.h>
#import <PerformanceAnalysis/PAPerfLoggingOperation.h>
#import <PerformanceAnalysis/PAPerfLoggingIntervalTransitionEvent.h>
#import <PerformanceAnalysis/PAPerfLoggingIntervalData.h>
#import <PerformanceAnalysis/PAMemoryObjectOrganizer.h>
#import <PerformanceAnalysis/PAMemoryObject.h>
#import <PerformanceAnalysis/PAProcessMemoryGroup.h>
#import <PerformanceAnalysis/PAMemoryCategoryOrganizer.h>
#import <PerformanceAnalysis/PACommpageRegion.h>
#import <PerformanceAnalysis/PAPage.h>
#import <PerformanceAnalysis/PAMemorySimpleGraphicsSummary.h>
#import <PerformanceAnalysis/PAGraphicsInfoCollector.h>
#import <PerformanceAnalysis/PAMallocRegion.h>
#import <PerformanceAnalysis/PAMemoryCategory.h>
#import <PerformanceAnalysis/PAMemorySubCategory.h>
#import <PerformanceAnalysis/PAMemoryCategoryAggregation.h>
#import <PerformanceAnalysis/PAFootprint.h>
