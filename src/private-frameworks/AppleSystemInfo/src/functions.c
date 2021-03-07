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
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* ASI_BusSpeedInHz(void) { if (verbose) puts("STUB: ASI_BusSpeedInHz called"); return NULL; }
void* ASI_CPUSpeedInHz(void) { if (verbose) puts("STUB: ASI_CPUSpeedInHz called"); return NULL; }
void* ASI_CopyBusSpeedDescription(void) { if (verbose) puts("STUB: ASI_CopyBusSpeedDescription called"); return NULL; }
void* ASI_CopyCPUDescription(void) { if (verbose) puts("STUB: ASI_CopyCPUDescription called"); return NULL; }
void* ASI_CopyCPUKind(void) { if (verbose) puts("STUB: ASI_CopyCPUKind called"); return NULL; }
void* ASI_CopyCurrentBusSpeedDescription(void) { if (verbose) puts("STUB: ASI_CopyCurrentBusSpeedDescription called"); return NULL; }
void* ASI_CopyCurrentCPUDescription(void) { if (verbose) puts("STUB: ASI_CopyCurrentCPUDescription called"); return NULL; }
void* ASI_CopyCurrentRAMDescription(void) { if (verbose) puts("STUB: ASI_CopyCurrentRAMDescription called"); return NULL; }
void* ASI_CopyCurrentRAMDescriptionWithType(void) { if (verbose) puts("STUB: ASI_CopyCurrentRAMDescriptionWithType called"); return NULL; }
void* ASI_CopyDiskSizeDescription(void) { if (verbose) puts("STUB: ASI_CopyDiskSizeDescription called"); return NULL; }
void* ASI_CopyFormattedSerialNumber(void) { if (verbose) puts("STUB: ASI_CopyFormattedSerialNumber called"); return NULL; }
void* ASI_CopyHostUUID(void) { if (verbose) puts("STUB: ASI_CopyHostUUID called"); return NULL; }
void* ASI_CopyMachineConfigurationCode(void) { if (verbose) puts("STUB: ASI_CopyMachineConfigurationCode called"); return NULL; }
void* ASI_CopyRAMDescription(void) { if (verbose) puts("STUB: ASI_CopyRAMDescription called"); return NULL; }
void* ASI_IsPlatformFeatureEnabled(void) { if (verbose) puts("STUB: ASI_IsPlatformFeatureEnabled called"); return NULL; }
void* ASI_PhysicalMemorySize(void) { if (verbose) puts("STUB: ASI_PhysicalMemorySize called"); return NULL; }
void* CopyIOPlatformSerialNumberString(void) { if (verbose) puts("STUB: CopyIOPlatformSerialNumberString called"); return NULL; }
void* CopyPropertyAsData(void) { if (verbose) puts("STUB: CopyPropertyAsData called"); return NULL; }
void* GetIOPlatformFeature(void) { if (verbose) puts("STUB: GetIOPlatformFeature called"); return NULL; }
void* PlatformOverideExistsAndIsSet(void) { if (verbose) puts("STUB: PlatformOverideExistsAndIsSet called"); return NULL; }
