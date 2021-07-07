/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <ServerInformation/ServerInformation.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* SICopyLocalizedServerProductName(void)
{
    if (verbose) puts("STUB: SICopyLocalizedServerProductName called");
    return NULL;
}

void* SICopyMinimumServerVersion(void)
{
    if (verbose) puts("STUB: SICopyMinimumServerVersion called");
    return NULL;
}

void* SICopyServerBuildVersion(void)
{
    if (verbose) puts("STUB: SICopyServerBuildVersion called");
    return NULL;
}

void* SICopyServerVersion(void)
{
    if (verbose) puts("STUB: SICopyServerVersion called");
    return NULL;
}

void* SIGetServerInstallPathPrefix(void)
{
    if (verbose) puts("STUB: SIGetServerInstallPathPrefix called");
    return NULL;
}

void* SIGetServerPerformanceModeEnabled(void)
{
    if (verbose) puts("STUB: SIGetServerPerformanceModeEnabled called");
    return NULL;
}

void* SIIsOSXServerVolume(void)
{
    if (verbose) puts("STUB: SIIsOSXServerVolume called");
    return NULL;
}

void* SIIsOSXServerVolumeConfigured(void)
{
    if (verbose) puts("STUB: SIIsOSXServerVolumeConfigured called");
    return NULL;
}

void* SIIsServerHardware(void)
{
    if (verbose) puts("STUB: SIIsServerHardware called");
    return NULL;
}

void* SIIsXsanCapable(void)
{
    if (verbose) puts("STUB: SIIsXsanCapable called");
    return NULL;
}

void* SISetServerPerformanceModeEnabled(void)
{
    if (verbose) puts("STUB: SISetServerPerformanceModeEnabled called");
    return NULL;
}

void* _SIIsOSXVolumeVersionEqualToOrGreater(void)
{
    if (verbose) puts("STUB: _SIIsOSXVolumeVersionEqualToOrGreater called");
    return NULL;
}
