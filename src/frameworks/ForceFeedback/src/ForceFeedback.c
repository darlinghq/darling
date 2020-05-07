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


#include <ForceFeedback/ForceFeedback.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* FFCreateDevice(void)
{
    if (verbose) puts("STUB: FFCreateDevice called");
    return NULL;
}

void* FFDeviceCreateEffect(void)
{
    if (verbose) puts("STUB: FFDeviceCreateEffect called");
    return NULL;
}

void* FFDeviceEscape(void)
{
    if (verbose) puts("STUB: FFDeviceEscape called");
    return NULL;
}

void* FFDeviceGetForceFeedbackCapabilities(void)
{
    if (verbose) puts("STUB: FFDeviceGetForceFeedbackCapabilities called");
    return NULL;
}

void* FFDeviceGetForceFeedbackProperty(void)
{
    if (verbose) puts("STUB: FFDeviceGetForceFeedbackProperty called");
    return NULL;
}

void* FFDeviceGetForceFeedbackState(void)
{
    if (verbose) puts("STUB: FFDeviceGetForceFeedbackState called");
    return NULL;
}

void* FFDeviceReleaseEffect(void)
{
    if (verbose) puts("STUB: FFDeviceReleaseEffect called");
    return NULL;
}

void* FFDeviceSendForceFeedbackCommand(void)
{
    if (verbose) puts("STUB: FFDeviceSendForceFeedbackCommand called");
    return NULL;
}

void* FFDeviceSetCooperativeLevel(void)
{
    if (verbose) puts("STUB: FFDeviceSetCooperativeLevel called");
    return NULL;
}

void* FFDeviceSetForceFeedbackProperty(void)
{
    if (verbose) puts("STUB: FFDeviceSetForceFeedbackProperty called");
    return NULL;
}

void* FFEffectDownload(void)
{
    if (verbose) puts("STUB: FFEffectDownload called");
    return NULL;
}

void* FFEffectEscape(void)
{
    if (verbose) puts("STUB: FFEffectEscape called");
    return NULL;
}

void* FFEffectGetEffectStatus(void)
{
    if (verbose) puts("STUB: FFEffectGetEffectStatus called");
    return NULL;
}

void* FFEffectGetParameters(void)
{
    if (verbose) puts("STUB: FFEffectGetParameters called");
    return NULL;
}

void* FFEffectSetParameters(void)
{
    if (verbose) puts("STUB: FFEffectSetParameters called");
    return NULL;
}

void* FFEffectStart(void)
{
    if (verbose) puts("STUB: FFEffectStart called");
    return NULL;
}

void* FFEffectStop(void)
{
    if (verbose) puts("STUB: FFEffectStop called");
    return NULL;
}

void* FFEffectUnload(void)
{
    if (verbose) puts("STUB: FFEffectUnload called");
    return NULL;
}

void* FFIsForceFeedback(void)
{
    if (verbose) puts("STUB: FFIsForceFeedback called");
    return NULL;
}

void* FFReleaseDevice(void)
{
    if (verbose) puts("STUB: FFReleaseDevice called");
    return NULL;
}
