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


#ifndef _ImageCapture_H_
#define _ImageCapture_H_

#import <Foundation/Foundation.h>


void* ICACloseSession(void);
void* ICAConvertJPEGDataToICADataInPlace(void);
void* ICAConvertTIFFDataToICADataInPlace(void);
void* ICACopyMatchingDeviceDictionaries(void);
void* ICACopyObjectData(void);
void* ICACopyObjectPropertyDictionary(void);
void* ICACopyObjectThumbnail(void);
void* ICACopyParameterBlockPtrForAsyncCall(void);
void* ICACreateCutsomIconForImageAtPath(void);
void* ICACreateJPEGThumbnailDataFromURL(void);
void* ICADictionaryApplierFunction(void);
void* ICADoMacho2CFMConversion(void);
void* ICADownloadFile(void);
void* ICADownloadFileWithOptions(void);
void* ICAElapsedTime(void);
void* ICAGetBehaviorFlags(void);
void* ICAGetChildCount(void);
void* ICAGetDateFromSeconds(void);
void* ICAGetDateFromSecondsSince1904(void);
void* ICAGetDateFromSecondsSince1970(void);
void* ICAGetDeviceList(void);
void* ICAGetEXIFDateCStringFromSecondsSince1904(void);
void* ICAGetEXIFDateCStringFromSecondsSince1970(void);
void* ICAGetEXIFUTCDateCStringFromSecondsSince1904(void);
void* ICAGetEXIFUTCDateCStringFromSecondsSince1970(void);
void* ICAGetICAObjectForFile(void);
void* ICAGetICAObjectFromDict(void);
void* ICAGetLongValueFromDict(void);
void* ICAGetNthChild(void);
void* ICAGetNthProperty(void);
void* ICAGetObjectInfo(void);
void* ICAGetObjectRefCon(void);
void* ICAGetParentOfObject(void);
void* ICAGetParentOfProperty(void);
void* ICAGetPropertyByType(void);
void* ICAGetPropertyCount(void);
void* ICAGetPropertyData(void);
void* ICAGetPropertyInfo(void);
void* ICAGetPropertyRefCon(void);
void* ICAGetRootOfObject(void);
void* ICAGetRootOfProperty(void);
void* ICAGetSInt16ValueFromDict(void);
void* ICAGetSInt32ValueFromDict(void);
void* ICAGetSInt64ValueFromDict(void);
void* ICAGetUTCDateFromDateCString(void);
void* ICAGetUTCDateFromSeconds(void);
void* ICAGetUTCDateFromSecondsSince1904(void);
void* ICAImportImage(void);
void* ICALoadDeviceModule(void);
void* ICAObjectSendMessage(void);
void* ICAOpenSession(void);
void* ICARegisterEventNotification(void);
void* ICARegisterForEventNotification(void);
void* ICAScannerCloseSession(void);
void* ICAScannerGetParameters(void);
void* ICAScannerInitialize(void);
void* ICAScannerOpenSession(void);
void* ICAScannerSetParameters(void);
void* ICAScannerStart(void);
void* ICAScannerStatus(void);
void* ICASetBehaviorFlags(void);
void* ICASetICAObjectInDict(void);
void* ICASetLongValueInDict(void);
void* ICASetObjectRefCon(void);
void* ICASetPropertyData(void);
void* ICASetPropertyRefCon(void);
void* ICASetSInt16ValueInDict(void);
void* ICASetSInt32ValueInDict(void);
void* ICASetSInt64ValueInDict(void);
void* ICASetUTF8StringInDict(void);
void* ICASharedMemoryCreate(void);
void* ICASharedMemoryDestroy(void);
void* ICASharedMemoryMap(void);
void* ICASharedMemoryUnmap(void);
void* ICAShowDeviceBrowser(void);
void* ICAUnloadDeviceModule(void);
void* ICAUpdateDeviceDictionary(void);
void* ICAUploadFile(void);
void* ICDBulkNewObject(void);
void* ICDBulkNewProperty(void);
void* ICDDeviceNotification(void);
void* ICDDeviceRegister(void);
void* ICDDeviceUnRegister(void);
void* ICDDisposeObject(void);
void* ICDDisposeProperty(void);
void* ICDNewObject(void);
void* ICDNewProperty(void);

#endif
