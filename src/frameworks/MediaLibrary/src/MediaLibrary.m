/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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


#include <MediaLibrary/MediaLibrary.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* MLLog(void)
{
    if (verbose) puts("STUB: MLLog called");
    return NULL;
}

void* MLLogWithLevel(void)
{
    if (verbose) puts("STUB: MLLogWithLevel called");
    return NULL;
}

void* MLPMRInstrument_private_createPool(void)
{
    if (verbose) puts("STUB: MLPMRInstrument_private_createPool called");
    return NULL;
}

void* MLPMRInstrument_private_deletePool(void)
{
    if (verbose) puts("STUB: MLPMRInstrument_private_deletePool called");
    return NULL;
}

void* _Z16ensurePathExistsP8NSString(void)
{
    if (verbose) puts("STUB: _Z16ensurePathExistsP8NSString called");
    return NULL;
}

void* _Z26mlpmrContext_cppInstrumentPK10__CFString(void)
{
    if (verbose) puts("STUB: _Z26mlpmrContext_cppInstrumentPK10__CFString called");
    return NULL;
}

void* _ZN22MLPMRInstrumentFactory16createInstrumentEP12MLPMRContextPK10__CFStringS4_(void)
{
    if (verbose) puts("STUB: _ZN22MLPMRInstrumentFactory16createInstrumentEP12MLPMRContextPK10__CFStringS4_ called");
    return NULL;
}

void* ilmbpmr_getCurrentTime(void)
{
    if (verbose) puts("STUB: ilmbpmr_getCurrentTime called");
    return NULL;
}

void* mlpmrContext_applicationName(void)
{
    if (verbose) puts("STUB: mlpmrContext_applicationName called");
    return NULL;
}

void* mlpmrContext_instrument(void)
{
    if (verbose) puts("STUB: mlpmrContext_instrument called");
    return NULL;
}

void* mlpmrContext_logPath(void)
{
    if (verbose) puts("STUB: mlpmrContext_logPath called");
    return NULL;
}

void* mlpmrContext_setParameters(void)
{
    if (verbose) puts("STUB: mlpmrContext_setParameters called");
    return NULL;
}

void* mlpmrFini(void)
{
    if (verbose) puts("STUB: mlpmrFini called");
    return NULL;
}

void* mlpmrInit(void)
{
    if (verbose) puts("STUB: mlpmrInit called");
    return NULL;
}

void* mlpmrInstrument_logBool(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logBool called");
    return NULL;
}

void* mlpmrInstrument_logDouble(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logDouble called");
    return NULL;
}

void* mlpmrInstrument_logSInt32(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logSInt32 called");
    return NULL;
}

void* mlpmrInstrument_logSInt64(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logSInt64 called");
    return NULL;
}

void* mlpmrInstrument_logString(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logString called");
    return NULL;
}

void* mlpmrInstrument_logUInt32(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logUInt32 called");
    return NULL;
}

void* mlpmrInstrument_logUInt64(void)
{
    if (verbose) puts("STUB: mlpmrInstrument_logUInt64 called");
    return NULL;
}

void* mlpmrTimer_begin(void)
{
    if (verbose) puts("STUB: mlpmrTimer_begin called");
    return NULL;
}

void* mlpmrTimer_delta(void)
{
    if (verbose) puts("STUB: mlpmrTimer_delta called");
    return NULL;
}

void* mlpmrTimer_end(void)
{
    if (verbose) puts("STUB: mlpmrTimer_end called");
    return NULL;
}

void* mlpmrTimer_init(void)
{
    if (verbose) puts("STUB: mlpmrTimer_init called");
    return NULL;
}

void* mlpmrTimer_pause(void)
{
    if (verbose) puts("STUB: mlpmrTimer_pause called");
    return NULL;
}

void* mlpmrTimer_resume(void)
{
    if (verbose) puts("STUB: mlpmrTimer_resume called");
    return NULL;
}
