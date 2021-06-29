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


#include <ScreenSaver/ScreenSaver.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* OSLogForCategory(void)
{
    if (verbose) puts("STUB: OSLogForCategory called");
    return NULL;
}

void* SSCenterRectInRect(void)
{
    if (verbose) puts("STUB: SSCenterRectInRect called");
    return NULL;
}

void* SSGetScaleFactor(void)
{
    if (verbose) puts("STUB: SSGetScaleFactor called");
    return NULL;
}

void* SSHasGfxAcceleration(void)
{
    if (verbose) puts("STUB: SSHasGfxAcceleration called");
    return NULL;
}

void* SSInitRandom(void)
{
    if (verbose) puts("STUB: SSInitRandom called");
    return NULL;
}

void* SSInitRandomWithSeed(void)
{
    if (verbose) puts("STUB: SSInitRandomWithSeed called");
    return NULL;
}

void* SSIsAppleSignedModule(void)
{
    if (verbose) puts("STUB: SSIsAppleSignedModule called");
    return NULL;
}

void* SSModuleIsCompatibleArch(void)
{
    if (verbose) puts("STUB: SSModuleIsCompatibleArch called");
    return NULL;
}

void* SSPreflightModuleLoadForPath(void)
{
    if (verbose) puts("STUB: SSPreflightModuleLoadForPath called");
    return NULL;
}

void* SSRandomFloat(void)
{
    if (verbose) puts("STUB: SSRandomFloat called");
    return NULL;
}

void* SSRandomInt(void)
{
    if (verbose) puts("STUB: SSRandomInt called");
    return NULL;
}

void* SSRandomPointForSizeWithinBounds(void)
{
    if (verbose) puts("STUB: SSRandomPointForSizeWithinBounds called");
    return NULL;
}

void* SSRunningAsNormalUser(void)
{
    if (verbose) puts("STUB: SSRunningAsNormalUser called");
    return NULL;
}

void* SSScreenIsMirrored(void)
{
    if (verbose) puts("STUB: SSScreenIsMirrored called");
    return NULL;
}

void* SSScreenSaverWindowLevel(void)
{
    if (verbose) puts("STUB: SSScreenSaverWindowLevel called");
    return NULL;
}

void* SSSetCrashReporterSignature(void)
{
    if (verbose) puts("STUB: SSSetCrashReporterSignature called");
    return NULL;
}

void* SSShouldUseAlernateDefault(void)
{
    if (verbose) puts("STUB: SSShouldUseAlernateDefault called");
    return NULL;
}

void* SSSystemScreenSaversPath(void)
{
    if (verbose) puts("STUB: SSSystemScreenSaversPath called");
    return NULL;
}
