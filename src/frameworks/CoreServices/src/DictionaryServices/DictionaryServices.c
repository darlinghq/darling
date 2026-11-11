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

#include <DictionaryServices/DictionaryServices.h>

// Public entry points. No dictionary records can be looked up yet, so no term is found.
CFRange DCSGetTermRangeInString(DCSDictionaryRef dictionary, CFStringRef textString, CFIndex offset)
{
    return CFRangeMake(kCFNotFound, 0);
}

CFStringRef DCSCopyTextDefinition(DCSDictionaryRef dictionary, CFStringRef textString, CFRange range)
{
    return NULL;
}

CFRange DCSGetTermRangeInStringWithOptions(void *dictionary, CFStringRef string, CFIndex offset, void *options)
{
    return CFRangeMake(kCFNotFound, 0);
}

void* DCSCopyRecordForReference(void)
{
    return NULL;
}

void* DCSCopyRecordsForSearchString(void)
{
    return NULL;
}

void* DCSCopyRecordsWithHeadword(void)
{
    return NULL;
}

void* DCSCreateHeadwordList(void)
{
    return NULL;
}

void* DCSCreateUserDictionariesDirectory(void)
{
    return NULL;
}

void* DCSDictionaryAssetCopyDiagnosticLog(void)
{
    return NULL;
}

void* DCSDictionaryDownloadFinished(void)
{
    return NULL;
}

void* DCSDictionaryGetAssetObj(void)
{
    return NULL;
}

void* DCSDictionaryGetBaseURL(void)
{
    return NULL;
}

void* DCSDictionaryGetParentDictionary(void)
{
    return NULL;
}

void* DCSDictionaryGetPreference(void)
{
    return NULL;
}

void* DCSDictionaryGetPreferenceHTML(void)
{
    return NULL;
}

void* DCSDictionaryGetPrimaryLanguage(void)
{
    return NULL;
}

void* DCSDictionaryGetShortName(void)
{
    return NULL;
}

void* DCSDictionaryGetStyleSheetURL(void)
{
    return NULL;
}

void* DCSDictionaryGetSubDictionaries(void)
{
    return NULL;
}

void* DCSDictionaryIsLanguageDictionary(void)
{
    return NULL;
}

void* DCSDictionaryIsNetworkService(void)
{
    return NULL;
}

void* DCSDictionarySetDataHeader(void)
{
    return NULL;
}

void* DCSDictionarySetPreference(void)
{
    return NULL;
}

void* DCSInvalidateDictionaryCache(void)
{
    return NULL;
}

void* DCSNormalizeSearchString(void)
{
    return NULL;
}

void* DCSPrepareMobileAssetQuery(void)
{
    return NULL;
}

void* DCSRecordCopyData(void)
{
    return NULL;
}

void* DCSRecordCopyDataURL(void)
{
    return NULL;
}

void* DCSRecordGetAnchor(void)
{
    return NULL;
}

void* DCSRecordGetAssociatedObj(void)
{
    return NULL;
}

void* DCSRecordGetDictionary(void)
{
    return NULL;
}

void* DCSRecordGetHeadword(void)
{
    return NULL;
}

void* DCSRecordGetRawHeadword(void)
{
    return NULL;
}

void* DCSRecordGetString(void)
{
    return NULL;
}

void* DCSRecordGetSubDictionary(void)
{
    return NULL;
}

void* DCSRecordGetTitle(void)
{
    return NULL;
}

void* DCSRecordSetAssociatedObj(void)
{
    return NULL;
}

void* DCSRecordSetHeadword(void)
{
    return NULL;
}

void* DCSSearchSessionCreate(void)
{
    return NULL;
}

void* DCSSearchSessionScheduleWithRunLoop(void)
{
    return NULL;
}

void* DCSSearchSessionUnscheduleFromRunLoop(void)
{
    return NULL;
}

void* DCSSetServicePresentationType(void)
{
    return NULL;
}

void* DCSSortRecordsWithHeadword(void)
{
    return NULL;
}

CFStringRef kDCSActiveDictionariesChangedNotification = CFSTR("DCSActiveDictionariesChangedNotification");
CFStringRef kDCSDictionaryDescriptionLanguage = CFSTR("DCSDictionaryDescriptionLanguage");
CFStringRef kDCSDictionaryIndexLanguage = CFSTR("DCSDictionaryIndexLanguage");
