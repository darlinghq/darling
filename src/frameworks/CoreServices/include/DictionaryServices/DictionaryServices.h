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

#ifndef _DictionaryServices_H_
#define _DictionaryServices_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct __DCSDictionary* DCSDictionaryRef;

// Dictionary discovery and the active set are implemented; record lookup is still stubbed,
// so lookups find nothing. Remaining stubs take and return untyped values.

CFRange DCSGetTermRangeInStringWithOptions(void *dictionary, CFStringRef string, CFIndex offset, void *options);

CFSetRef DCSCopyAvailableDictionaries(void);
void* DCSCopyRecordForReference(void);
void* DCSCopyRecordsForSearchString(void);
void* DCSCopyRecordsWithHeadword(void);
void* DCSCreateHeadwordList(void);
void* DCSCreateUserDictionariesDirectory(void);
void* DCSDictionaryAssetCopyDiagnosticLog(void);
DCSDictionaryRef DCSDictionaryCreate(CFURLRef url);
void* DCSDictionaryDownloadFinished(void);
void* DCSDictionaryGetAssetObj(void);
void* DCSDictionaryGetBaseURL(void);
CFStringRef DCSDictionaryGetIdentifier(DCSDictionaryRef dictionary);
CFArrayRef DCSDictionaryGetLanguages(DCSDictionaryRef dictionary);
CFStringRef DCSDictionaryGetName(DCSDictionaryRef dictionary);
void* DCSDictionaryGetParentDictionary(void);
void* DCSDictionaryGetPreference(void);
void* DCSDictionaryGetPreferenceHTML(void);
void* DCSDictionaryGetPrimaryLanguage(void);
void* DCSDictionaryGetShortName(void);
void* DCSDictionaryGetStyleSheetURL(void);
void* DCSDictionaryGetSubDictionaries(void);
CFURLRef DCSDictionaryGetURL(DCSDictionaryRef dictionary);
void* DCSDictionaryIsLanguageDictionary(void);
void* DCSDictionaryIsNetworkService(void);
void* DCSDictionarySetDataHeader(void);
void* DCSDictionarySetPreference(void);
CFArrayRef DCSGetActiveDictionaries(void);
void* DCSInvalidateDictionaryCache(void);
void* DCSNormalizeSearchString(void);
void* DCSPrepareMobileAssetQuery(void);
void* DCSRecordCopyData(void);
void* DCSRecordCopyDataURL(void);
void* DCSRecordGetAnchor(void);
void* DCSRecordGetAssociatedObj(void);
void* DCSRecordGetDictionary(void);
void* DCSRecordGetHeadword(void);
void* DCSRecordGetRawHeadword(void);
void* DCSRecordGetString(void);
void* DCSRecordGetSubDictionary(void);
void* DCSRecordGetTitle(void);
void* DCSRecordSetAssociatedObj(void);
void* DCSRecordSetHeadword(void);
void* DCSSearchSessionCreate(void);
void* DCSSearchSessionScheduleWithRunLoop(void);
void* DCSSearchSessionUnscheduleFromRunLoop(void);
void DCSSetActiveDictionaries(CFArrayRef dictionaries);
void* DCSSetServicePresentationType(void);
void* DCSSortRecordsWithHeadword(void);

extern CFStringRef kDCSActiveDictionariesChangedNotification;
extern CFStringRef kDCSDictionaryDescriptionLanguage;
extern CFStringRef kDCSDictionaryIndexLanguage;

#ifdef __cplusplus
}
#endif

#endif
