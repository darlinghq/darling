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


#include <ImageCapture/ImageCapture.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ICACloseSession(void)
{
    if (verbose) puts("STUB: ICACloseSession called");
    return NULL;
}

void* ICAConvertJPEGDataToICADataInPlace(void)
{
    if (verbose) puts("STUB: ICAConvertJPEGDataToICADataInPlace called");
    return NULL;
}

void* ICAConvertTIFFDataToICADataInPlace(void)
{
    if (verbose) puts("STUB: ICAConvertTIFFDataToICADataInPlace called");
    return NULL;
}

void* ICACopyMatchingDeviceDictionaries(void)
{
    if (verbose) puts("STUB: ICACopyMatchingDeviceDictionaries called");
    return NULL;
}

void* ICACopyObjectData(void)
{
    if (verbose) puts("STUB: ICACopyObjectData called");
    return NULL;
}

void* ICACopyObjectPropertyDictionary(void)
{
    if (verbose) puts("STUB: ICACopyObjectPropertyDictionary called");
    return NULL;
}

void* ICACopyObjectThumbnail(void)
{
    if (verbose) puts("STUB: ICACopyObjectThumbnail called");
    return NULL;
}

void* ICACopyParameterBlockPtrForAsyncCall(void)
{
    if (verbose) puts("STUB: ICACopyParameterBlockPtrForAsyncCall called");
    return NULL;
}

void* ICACreateCutsomIconForImageAtPath(void)
{
    if (verbose) puts("STUB: ICACreateCutsomIconForImageAtPath called");
    return NULL;
}

void* ICACreateJPEGThumbnailDataFromURL(void)
{
    if (verbose) puts("STUB: ICACreateJPEGThumbnailDataFromURL called");
    return NULL;
}

void* ICADictionaryApplierFunction(void)
{
    if (verbose) puts("STUB: ICADictionaryApplierFunction called");
    return NULL;
}

void* ICADoMacho2CFMConversion(void)
{
    if (verbose) puts("STUB: ICADoMacho2CFMConversion called");
    return NULL;
}

void* ICADownloadFile(void)
{
    if (verbose) puts("STUB: ICADownloadFile called");
    return NULL;
}

void* ICADownloadFileWithOptions(void)
{
    if (verbose) puts("STUB: ICADownloadFileWithOptions called");
    return NULL;
}

void* ICAElapsedTime(void)
{
    if (verbose) puts("STUB: ICAElapsedTime called");
    return NULL;
}

void* ICAGetBehaviorFlags(void)
{
    if (verbose) puts("STUB: ICAGetBehaviorFlags called");
    return NULL;
}

void* ICAGetChildCount(void)
{
    if (verbose) puts("STUB: ICAGetChildCount called");
    return NULL;
}

void* ICAGetDateFromSeconds(void)
{
    if (verbose) puts("STUB: ICAGetDateFromSeconds called");
    return NULL;
}

void* ICAGetDateFromSecondsSince1904(void)
{
    if (verbose) puts("STUB: ICAGetDateFromSecondsSince1904 called");
    return NULL;
}

void* ICAGetDateFromSecondsSince1970(void)
{
    if (verbose) puts("STUB: ICAGetDateFromSecondsSince1970 called");
    return NULL;
}

void* ICAGetDeviceList(void)
{
    if (verbose) puts("STUB: ICAGetDeviceList called");
    return NULL;
}

void* ICAGetEXIFDateCStringFromSecondsSince1904(void)
{
    if (verbose) puts("STUB: ICAGetEXIFDateCStringFromSecondsSince1904 called");
    return NULL;
}

void* ICAGetEXIFDateCStringFromSecondsSince1970(void)
{
    if (verbose) puts("STUB: ICAGetEXIFDateCStringFromSecondsSince1970 called");
    return NULL;
}

void* ICAGetEXIFUTCDateCStringFromSecondsSince1904(void)
{
    if (verbose) puts("STUB: ICAGetEXIFUTCDateCStringFromSecondsSince1904 called");
    return NULL;
}

void* ICAGetEXIFUTCDateCStringFromSecondsSince1970(void)
{
    if (verbose) puts("STUB: ICAGetEXIFUTCDateCStringFromSecondsSince1970 called");
    return NULL;
}

void* ICAGetICAObjectForFile(void)
{
    if (verbose) puts("STUB: ICAGetICAObjectForFile called");
    return NULL;
}

void* ICAGetICAObjectFromDict(void)
{
    if (verbose) puts("STUB: ICAGetICAObjectFromDict called");
    return NULL;
}

void* ICAGetLongValueFromDict(void)
{
    if (verbose) puts("STUB: ICAGetLongValueFromDict called");
    return NULL;
}

void* ICAGetNthChild(void)
{
    if (verbose) puts("STUB: ICAGetNthChild called");
    return NULL;
}

void* ICAGetNthProperty(void)
{
    if (verbose) puts("STUB: ICAGetNthProperty called");
    return NULL;
}

void* ICAGetObjectInfo(void)
{
    if (verbose) puts("STUB: ICAGetObjectInfo called");
    return NULL;
}

void* ICAGetObjectRefCon(void)
{
    if (verbose) puts("STUB: ICAGetObjectRefCon called");
    return NULL;
}

void* ICAGetParentOfObject(void)
{
    if (verbose) puts("STUB: ICAGetParentOfObject called");
    return NULL;
}

void* ICAGetParentOfProperty(void)
{
    if (verbose) puts("STUB: ICAGetParentOfProperty called");
    return NULL;
}

void* ICAGetPropertyByType(void)
{
    if (verbose) puts("STUB: ICAGetPropertyByType called");
    return NULL;
}

void* ICAGetPropertyCount(void)
{
    if (verbose) puts("STUB: ICAGetPropertyCount called");
    return NULL;
}

void* ICAGetPropertyData(void)
{
    if (verbose) puts("STUB: ICAGetPropertyData called");
    return NULL;
}

void* ICAGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICAGetPropertyInfo called");
    return NULL;
}

void* ICAGetPropertyRefCon(void)
{
    if (verbose) puts("STUB: ICAGetPropertyRefCon called");
    return NULL;
}

void* ICAGetRootOfObject(void)
{
    if (verbose) puts("STUB: ICAGetRootOfObject called");
    return NULL;
}

void* ICAGetRootOfProperty(void)
{
    if (verbose) puts("STUB: ICAGetRootOfProperty called");
    return NULL;
}

void* ICAGetSInt16ValueFromDict(void)
{
    if (verbose) puts("STUB: ICAGetSInt16ValueFromDict called");
    return NULL;
}

void* ICAGetSInt32ValueFromDict(void)
{
    if (verbose) puts("STUB: ICAGetSInt32ValueFromDict called");
    return NULL;
}

void* ICAGetSInt64ValueFromDict(void)
{
    if (verbose) puts("STUB: ICAGetSInt64ValueFromDict called");
    return NULL;
}

void* ICAGetUTCDateFromDateCString(void)
{
    if (verbose) puts("STUB: ICAGetUTCDateFromDateCString called");
    return NULL;
}

void* ICAGetUTCDateFromSeconds(void)
{
    if (verbose) puts("STUB: ICAGetUTCDateFromSeconds called");
    return NULL;
}

void* ICAGetUTCDateFromSecondsSince1904(void)
{
    if (verbose) puts("STUB: ICAGetUTCDateFromSecondsSince1904 called");
    return NULL;
}

void* ICAImportImage(void)
{
    if (verbose) puts("STUB: ICAImportImage called");
    return NULL;
}

void* ICALoadDeviceModule(void)
{
    if (verbose) puts("STUB: ICALoadDeviceModule called");
    return NULL;
}

void* ICAObjectSendMessage(void)
{
    if (verbose) puts("STUB: ICAObjectSendMessage called");
    return NULL;
}

void* ICAOpenSession(void)
{
    if (verbose) puts("STUB: ICAOpenSession called");
    return NULL;
}

void* ICARegisterEventNotification(void)
{
    if (verbose) puts("STUB: ICARegisterEventNotification called");
    return NULL;
}

void* ICARegisterForEventNotification(void)
{
    if (verbose) puts("STUB: ICARegisterForEventNotification called");
    return NULL;
}

void* ICAScannerCloseSession(void)
{
    if (verbose) puts("STUB: ICAScannerCloseSession called");
    return NULL;
}

void* ICAScannerGetParameters(void)
{
    if (verbose) puts("STUB: ICAScannerGetParameters called");
    return NULL;
}

void* ICAScannerInitialize(void)
{
    if (verbose) puts("STUB: ICAScannerInitialize called");
    return NULL;
}

void* ICAScannerOpenSession(void)
{
    if (verbose) puts("STUB: ICAScannerOpenSession called");
    return NULL;
}

void* ICAScannerSetParameters(void)
{
    if (verbose) puts("STUB: ICAScannerSetParameters called");
    return NULL;
}

void* ICAScannerStart(void)
{
    if (verbose) puts("STUB: ICAScannerStart called");
    return NULL;
}

void* ICAScannerStatus(void)
{
    if (verbose) puts("STUB: ICAScannerStatus called");
    return NULL;
}

void* ICASetBehaviorFlags(void)
{
    if (verbose) puts("STUB: ICASetBehaviorFlags called");
    return NULL;
}

void* ICASetICAObjectInDict(void)
{
    if (verbose) puts("STUB: ICASetICAObjectInDict called");
    return NULL;
}

void* ICASetLongValueInDict(void)
{
    if (verbose) puts("STUB: ICASetLongValueInDict called");
    return NULL;
}

void* ICASetObjectRefCon(void)
{
    if (verbose) puts("STUB: ICASetObjectRefCon called");
    return NULL;
}

void* ICASetPropertyData(void)
{
    if (verbose) puts("STUB: ICASetPropertyData called");
    return NULL;
}

void* ICASetPropertyRefCon(void)
{
    if (verbose) puts("STUB: ICASetPropertyRefCon called");
    return NULL;
}

void* ICASetSInt16ValueInDict(void)
{
    if (verbose) puts("STUB: ICASetSInt16ValueInDict called");
    return NULL;
}

void* ICASetSInt32ValueInDict(void)
{
    if (verbose) puts("STUB: ICASetSInt32ValueInDict called");
    return NULL;
}

void* ICASetSInt64ValueInDict(void)
{
    if (verbose) puts("STUB: ICASetSInt64ValueInDict called");
    return NULL;
}

void* ICASetUTF8StringInDict(void)
{
    if (verbose) puts("STUB: ICASetUTF8StringInDict called");
    return NULL;
}

void* ICASharedMemoryCreate(void)
{
    if (verbose) puts("STUB: ICASharedMemoryCreate called");
    return NULL;
}

void* ICASharedMemoryDestroy(void)
{
    if (verbose) puts("STUB: ICASharedMemoryDestroy called");
    return NULL;
}

void* ICASharedMemoryMap(void)
{
    if (verbose) puts("STUB: ICASharedMemoryMap called");
    return NULL;
}

void* ICASharedMemoryUnmap(void)
{
    if (verbose) puts("STUB: ICASharedMemoryUnmap called");
    return NULL;
}

void* ICAShowDeviceBrowser(void)
{
    if (verbose) puts("STUB: ICAShowDeviceBrowser called");
    return NULL;
}

void* ICAUnloadDeviceModule(void)
{
    if (verbose) puts("STUB: ICAUnloadDeviceModule called");
    return NULL;
}

void* ICAUpdateDeviceDictionary(void)
{
    if (verbose) puts("STUB: ICAUpdateDeviceDictionary called");
    return NULL;
}

void* ICAUploadFile(void)
{
    if (verbose) puts("STUB: ICAUploadFile called");
    return NULL;
}

void* ICDBulkNewObject(void)
{
    if (verbose) puts("STUB: ICDBulkNewObject called");
    return NULL;
}

void* ICDBulkNewProperty(void)
{
    if (verbose) puts("STUB: ICDBulkNewProperty called");
    return NULL;
}

void* ICDDeviceNotification(void)
{
    if (verbose) puts("STUB: ICDDeviceNotification called");
    return NULL;
}

void* ICDDeviceRegister(void)
{
    if (verbose) puts("STUB: ICDDeviceRegister called");
    return NULL;
}

void* ICDDeviceUnRegister(void)
{
    if (verbose) puts("STUB: ICDDeviceUnRegister called");
    return NULL;
}

void* ICDDisposeObject(void)
{
    if (verbose) puts("STUB: ICDDisposeObject called");
    return NULL;
}

void* ICDDisposeProperty(void)
{
    if (verbose) puts("STUB: ICDDisposeProperty called");
    return NULL;
}

void* ICDNewObject(void)
{
    if (verbose) puts("STUB: ICDNewObject called");
    return NULL;
}

void* ICDNewProperty(void)
{
    if (verbose) puts("STUB: ICDNewProperty called");
    return NULL;
}
