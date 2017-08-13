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
void* MachToNano(void) { if (verbose) puts("STUB: MachToNano called"); return NULL; }
void* PACFBundleCopyStringForKey(void) { if (verbose) puts("STUB: PACFBundleCopyStringForKey called"); return NULL; }
void* PACancelOperation(void) { if (verbose) puts("STUB: PACancelOperation called"); return NULL; }
void* PACommonAverageFromArray(void) { if (verbose) puts("STUB: PACommonAverageFromArray called"); return NULL; }
void* PACommonStandardDeviationFromArray(void) { if (verbose) puts("STUB: PACommonStandardDeviationFromArray called"); return NULL; }
void* PACurrentRealTimeInSeconds(void) { if (verbose) puts("STUB: PACurrentRealTimeInSeconds called"); return NULL; }
void* PADetailedPerfLoggingIsEnabled(void) { if (verbose) puts("STUB: PADetailedPerfLoggingIsEnabled called"); return NULL; }
void* PADetailedPerfLoggingWhiteListAddEntry(void) { if (verbose) puts("STUB: PADetailedPerfLoggingWhiteListAddEntry called"); return NULL; }
void* PADetailedPerfLoggingWhiteListClearList(void) { if (verbose) puts("STUB: PADetailedPerfLoggingWhiteListClearList called"); return NULL; }
void* PADetailedPerfLoggingWhiteListRemoveEntry(void) { if (verbose) puts("STUB: PADetailedPerfLoggingWhiteListRemoveEntry called"); return NULL; }
void* PAMachTimeForMachAbsTime(void) { if (verbose) puts("STUB: PAMachTimeForMachAbsTime called"); return NULL; }
void* PAMachTimeForMachAbsTimeWithTimebase(void) { if (verbose) puts("STUB: PAMachTimeForMachAbsTimeWithTimebase called"); return NULL; }
void* PAMachTimeForNSNumberOfMachAbsTime(void) { if (verbose) puts("STUB: PAMachTimeForNSNumberOfMachAbsTime called"); return NULL; }
void* PAMachTimeGetCurrent(void) { if (verbose) puts("STUB: PAMachTimeGetCurrent called"); return NULL; }
void* PAMachTimeToMachAbsTimeWithTimebase(void) { if (verbose) puts("STUB: PAMachTimeToMachAbsTimeWithTimebase called"); return NULL; }
void* PAMarkOperationEnd(void) { if (verbose) puts("STUB: PAMarkOperationEnd called"); return NULL; }
void* PAMarkOperationEndAndGetIntervalStats(void) { if (verbose) puts("STUB: PAMarkOperationEndAndGetIntervalStats called"); return NULL; }
void* PAMarkOperationEndAndTruncateByDurationInSec(void) { if (verbose) puts("STUB: PAMarkOperationEndAndTruncateByDurationInSec called"); return NULL; }
void* PAMarkOperationEndAndTruncateByDurationInSecAndGetIntervalStats(void) { if (verbose) puts("STUB: PAMarkOperationEndAndTruncateByDurationInSecAndGetIntervalStats called"); return NULL; }
void* PAMarkOperationEvent(void) { if (verbose) puts("STUB: PAMarkOperationEvent called"); return NULL; }
void* PAMarkOperationIntervalEnd(void) { if (verbose) puts("STUB: PAMarkOperationIntervalEnd called"); return NULL; }
void* PAMarkOperationIntervalEndAndStart(void) { if (verbose) puts("STUB: PAMarkOperationIntervalEndAndStart called"); return NULL; }
void* PAMarkOperationIntervalStart(void) { if (verbose) puts("STUB: PAMarkOperationIntervalStart called"); return NULL; }
void* PAMarkOperationStart(void) { if (verbose) puts("STUB: PAMarkOperationStart called"); return NULL; }
void* PAMarkOperationStartAndSetContextProcessingConfiguration(void) { if (verbose) puts("STUB: PAMarkOperationStartAndSetContextProcessingConfiguration called"); return NULL; }
void* PANSDictionaryCopyStringForKey(void) { if (verbose) puts("STUB: PANSDictionaryCopyStringForKey called"); return NULL; }
void* PAPerfLoggingGetLogHandle(void) { if (verbose) puts("STUB: PAPerfLoggingGetLogHandle called"); return NULL; }
void* PAPerfLoggingIntervalPrintHeader(void) { if (verbose) puts("STUB: PAPerfLoggingIntervalPrintHeader called"); return NULL; }
void* PAPerfLoggingIsEnabled(void) { if (verbose) puts("STUB: PAPerfLoggingIsEnabled called"); return NULL; }
void* PAPerfLoggingMachAbsoluteTimeToSeconds(void) { if (verbose) puts("STUB: PAPerfLoggingMachAbsoluteTimeToSeconds called"); return NULL; }
void* PAPerfLoggingOnlyReportFramerate(void) { if (verbose) puts("STUB: PAPerfLoggingOnlyReportFramerate called"); return NULL; }
void* PAPerfLoggingProcessWSUpdateInfoDictionary(void) { if (verbose) puts("STUB: PAPerfLoggingProcessWSUpdateInfoDictionary called"); return NULL; }
void* PAPerfLoggingSecondsToMachAbsoluteTime(void) { if (verbose) puts("STUB: PAPerfLoggingSecondsToMachAbsoluteTime called"); return NULL; }
void* PAPerfLoggingSetDataGatheringEnabled(void) { if (verbose) puts("STUB: PAPerfLoggingSetDataGatheringEnabled called"); return NULL; }
void* PAPerfLoggingTimeoutIsEnabled(void) { if (verbose) puts("STUB: PAPerfLoggingTimeoutIsEnabled called"); return NULL; }
void* PASetContextProcessingConfigurationForOperation(void) { if (verbose) puts("STUB: PASetContextProcessingConfigurationForOperation called"); return NULL; }
void* PASetContextProcessingConfigurationForOperationIntervalType(void) { if (verbose) puts("STUB: PASetContextProcessingConfigurationForOperationIntervalType called"); return NULL; }
void* PAXPCCancelOperation(void) { if (verbose) puts("STUB: PAXPCCancelOperation called"); return NULL; }
void* PAXPCGetAPDConnection(void) { if (verbose) puts("STUB: PAXPCGetAPDConnection called"); return NULL; }
void* PAXPCMarkOperationEnd(void) { if (verbose) puts("STUB: PAXPCMarkOperationEnd called"); return NULL; }
void* PAXPCMarkOperationEndAndTruncateByDurationInSec(void) { if (verbose) puts("STUB: PAXPCMarkOperationEndAndTruncateByDurationInSec called"); return NULL; }
void* PAXPCMarkOperationEndAndTruncateByDurationInSecNoSysLogLogging(void) { if (verbose) puts("STUB: PAXPCMarkOperationEndAndTruncateByDurationInSecNoSysLogLogging called"); return NULL; }
void* PAXPCMarkOperationEndNoSysLogLogging(void) { if (verbose) puts("STUB: PAXPCMarkOperationEndNoSysLogLogging called"); return NULL; }
void* PAXPCMarkOperationEvent(void) { if (verbose) puts("STUB: PAXPCMarkOperationEvent called"); return NULL; }
void* PAXPCMarkOperationIntervalEnd(void) { if (verbose) puts("STUB: PAXPCMarkOperationIntervalEnd called"); return NULL; }
void* PAXPCMarkOperationIntervalEndAndStart(void) { if (verbose) puts("STUB: PAXPCMarkOperationIntervalEndAndStart called"); return NULL; }
void* PAXPCMarkOperationIntervalStart(void) { if (verbose) puts("STUB: PAXPCMarkOperationIntervalStart called"); return NULL; }
void* PAXPCMarkOperationStart(void) { if (verbose) puts("STUB: PAXPCMarkOperationStart called"); return NULL; }
void* PAXPCMarkOperationStartAndForceDetailedLogging(void) { if (verbose) puts("STUB: PAXPCMarkOperationStartAndForceDetailedLogging called"); return NULL; }
void* PA_MachAbsTimeToNanosecondsWithTimebase(void) { if (verbose) puts("STUB: PA_MachAbsTimeToNanosecondsWithTimebase called"); return NULL; }
void* PA_NanosecondsToMachAbsTimeWithTimebase(void) { if (verbose) puts("STUB: PA_NanosecondsToMachAbsTimeWithTimebase called"); return NULL; }
void* _PALogTimestamp(void) { if (verbose) puts("STUB: _PALogTimestamp called"); return NULL; }
void* _PAPrintToMutableData(void) { if (verbose) puts("STUB: _PAPrintToMutableData called"); return NULL; }
void* formattedAddress(void) { if (verbose) puts("STUB: formattedAddress called"); return NULL; }
void* formattedBytes(void) { if (verbose) puts("STUB: formattedBytes called"); return NULL; }
void* formattedBytesDouble(void) { if (verbose) puts("STUB: formattedBytesDouble called"); return NULL; }
void* formattedSignedBytes(void) { if (verbose) puts("STUB: formattedSignedBytes called"); return NULL; }
void* levelFormattedString(void) { if (verbose) puts("STUB: levelFormattedString called"); return NULL; }
void* nameForProcInfo(void) { if (verbose) puts("STUB: nameForProcInfo called"); return NULL; }
void* newProcessStructures(void) { if (verbose) puts("STUB: newProcessStructures called"); return NULL; }
void* pagesToBytes(void) { if (verbose) puts("STUB: pagesToBytes called"); return NULL; }
void* pagesToBytesDefault(void) { if (verbose) puts("STUB: pagesToBytesDefault called"); return NULL; }
void* pointerForIndex(void) { if (verbose) puts("STUB: pointerForIndex called"); return NULL; }
void* sizeOfRAM(void) { if (verbose) puts("STUB: sizeOfRAM called"); return NULL; }
void* stringForIndex(void) { if (verbose) puts("STUB: stringForIndex called"); return NULL; }
void* stringIndexForNSString(void) { if (verbose) puts("STUB: stringIndexForNSString called"); return NULL; }
void* stringNameForPAInternalTransitionType(void) { if (verbose) puts("STUB: stringNameForPAInternalTransitionType called"); return NULL; }
void* uuidForBytes(void) { if (verbose) puts("STUB: uuidForBytes called"); return NULL; }
void* uuidForString(void) { if (verbose) puts("STUB: uuidForString called"); return NULL; }
