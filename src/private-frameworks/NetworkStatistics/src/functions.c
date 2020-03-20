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
void* NStatManagerAddAllRoutes(void) { if (verbose) puts("STUB: NStatManagerAddAllRoutes called"); return NULL; }
void* NStatManagerAddAllRoutesWithFilter(void) { if (verbose) puts("STUB: NStatManagerAddAllRoutesWithFilter called"); return NULL; }
void* NStatManagerAddAllTCP(void) { if (verbose) puts("STUB: NStatManagerAddAllTCP called"); return NULL; }
void* NStatManagerAddAllTCPWithFilter(void) { if (verbose) puts("STUB: NStatManagerAddAllTCPWithFilter called"); return NULL; }
void* NStatManagerAddAllTCPWithOptions(void) { if (verbose) puts("STUB: NStatManagerAddAllTCPWithOptions called"); return NULL; }
void* NStatManagerAddAllUDP(void) { if (verbose) puts("STUB: NStatManagerAddAllUDP called"); return NULL; }
void* NStatManagerAddAllUDPWithFilter(void) { if (verbose) puts("STUB: NStatManagerAddAllUDPWithFilter called"); return NULL; }
void* NStatManagerAddAllUDPWithOptions(void) { if (verbose) puts("STUB: NStatManagerAddAllUDPWithOptions called"); return NULL; }
void* NStatManagerCreate(void) { if (verbose) puts("STUB: NStatManagerCreate called"); return NULL; }
void* NStatManagerCreateInterfaceSource(void) { if (verbose) puts("STUB: NStatManagerCreateInterfaceSource called"); return NULL; }
void* NStatManagerCreateRouteSource(void) { if (verbose) puts("STUB: NStatManagerCreateRouteSource called"); return NULL; }
void* NStatManagerCreateSysinfoSource(void) { if (verbose) puts("STUB: NStatManagerCreateSysinfoSource called"); return NULL; }
void* NStatManagerCreateTCPSource(void) { if (verbose) puts("STUB: NStatManagerCreateTCPSource called"); return NULL; }
void* NStatManagerCreateUDPSource(void) { if (verbose) puts("STUB: NStatManagerCreateUDPSource called"); return NULL; }
void* NStatManagerDestroy(void) { if (verbose) puts("STUB: NStatManagerDestroy called"); return NULL; }
void* NStatManagerQueryAllSources(void) { if (verbose) puts("STUB: NStatManagerQueryAllSources called"); return NULL; }
void* NStatManagerQueryAllSourcesDescriptions(void) { if (verbose) puts("STUB: NStatManagerQueryAllSourcesDescriptions called"); return NULL; }
void* NStatManagerQueryAllSourcesUpdate(void) { if (verbose) puts("STUB: NStatManagerQueryAllSourcesUpdate called"); return NULL; }
void* NStatManagerSetFlags(void) { if (verbose) puts("STUB: NStatManagerSetFlags called"); return NULL; }
void* NStatManagerSetInterfaceSourceFD(void) { if (verbose) puts("STUB: NStatManagerSetInterfaceSourceFD called"); return NULL; }
void* NStatManagerSetInterfaceTraceFD(void) { if (verbose) puts("STUB: NStatManagerSetInterfaceTraceFD called"); return NULL; }
void* NStatManagerSetQueuePriority(void) { if (verbose) puts("STUB: NStatManagerSetQueuePriority called"); return NULL; }
void* NStatRouteValuesForIPv4Host(void) { if (verbose) puts("STUB: NStatRouteValuesForIPv4Host called"); return NULL; }
void* NStatRouteValuesForIPv6Host(void) { if (verbose) puts("STUB: NStatRouteValuesForIPv6Host called"); return NULL; }
void* NStatSourceCopyCounts(void) { if (verbose) puts("STUB: NStatSourceCopyCounts called"); return NULL; }
void* NStatSourceCopyProperties(void) { if (verbose) puts("STUB: NStatSourceCopyProperties called"); return NULL; }
void* NStatSourceCopyProperty(void) { if (verbose) puts("STUB: NStatSourceCopyProperty called"); return NULL; }
void* NStatSourceQueryCounts(void) { if (verbose) puts("STUB: NStatSourceQueryCounts called"); return NULL; }
void* NStatSourceQueryDescription(void) { if (verbose) puts("STUB: NStatSourceQueryDescription called"); return NULL; }
void* NStatSourceQueryUpdate(void) { if (verbose) puts("STUB: NStatSourceQueryUpdate called"); return NULL; }
void* NStatSourceRemove(void) { if (verbose) puts("STUB: NStatSourceRemove called"); return NULL; }
void* NStatSourceSetCountsBlock(void) { if (verbose) puts("STUB: NStatSourceSetCountsBlock called"); return NULL; }
void* NStatSourceSetDescriptionBlock(void) { if (verbose) puts("STUB: NStatSourceSetDescriptionBlock called"); return NULL; }
void* NStatSourceSetEventsBlock(void) { if (verbose) puts("STUB: NStatSourceSetEventsBlock called"); return NULL; }
void* NStatSourceSetFilter(void) { if (verbose) puts("STUB: NStatSourceSetFilter called"); return NULL; }
void* NStatSourceSetRemovedBlock(void) { if (verbose) puts("STUB: NStatSourceSetRemovedBlock called"); return NULL; }
