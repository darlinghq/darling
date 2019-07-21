/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <GameController/GameController.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ADClientAddValueForScalarKey(void)
{
    if (verbose) puts("STUB: ADClientAddValueForScalarKey called");
    return NULL;
}

void* GCCommonGetPreferencesBooleanValue(void)
{
    if (verbose) puts("STUB: GCCommonGetPreferencesBooleanValue called");
    return NULL;
}

void* GCControllerManagerInit(void)
{
    if (verbose) puts("STUB: GCControllerManagerInit called");
    return NULL;
}

void* GCControllerManagerInitXPC(void)
{
    if (verbose) puts("STUB: GCControllerManagerInitXPC called");
    return NULL;
}

void* GCExtendedGamepadSnapShotDataV100FromNSData(void)
{
    if (verbose) puts("STUB: GCExtendedGamepadSnapShotDataV100FromNSData called");
    return NULL;
}

void* GCExtendedGamepadSnapshotDataFromNSData(void)
{
    if (verbose) puts("STUB: GCExtendedGamepadSnapshotDataFromNSData called");
    return NULL;
}

void* GCGamepadSnapShotDataV100FromNSData(void)
{
    if (verbose) puts("STUB: GCGamepadSnapShotDataV100FromNSData called");
    return NULL;
}

void* GCMicroGamepadSnapShotDataV100FromNSData(void)
{
    if (verbose) puts("STUB: GCMicroGamepadSnapShotDataV100FromNSData called");
    return NULL;
}

void* GCMicroGamepadSnapshotDataFromNSData(void)
{
    if (verbose) puts("STUB: GCMicroGamepadSnapshotDataFromNSData called");
    return NULL;
}

void* NSDataFromGCExtendedGamepadSnapShotDataV100(void)
{
    if (verbose) puts("STUB: NSDataFromGCExtendedGamepadSnapShotDataV100 called");
    return NULL;
}

void* NSDataFromGCExtendedGamepadSnapshotData(void)
{
    if (verbose) puts("STUB: NSDataFromGCExtendedGamepadSnapshotData called");
    return NULL;
}

void* NSDataFromGCGamepadSnapShotDataV100(void)
{
    if (verbose) puts("STUB: NSDataFromGCGamepadSnapShotDataV100 called");
    return NULL;
}

void* NSDataFromGCMicroGamepadSnapShotDataV100(void)
{
    if (verbose) puts("STUB: NSDataFromGCMicroGamepadSnapShotDataV100 called");
    return NULL;
}

void* NSDataFromGCMicroGamepadSnapshotData(void)
{
    if (verbose) puts("STUB: NSDataFromGCMicroGamepadSnapshotData called");
    return NULL;
}

void* _GCHIDEventCallback(void)
{
    if (verbose) puts("STUB: _GCHIDEventCallback called");
    return NULL;
}

void* clamp(void)
{
    if (verbose) puts("STUB: clamp called");
    return NULL;
}

void* threadRunLoopSourceCallback(void)
{
    if (verbose) puts("STUB: threadRunLoopSourceCallback called");
    return NULL;
}
