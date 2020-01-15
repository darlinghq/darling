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


#include <LangAnalysis/LangAnalysis.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DCMAddFoundRecordToIteratorAM(void)
{
    if (verbose) puts("STUB: DCMAddFoundRecordToIteratorAM called");
    return NULL;
}

void* DCMAddRecord(void)
{
    if (verbose) puts("STUB: DCMAddRecord called");
    return NULL;
}

void* DCMCloseDictionary(void)
{
    if (verbose) puts("STUB: DCMCloseDictionary called");
    return NULL;
}

void* DCMCompactDictionary(void)
{
    if (verbose) puts("STUB: DCMCompactDictionary called");
    return NULL;
}

void* DCMCopyParametersFromData(void)
{
    if (verbose) puts("STUB: DCMCopyParametersFromData called");
    return NULL;
}

void* DCMCountObjectIterator(void)
{
    if (verbose) puts("STUB: DCMCountObjectIterator called");
    return NULL;
}

void* DCMCountRecord(void)
{
    if (verbose) puts("STUB: DCMCountRecord called");
    return NULL;
}

void* DCMCountRecordIterator(void)
{
    if (verbose) puts("STUB: DCMCountRecordIterator called");
    return NULL;
}

void* DCMCreateAccessMethodIterator(void)
{
    if (verbose) puts("STUB: DCMCreateAccessMethodIterator called");
    return NULL;
}

void* DCMCreateDataWithParameters(void)
{
    if (verbose) puts("STUB: DCMCreateDataWithParameters called");
    return NULL;
}

void* DCMCreateDictionaryIterator(void)
{
    if (verbose) puts("STUB: DCMCreateDictionaryIterator called");
    return NULL;
}

void* DCMCreateDictionaryIteratorInternal(void)
{
    if (verbose) puts("STUB: DCMCreateDictionaryIteratorInternal called");
    return NULL;
}

void* DCMCreateFieldInfoRecord(void)
{
    if (verbose) puts("STUB: DCMCreateFieldInfoRecord called");
    return NULL;
}

void* DCMCreateFieldInfoRecordAM(void)
{
    if (verbose) puts("STUB: DCMCreateFieldInfoRecordAM called");
    return NULL;
}

void* DCMDeleteDictionary(void)
{
    if (verbose) puts("STUB: DCMDeleteDictionary called");
    return NULL;
}

void* DCMDeleteRecord(void)
{
    if (verbose) puts("STUB: DCMDeleteRecord called");
    return NULL;
}

void* DCMDeriveNewDictionaryWithURL(void)
{
    if (verbose) puts("STUB: DCMDeriveNewDictionaryWithURL called");
    return NULL;
}

void* DCMDictionaryServiceWindowShow(void)
{
    if (verbose) puts("STUB: DCMDictionaryServiceWindowShow called");
    return NULL;
}

void* DCMDisposeObjectIterator(void)
{
    if (verbose) puts("STUB: DCMDisposeObjectIterator called");
    return NULL;
}

void* DCMDisposeRecordIterator(void)
{
    if (verbose) puts("STUB: DCMDisposeRecordIterator called");
    return NULL;
}

void* DCMFindRecords(void)
{
    if (verbose) puts("STUB: DCMFindRecords called");
    return NULL;
}

void* DCMGetAccessMethodIDFromName(void)
{
    if (verbose) puts("STUB: DCMGetAccessMethodIDFromName called");
    return NULL;
}

void* DCMGetDictionaryFieldInfo(void)
{
    if (verbose) puts("STUB: DCMGetDictionaryFieldInfo called");
    return NULL;
}

void* DCMGetDictionaryIDFromFileWithURL(void)
{
    if (verbose) puts("STUB: DCMGetDictionaryIDFromFileWithURL called");
    return NULL;
}

void* DCMGetDictionaryIDFromRef(void)
{
    if (verbose) puts("STUB: DCMGetDictionaryIDFromRef called");
    return NULL;
}

void* DCMGetDictionaryProperty(void)
{
    if (verbose) puts("STUB: DCMGetDictionaryProperty called");
    return NULL;
}

void* DCMGetDictionaryPropertyList(void)
{
    if (verbose) puts("STUB: DCMGetDictionaryPropertyList called");
    return NULL;
}

void* DCMGetDictionaryWriteAccess(void)
{
    if (verbose) puts("STUB: DCMGetDictionaryWriteAccess called");
    return NULL;
}

void* DCMGetFieldAttributes(void)
{
    if (verbose) puts("STUB: DCMGetFieldAttributes called");
    return NULL;
}

void* DCMGetFieldAttributesAM(void)
{
    if (verbose) puts("STUB: DCMGetFieldAttributesAM called");
    return NULL;
}

void* DCMGetFieldData(void)
{
    if (verbose) puts("STUB: DCMGetFieldData called");
    return NULL;
}

void* DCMGetFieldDefaultData(void)
{
    if (verbose) puts("STUB: DCMGetFieldDefaultData called");
    return NULL;
}

void* DCMGetFieldDefaultDataAM(void)
{
    if (verbose) puts("STUB: DCMGetFieldDefaultDataAM called");
    return NULL;
}

void* DCMGetFieldFindMethods(void)
{
    if (verbose) puts("STUB: DCMGetFieldFindMethods called");
    return NULL;
}

void* DCMGetFieldFindMethodsAM(void)
{
    if (verbose) puts("STUB: DCMGetFieldFindMethodsAM called");
    return NULL;
}

void* DCMGetFieldMaxRecordSize(void)
{
    if (verbose) puts("STUB: DCMGetFieldMaxRecordSize called");
    return NULL;
}

void* DCMGetFieldMaxRecordSizeAM(void)
{
    if (verbose) puts("STUB: DCMGetFieldMaxRecordSizeAM called");
    return NULL;
}

void* DCMGetFieldTagAndType(void)
{
    if (verbose) puts("STUB: DCMGetFieldTagAndType called");
    return NULL;
}

void* DCMGetFieldTagAndTypeAM(void)
{
    if (verbose) puts("STUB: DCMGetFieldTagAndTypeAM called");
    return NULL;
}

void* DCMGetFileFromDictionaryIDWithURL(void)
{
    if (verbose) puts("STUB: DCMGetFileFromDictionaryIDWithURL called");
    return NULL;
}

void* DCMGetNextRecord(void)
{
    if (verbose) puts("STUB: DCMGetNextRecord called");
    return NULL;
}

void* DCMGetNthRecord(void)
{
    if (verbose) puts("STUB: DCMGetNthRecord called");
    return NULL;
}

void* DCMGetPrevRecord(void)
{
    if (verbose) puts("STUB: DCMGetPrevRecord called");
    return NULL;
}

void* DCMGetRecordSequenceNumber(void)
{
    if (verbose) puts("STUB: DCMGetRecordSequenceNumber called");
    return NULL;
}

void* DCMIterateFoundRecord(void)
{
    if (verbose) puts("STUB: DCMIterateFoundRecord called");
    return NULL;
}

void* DCMIterateFoundRecordFast(void)
{
    if (verbose) puts("STUB: DCMIterateFoundRecordFast called");
    return NULL;
}

void* DCMIterateObject(void)
{
    if (verbose) puts("STUB: DCMIterateObject called");
    return NULL;
}

void* DCMLibraryVersion(void)
{
    if (verbose) puts("STUB: DCMLibraryVersion called");
    return NULL;
}

void* DCMNewDictionaryWithURL(void)
{
    if (verbose) puts("STUB: DCMNewDictionaryWithURL called");
    return NULL;
}

void* DCMOpenDictionary(void)
{
    if (verbose) puts("STUB: DCMOpenDictionary called");
    return NULL;
}

void* DCMRegisterDictionaryFileWithURL(void)
{
    if (verbose) puts("STUB: DCMRegisterDictionaryFileWithURL called");
    return NULL;
}

void* DCMReleaseDictionaryWriteAccess(void)
{
    if (verbose) puts("STUB: DCMReleaseDictionaryWriteAccess called");
    return NULL;
}

void* DCMReorganizeDictionary(void)
{
    if (verbose) puts("STUB: DCMReorganizeDictionary called");
    return NULL;
}

void* DCMResetObjectIterator(void)
{
    if (verbose) puts("STUB: DCMResetObjectIterator called");
    return NULL;
}

void* DCMSetDictionaryProperty(void)
{
    if (verbose) puts("STUB: DCMSetDictionaryProperty called");
    return NULL;
}

void* DCMSetFieldData(void)
{
    if (verbose) puts("STUB: DCMSetFieldData called");
    return NULL;
}

void* DCMShowDictionaryServiceWindow(void)
{
    if (verbose) puts("STUB: DCMShowDictionaryServiceWindow called");
    return NULL;
}

void* DCMSubscribeDictionaryNotification(void)
{
    if (verbose) puts("STUB: DCMSubscribeDictionaryNotification called");
    return NULL;
}

void* DCMUnregisterDictionary(void)
{
    if (verbose) puts("STUB: DCMUnregisterDictionary called");
    return NULL;
}

void* DCMUnsubscribeDictionaryNotification(void)
{
    if (verbose) puts("STUB: DCMUnsubscribeDictionaryNotification called");
    return NULL;
}

void* LAAddNewWordWithURL(void)
{
    if (verbose) puts("STUB: LAAddNewWordWithURL called");
    return NULL;
}

void* LAChangeForm(void)
{
    if (verbose) puts("STUB: LAChangeForm called");
    return NULL;
}

void* LACloseAnalysisContext(void)
{
    if (verbose) puts("STUB: LACloseAnalysisContext called");
    return NULL;
}

void* LACloseContext(void)
{
    if (verbose) puts("STUB: LACloseContext called");
    return NULL;
}

void* LACloseDictionaryWithURL(void)
{
    if (verbose) puts("STUB: LACloseDictionaryWithURL called");
    return NULL;
}

void* LAConfirmMorpheme(void)
{
    if (verbose) puts("STUB: LAConfirmMorpheme called");
    return NULL;
}

void* LAContinuousMorphemeAnalysis(void)
{
    if (verbose) puts("STUB: LAContinuousMorphemeAnalysis called");
    return NULL;
}

void* LAContinuousMorphemeAnalysisPriv(void)
{
    if (verbose) puts("STUB: LAContinuousMorphemeAnalysisPriv called");
    return NULL;
}

void* LACreateCustomEnvironment(void)
{
    if (verbose) puts("STUB: LACreateCustomEnvironment called");
    return NULL;
}

void* LADeleteCustomEnvironment(void)
{
    if (verbose) puts("STUB: LADeleteCustomEnvironment called");
    return NULL;
}

void* LAFeedbackPath(void)
{
    if (verbose) puts("STUB: LAFeedbackPath called");
    return NULL;
}

void* LAGetEnvironmentList(void)
{
    if (verbose) puts("STUB: LAGetEnvironmentList called");
    return NULL;
}

void* LAGetEnvironmentName(void)
{
    if (verbose) puts("STUB: LAGetEnvironmentName called");
    return NULL;
}

void* LAGetEnvironmentProperty(void)
{
    if (verbose) puts("STUB: LAGetEnvironmentProperty called");
    return NULL;
}

void* LAGetEnvironmentRef(void)
{
    if (verbose) puts("STUB: LAGetEnvironmentRef called");
    return NULL;
}

void* LAGetMorphemes(void)
{
    if (verbose) puts("STUB: LAGetMorphemes called");
    return NULL;
}

void* LAGetMorphemesPriv(void)
{
    if (verbose) puts("STUB: LAGetMorphemesPriv called");
    return NULL;
}

void* LALibraryVersion(void)
{
    if (verbose) puts("STUB: LALibraryVersion called");
    return NULL;
}

void* LAListAvailableDictionariesWithURL(void)
{
    if (verbose) puts("STUB: LAListAvailableDictionariesWithURL called");
    return NULL;
}

void* LAMorphemeAnalysis(void)
{
    if (verbose) puts("STUB: LAMorphemeAnalysis called");
    return NULL;
}

void* LAMorphemeAnalysisPriv(void)
{
    if (verbose) puts("STUB: LAMorphemeAnalysisPriv called");
    return NULL;
}

void* LAOpenAnalysisContext(void)
{
    if (verbose) puts("STUB: LAOpenAnalysisContext called");
    return NULL;
}

void* LAOpenContext(void)
{
    if (verbose) puts("STUB: LAOpenContext called");
    return NULL;
}

void* LAOpenDictionaryPriv(void)
{
    if (verbose) puts("STUB: LAOpenDictionaryPriv called");
    return NULL;
}

void* LAOpenDictionaryWithURL(void)
{
    if (verbose) puts("STUB: LAOpenDictionaryWithURL called");
    return NULL;
}

void* LAReanalyzePartially(void)
{
    if (verbose) puts("STUB: LAReanalyzePartially called");
    return NULL;
}

void* LAResetAnalysis(void)
{
    if (verbose) puts("STUB: LAResetAnalysis called");
    return NULL;
}

void* LASetEnvironmentProperty(void)
{
    if (verbose) puts("STUB: LASetEnvironmentProperty called");
    return NULL;
}

void* LAShiftMorphemes(void)
{
    if (verbose) puts("STUB: LAShiftMorphemes called");
    return NULL;
}

void* LATextToMorphemes(void)
{
    if (verbose) puts("STUB: LATextToMorphemes called");
    return NULL;
}
