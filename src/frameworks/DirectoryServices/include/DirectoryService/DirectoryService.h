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


#ifndef _DirectoryService_H_
#define _DirectoryService_H_

void* dsAddAttribute(void);
void* dsAddAttributeValue(void);
void* dsAddChildPIDToReference(void);
void* dsAllocAttributeValueEntry(void);
void* dsAllocStringsFromList(void);
void* dsAppendAuthBuffer(void);
void* dsAppendAuthBufferWithAuthorityAttribute(void);
void* dsAppendAuthBufferWithAuthorityStrings(void);
void* dsAppendStringToList(void);
void* dsAppendStringToListAlloc(void);
void* dsAuthBufferGetDataListAllocPriv(void);
void* dsAuthBufferGetDataListPriv(void);
void* dsBuildFromPath(void);
void* dsBuildListFromNodes(void);
void* dsBuildListFromNodesAlloc(void);
void* dsBuildListFromPathAlloc(void);
void* dsBuildListFromStrings(void);
void* dsBuildListFromStringsAlloc(void);
void* dsBuildListFromStringsAllocV(void);
void* dsCloseAttributeList(void);
void* dsCloseAttributeValueList(void);
void* dsCloseDirNode(void);
void* dsCloseDirService(void);
void* dsCloseRecord(void);
void* dsConvertAuthAuthorityToCFDict(void);
void* dsConvertCFDictToAuthAuthority(void);
void* dsCopyDataFromDirRef(void);
void* dsCopyDataFromNodeRef(void);
void* dsCopyDirStatusName(void);
void* dsCreateDataDirRefData(void);
void* dsCreateNodeRefData(void);
void* dsCreateRecord(void);
void* dsCreateRecordAndOpen(void);
void* dsDataBufferAllocate(void);
void* dsDataBufferDeAllocate(void);
void* dsDataListAllocate(void);
void* dsDataListCopyList(void);
void* dsDataListDeAllocate(void);
void* dsDataListDeallocate(void);
void* dsDataListDeleteThisNode(void);
void* dsDataListGetNode(void);
void* dsDataListGetNodeAlloc(void);
void* dsDataListGetNodeCount(void);
void* dsDataListGetNodeStringPriv(void);
void* dsDataListInsertAfter(void);
void* dsDataListInsertNode(void);
void* dsDataListMergeList(void);
void* dsDataListMergeListAfter(void);
void* dsDataListRemoveNodes(void);
void* dsDataListRemoveThisNode(void);
void* dsDataNodeAllocateBlock(void);
void* dsDataNodeAllocateString(void);
void* dsDataNodeDeAllocate(void);
void* dsDataNodeGetLength(void);
void* dsDataNodeGetSize(void);
void* dsDataNodeSetLength(void);
void* dsDeallocAttributeEntry(void);
void* dsDeallocAttributeValueEntry(void);
void* dsDeallocRecordEntry(void);
void* dsDeleteRecord(void);
void* dsDoAttributeValueSearch(void);
void* dsDoAttributeValueSearchWithData(void);
void* dsDoDirNodeAuth(void);
void* dsDoDirNodeAuthOnRecordType(void);
void* dsDoMultipleAttributeValueSearch(void);
void* dsDoMultipleAttributeValueSearchWithData(void);
void* dsDoPlugInCustomCall(void);
void* dsFillAuthBuffer(void);
void* dsFindDirNodes(void);
void* dsFlushRecord(void);
void* dsGetAttributeEntry(void);
void* dsGetAttributeValue(void);
void* dsGetCustomAllocate(void);
void* dsGetCustomThread(void);
void* dsGetDataLength(void);
void* dsGetDirNodeCount(void);
void* dsGetDirNodeCountWithInfo(void);
void* dsGetDirNodeInfo(void);
void* dsGetDirNodeList(void);
void* dsGetDirNodeName(void);
void* dsGetNextAttributeEntry(void);
void* dsGetNextAttributeValue(void);
void* dsGetPathFromList(void);
void* dsGetRecordAttributeInfo(void);
void* dsGetRecordAttributeValueByID(void);
void* dsGetRecordAttributeValueByIndex(void);
void* dsGetRecordAttributeValueByValue(void);
void* dsGetRecordEntry(void);
void* dsGetRecordList(void);
void* dsGetRecordNameFromEntry(void);
void* dsGetRecordReferenceInfo(void);
void* dsGetRecordTypeFromEntry(void);
void* dsOpenDirNode(void);
void* dsOpenDirService(void);
void* dsOpenDirServiceLocal(void);
void* dsOpenDirServiceProxy(void);
void* dsOpenRecord(void);
void* dsParseAuthAuthority(void);
void* dsParseAuthAuthorityExtended(void);
void* dsRegisterCustomMemory(void);
void* dsRegisterCustomThread(void);
void* dsReleaseContinueData(void);
void* dsRemoveAttribute(void);
void* dsRemoveAttributeValue(void);
void* dsServiceInformationAllocate(void);
void* dsSetAttributeValue(void);
void* dsSetAttributeValues(void);
void* dsSetRecordName(void);
void* dsSetRecordType(void);
void* dsUnRegisterCustomMemory(void);
void* dsUnRegisterCustomThread(void);
void* dsVerifyDirRefNum(void);

#endif
