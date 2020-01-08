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


#ifndef _LangAnalysis_H_
#define _LangAnalysis_H_

void* DCMAddFoundRecordToIteratorAM(void);
void* DCMAddRecord(void);
void* DCMCloseDictionary(void);
void* DCMCompactDictionary(void);
void* DCMCopyParametersFromData(void);
void* DCMCountObjectIterator(void);
void* DCMCountRecord(void);
void* DCMCountRecordIterator(void);
void* DCMCreateAccessMethodIterator(void);
void* DCMCreateDataWithParameters(void);
void* DCMCreateDictionaryIterator(void);
void* DCMCreateDictionaryIteratorInternal(void);
void* DCMCreateFieldInfoRecord(void);
void* DCMCreateFieldInfoRecordAM(void);
void* DCMDeleteDictionary(void);
void* DCMDeleteRecord(void);
void* DCMDeriveNewDictionaryWithURL(void);
void* DCMDictionaryServiceWindowShow(void);
void* DCMDisposeObjectIterator(void);
void* DCMDisposeRecordIterator(void);
void* DCMFindRecords(void);
void* DCMGetAccessMethodIDFromName(void);
void* DCMGetDictionaryFieldInfo(void);
void* DCMGetDictionaryIDFromFileWithURL(void);
void* DCMGetDictionaryIDFromRef(void);
void* DCMGetDictionaryProperty(void);
void* DCMGetDictionaryPropertyList(void);
void* DCMGetDictionaryWriteAccess(void);
void* DCMGetFieldAttributes(void);
void* DCMGetFieldAttributesAM(void);
void* DCMGetFieldData(void);
void* DCMGetFieldDefaultData(void);
void* DCMGetFieldDefaultDataAM(void);
void* DCMGetFieldFindMethods(void);
void* DCMGetFieldFindMethodsAM(void);
void* DCMGetFieldMaxRecordSize(void);
void* DCMGetFieldMaxRecordSizeAM(void);
void* DCMGetFieldTagAndType(void);
void* DCMGetFieldTagAndTypeAM(void);
void* DCMGetFileFromDictionaryIDWithURL(void);
void* DCMGetNextRecord(void);
void* DCMGetNthRecord(void);
void* DCMGetPrevRecord(void);
void* DCMGetRecordSequenceNumber(void);
void* DCMIterateFoundRecord(void);
void* DCMIterateFoundRecordFast(void);
void* DCMIterateObject(void);
void* DCMLibraryVersion(void);
void* DCMNewDictionaryWithURL(void);
void* DCMOpenDictionary(void);
void* DCMRegisterDictionaryFileWithURL(void);
void* DCMReleaseDictionaryWriteAccess(void);
void* DCMReorganizeDictionary(void);
void* DCMResetObjectIterator(void);
void* DCMSetDictionaryProperty(void);
void* DCMSetFieldData(void);
void* DCMShowDictionaryServiceWindow(void);
void* DCMSubscribeDictionaryNotification(void);
void* DCMUnregisterDictionary(void);
void* DCMUnsubscribeDictionaryNotification(void);
void* LAAddNewWordWithURL(void);
void* LAChangeForm(void);
void* LACloseAnalysisContext(void);
void* LACloseContext(void);
void* LACloseDictionaryWithURL(void);
void* LAConfirmMorpheme(void);
void* LAContinuousMorphemeAnalysis(void);
void* LAContinuousMorphemeAnalysisPriv(void);
void* LACreateCustomEnvironment(void);
void* LADeleteCustomEnvironment(void);
void* LAFeedbackPath(void);
void* LAGetEnvironmentList(void);
void* LAGetEnvironmentName(void);
void* LAGetEnvironmentProperty(void);
void* LAGetEnvironmentRef(void);
void* LAGetMorphemes(void);
void* LAGetMorphemesPriv(void);
void* LALibraryVersion(void);
void* LAListAvailableDictionariesWithURL(void);
void* LAMorphemeAnalysis(void);
void* LAMorphemeAnalysisPriv(void);
void* LAOpenAnalysisContext(void);
void* LAOpenContext(void);
void* LAOpenDictionaryPriv(void);
void* LAOpenDictionaryWithURL(void);
void* LAReanalyzePartially(void);
void* LAResetAnalysis(void);
void* LASetEnvironmentProperty(void);
void* LAShiftMorphemes(void);
void* LATextToMorphemes(void);

#endif
