/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#include "DirectoryService.h"
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* dsAddAttribute(void) {
    if (verbose) puts("STUB: dsAddAttribute called");
    return NULL;
}

void* dsAddAttributeValue(void) {
    if (verbose) puts("STUB: dsAddAttributeValue called");
    return NULL;
}

void* dsAddChildPIDToReference(void) {
    if (verbose) puts("STUB: dsAddChildPIDToReference called");
    return NULL;
}

void* dsAllocAttributeValueEntry(void) {
    if (verbose) puts("STUB: dsAllocAttributeValueEntry called");
    return NULL;
}

void* dsAllocStringsFromList(void) {
    if (verbose) puts("STUB: dsAllocStringsFromList called");
    return NULL;
}

void* dsAppendAuthBuffer(void) {
    if (verbose) puts("STUB: dsAppendAuthBuffer called");
    return NULL;
}

void* dsAppendAuthBufferWithAuthorityAttribute(void) {
    if (verbose) puts("STUB: dsAppendAuthBufferWithAuthorityAttribute called");
    return NULL;
}

void* dsAppendAuthBufferWithAuthorityStrings(void) {
    if (verbose) puts("STUB: dsAppendAuthBufferWithAuthorityStrings called");
    return NULL;
}

void* dsAppendStringToList(void) {
    if (verbose) puts("STUB: dsAppendStringToList called");
    return NULL;
}

void* dsAppendStringToListAlloc(void) {
    if (verbose) puts("STUB: dsAppendStringToListAlloc called");
    return NULL;
}

void* dsAuthBufferGetDataListAllocPriv(void) {
    if (verbose) puts("STUB: dsAuthBufferGetDataListAllocPriv called");
    return NULL;
}

void* dsAuthBufferGetDataListPriv(void) {
    if (verbose) puts("STUB: dsAuthBufferGetDataListPriv called");
    return NULL;
}

void* dsBuildFromPath(void) {
    if (verbose) puts("STUB: dsBuildFromPath called");
    return NULL;
}

void* dsBuildListFromNodes(void) {
    if (verbose) puts("STUB: dsBuildListFromNodes called");
    return NULL;
}

void* dsBuildListFromNodesAlloc(void) {
    if (verbose) puts("STUB: dsBuildListFromNodesAlloc called");
    return NULL;
}

void* dsBuildListFromPathAlloc(void) {
    if (verbose) puts("STUB: dsBuildListFromPathAlloc called");
    return NULL;
}

void* dsBuildListFromStrings(void) {
    if (verbose) puts("STUB: dsBuildListFromStrings called");
    return NULL;
}

void* dsBuildListFromStringsAlloc(void) {
    if (verbose) puts("STUB: dsBuildListFromStringsAlloc called");
    return NULL;
}

void* dsBuildListFromStringsAllocV(void) {
    if (verbose) puts("STUB: dsBuildListFromStringsAllocV called");
    return NULL;
}

void* dsCloseAttributeList(void) {
    if (verbose) puts("STUB: dsCloseAttributeList called");
    return NULL;
}

void* dsCloseAttributeValueList(void) {
    if (verbose) puts("STUB: dsCloseAttributeValueList called");
    return NULL;
}

void* dsCloseDirNode(void) {
    if (verbose) puts("STUB: dsCloseDirNode called");
    return NULL;
}

void* dsCloseDirService(void) {
    if (verbose) puts("STUB: dsCloseDirService called");
    return NULL;
}

void* dsCloseRecord(void) {
    if (verbose) puts("STUB: dsCloseRecord called");
    return NULL;
}

void* dsConvertAuthAuthorityToCFDict(void) {
    if (verbose) puts("STUB: dsConvertAuthAuthorityToCFDict called");
    return NULL;
}

void* dsConvertCFDictToAuthAuthority(void) {
    if (verbose) puts("STUB: dsConvertCFDictToAuthAuthority called");
    return NULL;
}

void* dsCopyDataFromDirRef(void) {
    if (verbose) puts("STUB: dsCopyDataFromDirRef called");
    return NULL;
}

void* dsCopyDataFromNodeRef(void) {
    if (verbose) puts("STUB: dsCopyDataFromNodeRef called");
    return NULL;
}

void* dsCopyDirStatusName(void) {
    if (verbose) puts("STUB: dsCopyDirStatusName called");
    return NULL;
}

void* dsCreateDataDirRefData(void) {
    if (verbose) puts("STUB: dsCreateDataDirRefData called");
    return NULL;
}

void* dsCreateNodeRefData(void) {
    if (verbose) puts("STUB: dsCreateNodeRefData called");
    return NULL;
}

void* dsCreateRecord(void) {
    if (verbose) puts("STUB: dsCreateRecord called");
    return NULL;
}

void* dsCreateRecordAndOpen(void) {
    if (verbose) puts("STUB: dsCreateRecordAndOpen called");
    return NULL;
}

void* dsDataBufferAllocate(void) {
    if (verbose) puts("STUB: dsDataBufferAllocate called");
    return NULL;
}

void* dsDataBufferDeAllocate(void) {
    if (verbose) puts("STUB: dsDataBufferDeAllocate called");
    return NULL;
}

void* dsDataListAllocate(void) {
    if (verbose) puts("STUB: dsDataListAllocate called");
    return NULL;
}

void* dsDataListCopyList(void) {
    if (verbose) puts("STUB: dsDataListCopyList called");
    return NULL;
}

void* dsDataListDeAllocate(void) {
    if (verbose) puts("STUB: dsDataListDeAllocate called");
    return NULL;
}

void* dsDataListDeallocate(void) {
    if (verbose) puts("STUB: dsDataListDeallocate called");
    return NULL;
}

void* dsDataListDeleteThisNode(void) {
    if (verbose) puts("STUB: dsDataListDeleteThisNode called");
    return NULL;
}

void* dsDataListGetNode(void) {
    if (verbose) puts("STUB: dsDataListGetNode called");
    return NULL;
}

void* dsDataListGetNodeAlloc(void) {
    if (verbose) puts("STUB: dsDataListGetNodeAlloc called");
    return NULL;
}

void* dsDataListGetNodeCount(void) {
    if (verbose) puts("STUB: dsDataListGetNodeCount called");
    return NULL;
}

void* dsDataListGetNodeStringPriv(void) {
    if (verbose) puts("STUB: dsDataListGetNodeStringPriv called");
    return NULL;
}

void* dsDataListInsertAfter(void) {
    if (verbose) puts("STUB: dsDataListInsertAfter called");
    return NULL;
}

void* dsDataListInsertNode(void) {
    if (verbose) puts("STUB: dsDataListInsertNode called");
    return NULL;
}

void* dsDataListMergeList(void) {
    if (verbose) puts("STUB: dsDataListMergeList called");
    return NULL;
}

void* dsDataListMergeListAfter(void) {
    if (verbose) puts("STUB: dsDataListMergeListAfter called");
    return NULL;
}

void* dsDataListRemoveNodes(void) {
    if (verbose) puts("STUB: dsDataListRemoveNodes called");
    return NULL;
}

void* dsDataListRemoveThisNode(void) {
    if (verbose) puts("STUB: dsDataListRemoveThisNode called");
    return NULL;
}

void* dsDataNodeAllocateBlock(void) {
    if (verbose) puts("STUB: dsDataNodeAllocateBlock called");
    return NULL;
}

void* dsDataNodeAllocateString(void) {
    if (verbose) puts("STUB: dsDataNodeAllocateString called");
    return NULL;
}

void* dsDataNodeDeAllocate(void) {
    if (verbose) puts("STUB: dsDataNodeDeAllocate called");
    return NULL;
}

void* dsDataNodeGetLength(void) {
    if (verbose) puts("STUB: dsDataNodeGetLength called");
    return NULL;
}

void* dsDataNodeGetSize(void) {
    if (verbose) puts("STUB: dsDataNodeGetSize called");
    return NULL;
}

void* dsDataNodeSetLength(void) {
    if (verbose) puts("STUB: dsDataNodeSetLength called");
    return NULL;
}

void* dsDeallocAttributeEntry(void) {
    if (verbose) puts("STUB: dsDeallocAttributeEntry called");
    return NULL;
}

void* dsDeallocAttributeValueEntry(void) {
    if (verbose) puts("STUB: dsDeallocAttributeValueEntry called");
    return NULL;
}

void* dsDeallocRecordEntry(void) {
    if (verbose) puts("STUB: dsDeallocRecordEntry called");
    return NULL;
}

void* dsDeleteRecord(void) {
    if (verbose) puts("STUB: dsDeleteRecord called");
    return NULL;
}

void* dsDoAttributeValueSearch(void) {
    if (verbose) puts("STUB: dsDoAttributeValueSearch called");
    return NULL;
}

void* dsDoAttributeValueSearchWithData(void) {
    if (verbose) puts("STUB: dsDoAttributeValueSearchWithData called");
    return NULL;
}

void* dsDoDirNodeAuth(void) {
    if (verbose) puts("STUB: dsDoDirNodeAuth called");
    return NULL;
}

void* dsDoDirNodeAuthOnRecordType(void) {
    if (verbose) puts("STUB: dsDoDirNodeAuthOnRecordType called");
    return NULL;
}

void* dsDoMultipleAttributeValueSearch(void) {
    if (verbose) puts("STUB: dsDoMultipleAttributeValueSearch called");
    return NULL;
}

void* dsDoMultipleAttributeValueSearchWithData(void) {
    if (verbose) puts("STUB: dsDoMultipleAttributeValueSearchWithData called");
    return NULL;
}

void* dsDoPlugInCustomCall(void) {
    if (verbose) puts("STUB: dsDoPlugInCustomCall called");
    return NULL;
}

void* dsFillAuthBuffer(void) {
    if (verbose) puts("STUB: dsFillAuthBuffer called");
    return NULL;
}

void* dsFindDirNodes(void) {
    if (verbose) puts("STUB: dsFindDirNodes called");
    return NULL;
}

void* dsFlushRecord(void) {
    if (verbose) puts("STUB: dsFlushRecord called");
    return NULL;
}

void* dsGetAttributeEntry(void) {
    if (verbose) puts("STUB: dsGetAttributeEntry called");
    return NULL;
}

void* dsGetAttributeValue(void) {
    if (verbose) puts("STUB: dsGetAttributeValue called");
    return NULL;
}

void* dsGetCustomAllocate(void) {
    if (verbose) puts("STUB: dsGetCustomAllocate called");
    return NULL;
}

void* dsGetCustomThread(void) {
    if (verbose) puts("STUB: dsGetCustomThread called");
    return NULL;
}

void* dsGetDataLength(void) {
    if (verbose) puts("STUB: dsGetDataLength called");
    return NULL;
}

void* dsGetDirNodeCount(void) {
    if (verbose) puts("STUB: dsGetDirNodeCount called");
    return NULL;
}

void* dsGetDirNodeCountWithInfo(void) {
    if (verbose) puts("STUB: dsGetDirNodeCountWithInfo called");
    return NULL;
}

void* dsGetDirNodeInfo(void) {
    if (verbose) puts("STUB: dsGetDirNodeInfo called");
    return NULL;
}

void* dsGetDirNodeList(void) {
    if (verbose) puts("STUB: dsGetDirNodeList called");
    return NULL;
}

void* dsGetDirNodeName(void) {
    if (verbose) puts("STUB: dsGetDirNodeName called");
    return NULL;
}

void* dsGetNextAttributeEntry(void) {
    if (verbose) puts("STUB: dsGetNextAttributeEntry called");
    return NULL;
}

void* dsGetNextAttributeValue(void) {
    if (verbose) puts("STUB: dsGetNextAttributeValue called");
    return NULL;
}

void* dsGetPathFromList(void) {
    if (verbose) puts("STUB: dsGetPathFromList called");
    return NULL;
}

void* dsGetRecordAttributeInfo(void) {
    if (verbose) puts("STUB: dsGetRecordAttributeInfo called");
    return NULL;
}

void* dsGetRecordAttributeValueByID(void) {
    if (verbose) puts("STUB: dsGetRecordAttributeValueByID called");
    return NULL;
}

void* dsGetRecordAttributeValueByIndex(void) {
    if (verbose) puts("STUB: dsGetRecordAttributeValueByIndex called");
    return NULL;
}

void* dsGetRecordAttributeValueByValue(void) {
    if (verbose) puts("STUB: dsGetRecordAttributeValueByValue called");
    return NULL;
}

void* dsGetRecordEntry(void) {
    if (verbose) puts("STUB: dsGetRecordEntry called");
    return NULL;
}

void* dsGetRecordList(void) {
    if (verbose) puts("STUB: dsGetRecordList called");
    return NULL;
}

void* dsGetRecordNameFromEntry(void) {
    if (verbose) puts("STUB: dsGetRecordNameFromEntry called");
    return NULL;
}

void* dsGetRecordReferenceInfo(void) {
    if (verbose) puts("STUB: dsGetRecordReferenceInfo called");
    return NULL;
}

void* dsGetRecordTypeFromEntry(void) {
    if (verbose) puts("STUB: dsGetRecordTypeFromEntry called");
    return NULL;
}

void* dsOpenDirNode(void) {
    if (verbose) puts("STUB: dsOpenDirNode called");
    return NULL;
}

void* dsOpenDirService(void) {
    if (verbose) puts("STUB: dsOpenDirService called");
    return NULL;
}

void* dsOpenDirServiceLocal(void) {
    if (verbose) puts("STUB: dsOpenDirServiceLocal called");
    return NULL;
}

void* dsOpenDirServiceProxy(void) {
    if (verbose) puts("STUB: dsOpenDirServiceProxy called");
    return NULL;
}

void* dsOpenRecord(void) {
    if (verbose) puts("STUB: dsOpenRecord called");
    return NULL;
}

void* dsParseAuthAuthority(void) {
    if (verbose) puts("STUB: dsParseAuthAuthority called");
    return NULL;
}

void* dsParseAuthAuthorityExtended(void) {
    if (verbose) puts("STUB: dsParseAuthAuthorityExtended called");
    return NULL;
}

void* dsRegisterCustomMemory(void) {
    if (verbose) puts("STUB: dsRegisterCustomMemory called");
    return NULL;
}

void* dsRegisterCustomThread(void) {
    if (verbose) puts("STUB: dsRegisterCustomThread called");
    return NULL;
}

void* dsReleaseContinueData(void) {
    if (verbose) puts("STUB: dsReleaseContinueData called");
    return NULL;
}

void* dsRemoveAttribute(void) {
    if (verbose) puts("STUB: dsRemoveAttribute called");
    return NULL;
}

void* dsRemoveAttributeValue(void) {
    if (verbose) puts("STUB: dsRemoveAttributeValue called");
    return NULL;
}

void* dsServiceInformationAllocate(void) {
    if (verbose) puts("STUB: dsServiceInformationAllocate called");
    return NULL;
}

void* dsSetAttributeValue(void) {
    if (verbose) puts("STUB: dsSetAttributeValue called");
    return NULL;
}

void* dsSetAttributeValues(void) {
    if (verbose) puts("STUB: dsSetAttributeValues called");
    return NULL;
}

void* dsSetRecordName(void) {
    if (verbose) puts("STUB: dsSetRecordName called");
    return NULL;
}

void* dsSetRecordType(void) {
    if (verbose) puts("STUB: dsSetRecordType called");
    return NULL;
}

void* dsUnRegisterCustomMemory(void) {
    if (verbose) puts("STUB: dsUnRegisterCustomMemory called");
    return NULL;
}

void* dsUnRegisterCustomThread(void) {
    if (verbose) puts("STUB: dsUnRegisterCustomThread called");
    return NULL;
}

void* dsVerifyDirRefNum(void) {
    if (verbose) puts("STUB: dsVerifyDirRefNum called");
    return NULL;
}
