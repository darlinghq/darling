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


#include <ImageCaptureCore/ImageCaptureCore.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* _ICCreateCGColorSpace(void)
{
    if (verbose) puts("STUB: _ICCreateCGColorSpace called");
    return NULL;
}

void* _ICCreateDeviceMatchingDeviceID(void)
{
    if (verbose) puts("STUB: _ICCreateDeviceMatchingDeviceID called");
    return NULL;
}

void* _ICDidFindLegacyDevice(void)
{
    if (verbose) puts("STUB: _ICDidFindLegacyDevice called");
    return NULL;
}

void* _ICDidInstallNewImageCaptureSoftware(void)
{
    if (verbose) puts("STUB: _ICDidInstallNewImageCaptureSoftware called");
    return NULL;
}

void* _ICDidRemoveLegacyDevice(void)
{
    if (verbose) puts("STUB: _ICDidRemoveLegacyDevice called");
    return NULL;
}

void* _ICImageCaptureGetScannerButtonMappingPreference(void)
{
    if (verbose) puts("STUB: _ICImageCaptureGetScannerButtonMappingPreference called");
    return NULL;
}

void* _ICImageCaptureSetScannerButtonMappingPreference(void)
{
    if (verbose) puts("STUB: _ICImageCaptureSetScannerButtonMappingPreference called");
    return NULL;
}

void* _ICInitialize(void)
{
    if (verbose) puts("STUB: _ICInitialize called");
    return NULL;
}

void* _ICIsValidDevice(void)
{
    if (verbose) puts("STUB: _ICIsValidDevice called");
    return NULL;
}

void* _ICSetSharingPreferences(void)
{
    if (verbose) puts("STUB: _ICSetSharingPreferences called");
    return NULL;
}

void* _ICStartReceivingSharingPreferences(void)
{
    if (verbose) puts("STUB: _ICStartReceivingSharingPreferences called");
    return NULL;
}

void* __ICAddDecodedErrorToDict(void)
{
    if (verbose) puts("STUB: __ICAddDecodedErrorToDict called");
    return NULL;
}

void* __ICLogDateString(void)
{
    if (verbose) puts("STUB: __ICLogDateString called");
    return NULL;
}

void* __ICLogTypeEnabled(void)
{
    if (verbose) puts("STUB: __ICLogTypeEnabled called");
    return NULL;
}

void* __ICLongValueToErrorString(void)
{
    if (verbose) puts("STUB: __ICLongValueToErrorString called");
    return NULL;
}

void* __ICOSLogCreate(void)
{
    if (verbose) puts("STUB: __ICOSLogCreate called");
    return NULL;
}

void* __ICReadPrefs(void)
{
    if (verbose) puts("STUB: __ICReadPrefs called");
    return NULL;
}

void* __ICValueToErrorString(void)
{
    if (verbose) puts("STUB: __ICValueToErrorString called");
    return NULL;
}
