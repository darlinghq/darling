/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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


#include <AudioVideoBridging/AudioVideoBridging.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AVB17221AEMVideoColorSpaceBitsPerPixel(void)
{
    if (verbose) puts("STUB: AVB17221AEMVideoColorSpaceBitsPerPixel called");
    return NULL;
}

void* AVB17221AEMVideoColorSpaceFromCVPixelFormatType(void)
{
    if (verbose) puts("STUB: AVB17221AEMVideoColorSpaceFromCVPixelFormatType called");
    return NULL;
}

void* AVB17221AEMVideoColorSpaceFromIIDCVideoFormat(void)
{
    if (verbose) puts("STUB: AVB17221AEMVideoColorSpaceFromIIDCVideoFormat called");
    return NULL;
}

void* AVB17221AEMVideoColorSpaceIsSupportedByCVPixelFormatType(void)
{
    if (verbose) puts("STUB: AVB17221AEMVideoColorSpaceIsSupportedByCVPixelFormatType called");
    return NULL;
}

void* AVB17221AEMVideoColorSpaceSupportsCVPixelFormatType(void)
{
    if (verbose) puts("STUB: AVB17221AEMVideoColorSpaceSupportsCVPixelFormatType called");
    return NULL;
}

void* AVB17221AEMVideoColorSpaceToCVPixelFormatType(void)
{
    if (verbose) puts("STUB: AVB17221AEMVideoColorSpaceToCVPixelFormatType called");
    return NULL;
}

void* _Z11AAA_add_128yy(void)
{
    if (verbose) puts("STUB: _Z11AAA_add_128yy called");
    return NULL;
}

void* _Z13AAA_uint64mulyy(void)
{
    if (verbose) puts("STUB: _Z13AAA_uint64mulyy called");
    return NULL;
}

void* _ZNK8AAA_U128dvERKS_(void)
{
    if (verbose) puts("STUB: _ZNK8AAA_U128dvERKS_ called");
    return NULL;
}

void* _ZNK8AAA_U128mlERKS_(void)
{
    if (verbose) puts("STUB: _ZNK8AAA_U128mlERKS_ called");
    return NULL;
}

void* _ZmlRK8AAA_U128y(void)
{
    if (verbose) puts("STUB: _ZmlRK8AAA_U128y called");
    return NULL;
}

void* _ZmlyRK8AAA_U128(void)
{
    if (verbose) puts("STUB: _ZmlyRK8AAA_U128 called");
    return NULL;
}
