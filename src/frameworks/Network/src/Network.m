/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#include <Network/Network.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* NWCreateDispatchDataFromNSData(void)
{
    if (verbose) puts("STUB: NWCreateDispatchDataFromNSData called");
    return NULL;
}

void* NWCreateNSDataFromDispatchData(void)
{
    if (verbose) puts("STUB: NWCreateNSDataFromDispatchData called");
    return NULL;
}

void* NWMonitorStatusToString(void)
{
    if (verbose) puts("STUB: NWMonitorStatusToString called");
    return NULL;
}

void* NWPBAddressEndpointReadFrom(void)
{
    if (verbose) puts("STUB: NWPBAddressEndpointReadFrom called");
    return NULL;
}

void* NWPBAgentClassReadFrom(void)
{
    if (verbose) puts("STUB: NWPBAgentClassReadFrom called");
    return NULL;
}

void* NWPBAgentReadFrom(void)
{
    if (verbose) puts("STUB: NWPBAgentReadFrom called");
    return NULL;
}

void* NWPBBrowseDescriptorReadFrom(void)
{
    if (verbose) puts("STUB: NWPBBrowseDescriptorReadFrom called");
    return NULL;
}

void* NWPBCloseConnectionReadFrom(void)
{
    if (verbose) puts("STUB: NWPBCloseConnectionReadFrom called");
    return NULL;
}

void* NWPBCommandMessageReadFrom(void)
{
    if (verbose) puts("STUB: NWPBCommandMessageReadFrom called");
    return NULL;
}

void* NWPBEndpointReadFrom(void)
{
    if (verbose) puts("STUB: NWPBEndpointReadFrom called");
    return NULL;
}

void* NWPBHostEndpointReadFrom(void)
{
    if (verbose) puts("STUB: NWPBHostEndpointReadFrom called");
    return NULL;
}

void* NWPBInterfaceReadFrom(void)
{
    if (verbose) puts("STUB: NWPBInterfaceReadFrom called");
    return NULL;
}

void* NWPBOpenConnectionReadFrom(void)
{
    if (verbose) puts("STUB: NWPBOpenConnectionReadFrom called");
    return NULL;
}

void* NWPBParametersReadFrom(void)
{
    if (verbose) puts("STUB: NWPBParametersReadFrom called");
    return NULL;
}

void* NWPBPathReadFrom(void)
{
    if (verbose) puts("STUB: NWPBPathReadFrom called");
    return NULL;
}

void* NWPBSendDataReadFrom(void)
{
    if (verbose) puts("STUB: NWPBSendDataReadFrom called");
    return NULL;
}

void* NWPBServiceBrowseReadFrom(void)
{
    if (verbose) puts("STUB: NWPBServiceBrowseReadFrom called");
    return NULL;
}

void* NWPBServiceEndpointReadFrom(void)
{
    if (verbose) puts("STUB: NWPBServiceEndpointReadFrom called");
    return NULL;
}

void* NWPBStartBrowseReadFrom(void)
{
    if (verbose) puts("STUB: NWPBStartBrowseReadFrom called");
    return NULL;
}

void* NWPBStopBrowseReadFrom(void)
{
    if (verbose) puts("STUB: NWPBStopBrowseReadFrom called");
    return NULL;
}

void* NWPBUpdateBrowseReadFrom(void)
{
    if (verbose) puts("STUB: NWPBUpdateBrowseReadFrom called");
    return NULL;
}

void* NWPBUpdatePathReadFrom(void)
{
    if (verbose) puts("STUB: NWPBUpdatePathReadFrom called");
    return NULL;
}

void* NWPrivilegedHelperHandleEvent(void)
{
    if (verbose) puts("STUB: NWPrivilegedHelperHandleEvent called");
    return NULL;
}

void* NWPrivilegedHelperInitialize(void)
{
    if (verbose) puts("STUB: NWPrivilegedHelperInitialize called");
    return NULL;
}

void* NWPrivilegedHelperStart(void)
{
    if (verbose) puts("STUB: NWPrivilegedHelperStart called");
    return NULL;
}

void* NWUtilsCreateNSDictionaryFromXPCDictionary(void)
{
    if (verbose) puts("STUB: NWUtilsCreateNSDictionaryFromXPCDictionary called");
    return NULL;
}

void* NWUtilsCreateNSStringFromSockaddr(void)
{
    if (verbose) puts("STUB: NWUtilsCreateNSStringFromSockaddr called");
    return NULL;
}

void* NWUtilsCreateXPCDictionaryFromNSDictionary(void)
{
    if (verbose) puts("STUB: NWUtilsCreateXPCDictionaryFromNSDictionary called");
    return NULL;
}

void* NWUtilsStringMatchesPattern(void)
{
    if (verbose) puts("STUB: NWUtilsStringMatchesPattern called");
    return NULL;
}

void* updateOSLogPlistFromLevel(void)
{
    if (verbose) puts("STUB: updateOSLogPlistFromLevel called");
    return NULL;
}
