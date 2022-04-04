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


#include <DiscRecordingUI/DiscRecordingUI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DRBurnSessionBeginProgressDialog(void)
{
    if (verbose) puts("STUB: DRBurnSessionBeginProgressDialog called");
    return NULL;
}

void* DRBurnSessionCreate(void)
{
    if (verbose) puts("STUB: DRBurnSessionCreate called");
    return NULL;
}

void* DRBurnSessionGetBurn(void)
{
    if (verbose) puts("STUB: DRBurnSessionGetBurn called");
    return NULL;
}

void* DRBurnSessionGetTypeID(void)
{
    if (verbose) puts("STUB: DRBurnSessionGetTypeID called");
    return NULL;
}

void* DRBurnSessionSetBurn(void)
{
    if (verbose) puts("STUB: DRBurnSessionSetBurn called");
    return NULL;
}

void* DRBurnSessionSetupDialog(void)
{
    if (verbose) puts("STUB: DRBurnSessionSetupDialog called");
    return NULL;
}

void* DREraseSessionBeginProgressDialog(void)
{
    if (verbose) puts("STUB: DREraseSessionBeginProgressDialog called");
    return NULL;
}

void* DREraseSessionCreate(void)
{
    if (verbose) puts("STUB: DREraseSessionCreate called");
    return NULL;
}

void* DREraseSessionGetErase(void)
{
    if (verbose) puts("STUB: DREraseSessionGetErase called");
    return NULL;
}

void* DREraseSessionGetTypeID(void)
{
    if (verbose) puts("STUB: DREraseSessionGetTypeID called");
    return NULL;
}

void* DREraseSessionSetErase(void)
{
    if (verbose) puts("STUB: DREraseSessionSetErase called");
    return NULL;
}

void* DREraseSessionSetupDialog(void)
{
    if (verbose) puts("STUB: DREraseSessionSetupDialog called");
    return NULL;
}
