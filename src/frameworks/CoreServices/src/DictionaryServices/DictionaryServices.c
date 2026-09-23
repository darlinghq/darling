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
#include <stdio.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

CFRange DCSGetTermRangeInStringWithOptions(void *dictionary, CFStringRef string, CFIndex offset, void *options)
{
    if (verbose) puts("STUB: DCSGetTermRangeInStringWithOptions called");
    return CFRangeMake(kCFNotFound, 0);
}

void* DCSCopyRecordForReference(void)
{
    if (verbose) puts("STUB: DCSCopyRecordForReference called");
    return NULL;
}

void* DCSCopyRecordsForSearchString(void)
{
    if (verbose) puts("STUB: DCSCopyRecordsForSearchString called");
    return NULL;
}

void* DCSCopyRecordsWithHeadword(void)
{
    if (verbose) puts("STUB: DCSCopyRecordsWithHeadword called");
    return NULL;
}

void* DCSCreateHeadwordList(void)
{
    if (verbose) puts("STUB: DCSCreateHeadwordList called");
    return NULL;
}

void* DCSCreateUserDictionariesDirectory(void)
{
    if (verbose) puts("STUB: DCSCreateUserDictionariesDirectory called");
    return NULL;
}

void* DCSDictionaryAssetCopyDiagnosticLog(void)
{
    if (verbose) puts("STUB: DCSDictionaryAssetCopyDiagnosticLog called");
    return NULL;
}

void* DCSDictionaryDownloadFinished(void)
{
    if (verbose) puts("STUB: DCSDictionaryDownloadFinished called");
    return NULL;
}

void* DCSDictionaryGetAssetObj(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetAssetObj called");
    return NULL;
}

void* DCSDictionaryGetBaseURL(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetBaseURL called");
    return NULL;
}

void* DCSDictionaryGetParentDictionary(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetParentDictionary called");
    return NULL;
}

void* DCSDictionaryGetPreference(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetPreference called");
    return NULL;
}

void* DCSDictionaryGetPreferenceHTML(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetPreferenceHTML called");
    return NULL;
}

void* DCSDictionaryGetPrimaryLanguage(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetPrimaryLanguage called");
    return NULL;
}

void* DCSDictionaryGetShortName(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetShortName called");
    return NULL;
}

void* DCSDictionaryGetStyleSheetURL(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetStyleSheetURL called");
    return NULL;
}

void* DCSDictionaryGetSubDictionaries(void)
{
    if (verbose) puts("STUB: DCSDictionaryGetSubDictionaries called");
    return NULL;
}

void* DCSDictionaryIsLanguageDictionary(void)
{
    if (verbose) puts("STUB: DCSDictionaryIsLanguageDictionary called");
    return NULL;
}

void* DCSDictionaryIsNetworkService(void)
{
    if (verbose) puts("STUB: DCSDictionaryIsNetworkService called");
    return NULL;
}

void* DCSDictionarySetDataHeader(void)
{
    if (verbose) puts("STUB: DCSDictionarySetDataHeader called");
    return NULL;
}

void* DCSDictionarySetPreference(void)
{
    if (verbose) puts("STUB: DCSDictionarySetPreference called");
    return NULL;
}

void* DCSInvalidateDictionaryCache(void)
{
    if (verbose) puts("STUB: DCSInvalidateDictionaryCache called");
    return NULL;
}

void* DCSNormalizeSearchString(void)
{
    if (verbose) puts("STUB: DCSNormalizeSearchString called");
    return NULL;
}

void* DCSPrepareMobileAssetQuery(void)
{
    if (verbose) puts("STUB: DCSPrepareMobileAssetQuery called");
    return NULL;
}

void* DCSRecordCopyData(void)
{
    if (verbose) puts("STUB: DCSRecordCopyData called");
    return NULL;
}

void* DCSRecordCopyDataURL(void)
{
    if (verbose) puts("STUB: DCSRecordCopyDataURL called");
    return NULL;
}

void* DCSRecordGetAnchor(void)
{
    if (verbose) puts("STUB: DCSRecordGetAnchor called");
    return NULL;
}

void* DCSRecordGetAssociatedObj(void)
{
    if (verbose) puts("STUB: DCSRecordGetAssociatedObj called");
    return NULL;
}

void* DCSRecordGetDictionary(void)
{
    if (verbose) puts("STUB: DCSRecordGetDictionary called");
    return NULL;
}

void* DCSRecordGetHeadword(void)
{
    if (verbose) puts("STUB: DCSRecordGetHeadword called");
    return NULL;
}

void* DCSRecordGetRawHeadword(void)
{
    if (verbose) puts("STUB: DCSRecordGetRawHeadword called");
    return NULL;
}

void* DCSRecordGetString(void)
{
    if (verbose) puts("STUB: DCSRecordGetString called");
    return NULL;
}

void* DCSRecordGetSubDictionary(void)
{
    if (verbose) puts("STUB: DCSRecordGetSubDictionary called");
    return NULL;
}

void* DCSRecordGetTitle(void)
{
    if (verbose) puts("STUB: DCSRecordGetTitle called");
    return NULL;
}

void* DCSRecordSetAssociatedObj(void)
{
    if (verbose) puts("STUB: DCSRecordSetAssociatedObj called");
    return NULL;
}

void* DCSRecordSetHeadword(void)
{
    if (verbose) puts("STUB: DCSRecordSetHeadword called");
    return NULL;
}

void* DCSSearchSessionCreate(void)
{
    if (verbose) puts("STUB: DCSSearchSessionCreate called");
    return NULL;
}

void* DCSSearchSessionScheduleWithRunLoop(void)
{
    if (verbose) puts("STUB: DCSSearchSessionScheduleWithRunLoop called");
    return NULL;
}

void* DCSSearchSessionUnscheduleFromRunLoop(void)
{
    if (verbose) puts("STUB: DCSSearchSessionUnscheduleFromRunLoop called");
    return NULL;
}

void* DCSSetServicePresentationType(void)
{
    if (verbose) puts("STUB: DCSSetServicePresentationType called");
    return NULL;
}

void* DCSSortRecordsWithHeadword(void)
{
    if (verbose) puts("STUB: DCSSortRecordsWithHeadword called");
    return NULL;
}

CFStringRef kDCSActiveDictionariesChangedNotification = CFSTR("DCSActiveDictionariesChangedNotification");
CFStringRef kDCSDictionaryDescriptionLanguage = CFSTR("DCSDictionaryDescriptionLanguage");
CFStringRef kDCSDictionaryIndexLanguage = CFSTR("DCSDictionaryIndexLanguage");
