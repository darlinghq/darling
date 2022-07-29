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


#include <Bom/Bom.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* BOMAppleDoubleADPathToPath(void)
{
    if (verbose) puts("STUB: BOMAppleDoubleADPathToPath called");
    return NULL;
}

void* BOMAppleDoubleCopyHeader(void)
{
    if (verbose) puts("STUB: BOMAppleDoubleCopyHeader called");
    return NULL;
}

void* BOMAppleDoubleIsADFile(void)
{
    if (verbose) puts("STUB: BOMAppleDoubleIsADFile called");
    return NULL;
}

void* BOMAppleDoublePathToADPath(void)
{
    if (verbose) puts("STUB: BOMAppleDoublePathToADPath called");
    return NULL;
}

void* BOMAppleDoubleReadHeader(void)
{
    if (verbose) puts("STUB: BOMAppleDoubleReadHeader called");
    return NULL;
}

void* BOMAppleDoubleSize(void)
{
    if (verbose) puts("STUB: BOMAppleDoubleSize called");
    return NULL;
}

void* BOMAppleDoubleWriteHeader(void)
{
    if (verbose) puts("STUB: BOMAppleDoubleWriteHeader called");
    return NULL;
}

void* BOMArchFlagForFile(void)
{
    if (verbose) puts("STUB: BOMArchFlagForFile called");
    return NULL;
}

void* BOMArchFlagForFileWithSys(void)
{
    if (verbose) puts("STUB: BOMArchFlagForFileWithSys called");
    return NULL;
}

void* BOMArchFlagForHeader(void)
{
    if (verbose) puts("STUB: BOMArchFlagForHeader called");
    return NULL;
}

void* BOMBomApproximateBytesRepresented(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresented called");
    return NULL;
}

void* BOMBomApproximateBytesRepresented64(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresented64 called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedByVariant(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedByVariant called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedByVariant64(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedByVariant64 called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedByVariantIsValid(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedByVariantIsValid called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedByVariantWithBlockSize(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedByVariantWithBlockSize called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedByVariantWithBlockSize64(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedByVariantWithBlockSize64 called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedWithBlockSize(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedWithBlockSize called");
    return NULL;
}

void* BOMBomApproximateBytesRepresentedWithBlockSize64(void)
{
    if (verbose) puts("STUB: BOMBomApproximateBytesRepresentedWithBlockSize64 called");
    return NULL;
}

void* BOMBomCommit(void)
{
    if (verbose) puts("STUB: BOMBomCommit called");
    return NULL;
}

void* BOMBomDumpLinks(void)
{
    if (verbose) puts("STUB: BOMBomDumpLinks called");
    return NULL;
}

void* BOMBomEInsertFSObject(void)
{
    if (verbose) puts("STUB: BOMBomEInsertFSObject called");
    return NULL;
}

void* BOMBomEInvalidateCachedData(void)
{
    if (verbose) puts("STUB: BOMBomEInvalidateCachedData called");
    return NULL;
}

void* BOMBomELinkFromPathToPath(void)
{
    if (verbose) puts("STUB: BOMBomELinkFromPathToPath called");
    return NULL;
}

void* BOMBomEnumeratorFree(void)
{
    if (verbose) puts("STUB: BOMBomEnumeratorFree called");
    return NULL;
}

void* BOMBomEnumeratorNew(void)
{
    if (verbose) puts("STUB: BOMBomEnumeratorNew called");
    return NULL;
}

void* BOMBomEnumeratorNewWithOptions(void)
{
    if (verbose) puts("STUB: BOMBomEnumeratorNewWithOptions called");
    return NULL;
}

void* BOMBomEnumeratorNext(void)
{
    if (verbose) puts("STUB: BOMBomEnumeratorNext called");
    return NULL;
}

void* BOMBomEnumeratorSkip(void)
{
    if (verbose) puts("STUB: BOMBomEnumeratorSkip called");
    return NULL;
}

void* BOMBomEnumeratorSkipAll(void)
{
    if (verbose) puts("STUB: BOMBomEnumeratorSkipAll called");
    return NULL;
}

void* BOMBomFSObjectCount(void)
{
    if (verbose) puts("STUB: BOMBomFSObjectCount called");
    return NULL;
}

void* BOMBomFSObjectExistsAtPath(void)
{
    if (verbose) puts("STUB: BOMBomFSObjectExistsAtPath called");
    return NULL;
}

void* BOMBomFree(void)
{
    if (verbose) puts("STUB: BOMBomFree called");
    return NULL;
}

void* BOMBomGetFSObjectAtPath(void)
{
    if (verbose) puts("STUB: BOMBomGetFSObjectAtPath called");
    return NULL;
}

void* BOMBomGetRootFSObject(void)
{
    if (verbose) puts("STUB: BOMBomGetRootFSObject called");
    return NULL;
}

void* BOMBomInsertFSObject(void)
{
    if (verbose) puts("STUB: BOMBomInsertFSObject called");
    return NULL;
}

void* BOMBomLinkToFSObjectFromPath(void)
{
    if (verbose) puts("STUB: BOMBomLinkToFSObjectFromPath called");
    return NULL;
}

void* BOMBomLinkToPathFromPath(void)
{
    if (verbose) puts("STUB: BOMBomLinkToPathFromPath called");
    return NULL;
}

void* BOMBomMaskWithBom(void)
{
    if (verbose) puts("STUB: BOMBomMaskWithBom called");
    return NULL;
}

void* BOMBomMaskWithPatternList(void)
{
    if (verbose) puts("STUB: BOMBomMaskWithPatternList called");
    return NULL;
}

void* BOMBomMergeInto(void)
{
    if (verbose) puts("STUB: BOMBomMergeInto called");
    return NULL;
}

void* BOMBomMergeIntoBom(void)
{
    if (verbose) puts("STUB: BOMBomMergeIntoBom called");
    return NULL;
}

void* BOMBomMergeIntoBomWithPatternList(void)
{
    if (verbose) puts("STUB: BOMBomMergeIntoBomWithPatternList called");
    return NULL;
}

void* BOMBomNew(void)
{
    if (verbose) puts("STUB: BOMBomNew called");
    return NULL;
}

void* BOMBomNewFromBom(void)
{
    if (verbose) puts("STUB: BOMBomNewFromBom called");
    return NULL;
}

void* BOMBomNewFromBomWithOptions(void)
{
    if (verbose) puts("STUB: BOMBomNewFromBomWithOptions called");
    return NULL;
}

void* BOMBomNewFromBomWithStripping(void)
{
    if (verbose) puts("STUB: BOMBomNewFromBomWithStripping called");
    return NULL;
}

void* BOMBomNewFromDirectory(void)
{
    if (verbose) puts("STUB: BOMBomNewFromDirectory called");
    return NULL;
}

void* BOMBomNewFromDirectoryWithSys(void)
{
    if (verbose) puts("STUB: BOMBomNewFromDirectoryWithSys called");
    return NULL;
}

void* BOMBomNewFromPath(void)
{
    if (verbose) puts("STUB: BOMBomNewFromPath called");
    return NULL;
}

void* BOMBomNewFromPathWithSys(void)
{
    if (verbose) puts("STUB: BOMBomNewFromPathWithSys called");
    return NULL;
}

void* BOMBomNewWithStorage(void)
{
    if (verbose) puts("STUB: BOMBomNewWithStorage called");
    return NULL;
}

void* BOMBomNewWithSys(void)
{
    if (verbose) puts("STUB: BOMBomNewWithSys called");
    return NULL;
}

void* BOMBomOpen(void)
{
    if (verbose) puts("STUB: BOMBomOpen called");
    return NULL;
}

void* BOMBomOpenWithStorage(void)
{
    if (verbose) puts("STUB: BOMBomOpenWithStorage called");
    return NULL;
}

void* BOMBomOpenWithSys(void)
{
    if (verbose) puts("STUB: BOMBomOpenWithSys called");
    return NULL;
}

void* BOMBomPathIDAndArchsForKey(void)
{
    if (verbose) puts("STUB: BOMBomPathIDAndArchsForKey called");
    return NULL;
}

void* BOMBomPathIDForKey(void)
{
    if (verbose) puts("STUB: BOMBomPathIDForKey called");
    return NULL;
}

void* BOMBomPathsTree(void)
{
    if (verbose) puts("STUB: BOMBomPathsTree called");
    return NULL;
}

void* BOMBomRemoveFSObject(void)
{
    if (verbose) puts("STUB: BOMBomRemoveFSObject called");
    return NULL;
}

void* BOMBomSync(void)
{
    if (verbose) puts("STUB: BOMBomSync called");
    return NULL;
}

void* BOMBomVariantAdd(void)
{
    if (verbose) puts("STUB: BOMBomVariantAdd called");
    return NULL;
}

void* BOMBomVariantCount(void)
{
    if (verbose) puts("STUB: BOMBomVariantCount called");
    return NULL;
}

void* BOMBomVariantGet(void)
{
    if (verbose) puts("STUB: BOMBomVariantGet called");
    return NULL;
}

void* BOMBomVariantGetList(void)
{
    if (verbose) puts("STUB: BOMBomVariantGetList called");
    return NULL;
}

void* BOMBomVariantRemove(void)
{
    if (verbose) puts("STUB: BOMBomVariantRemove called");
    return NULL;
}

void* BOMBomVerifyBom(void)
{
    if (verbose) puts("STUB: BOMBomVerifyBom called");
    return NULL;
}

void* BOMCFArrayMaxRange(void)
{
    if (verbose) puts("STUB: BOMCFArrayMaxRange called");
    return NULL;
}

void* BOMCFGetBoolValue(void)
{
    if (verbose) puts("STUB: BOMCFGetBoolValue called");
    return NULL;
}

void* BOMCFPropertyListReadFromPath(void)
{
    if (verbose) puts("STUB: BOMCFPropertyListReadFromPath called");
    return NULL;
}

void* BOMCFPropertyListReadFromPathWithSys(void)
{
    if (verbose) puts("STUB: BOMCFPropertyListReadFromPathWithSys called");
    return NULL;
}

void* BOMCFPropertyListWriteToPath(void)
{
    if (verbose) puts("STUB: BOMCFPropertyListWriteToPath called");
    return NULL;
}

void* BOMCFPropertyListWriteToPathWithSys(void)
{
    if (verbose) puts("STUB: BOMCFPropertyListWriteToPathWithSys called");
    return NULL;
}

void* BOMCFStringGetUTF8String(void)
{
    if (verbose) puts("STUB: BOMCFStringGetUTF8String called");
    return NULL;
}

void* BOMCKTreeBuildKey(void)
{
    if (verbose) puts("STUB: BOMCKTreeBuildKey called");
    return NULL;
}

void* BOMCKTreeCount(void)
{
    if (verbose) puts("STUB: BOMCKTreeCount called");
    return NULL;
}

void* BOMCKTreeGet(void)
{
    if (verbose) puts("STUB: BOMCKTreeGet called");
    return NULL;
}

void* BOMCPIOFree(void)
{
    if (verbose) puts("STUB: BOMCPIOFree called");
    return NULL;
}

void* BOMCPIOGetFile(void)
{
    if (verbose) puts("STUB: BOMCPIOGetFile called");
    return NULL;
}

void* BOMCPIONew(void)
{
    if (verbose) puts("STUB: BOMCPIONew called");
    return NULL;
}

void* BOMCPIORead(void)
{
    if (verbose) puts("STUB: BOMCPIORead called");
    return NULL;
}

void* BOMCPIOReadHeader(void)
{
    if (verbose) puts("STUB: BOMCPIOReadHeader called");
    return NULL;
}

void* BOMCPIOSeek(void)
{
    if (verbose) puts("STUB: BOMCPIOSeek called");
    return NULL;
}

void* BOMCPIOWrite(void)
{
    if (verbose) puts("STUB: BOMCPIOWrite called");
    return NULL;
}

void* BOMCPIOWriteDevice(void)
{
    if (verbose) puts("STUB: BOMCPIOWriteDevice called");
    return NULL;
}

void* BOMCPIOWriteDirectory(void)
{
    if (verbose) puts("STUB: BOMCPIOWriteDirectory called");
    return NULL;
}

void* BOMCPIOWriteHeader(void)
{
    if (verbose) puts("STUB: BOMCPIOWriteHeader called");
    return NULL;
}

void* BOMCPIOWriteSymlink(void)
{
    if (verbose) puts("STUB: BOMCPIOWriteSymlink called");
    return NULL;
}

void* BOMCPIOWriteTerminator(void)
{
    if (verbose) puts("STUB: BOMCPIOWriteTerminator called");
    return NULL;
}

void* BOMCRC32ForBuffer(void)
{
    if (verbose) puts("STUB: BOMCRC32ForBuffer called");
    return NULL;
}

void* BOMCRC32ForBufferSegment(void)
{
    if (verbose) puts("STUB: BOMCRC32ForBufferSegment called");
    return NULL;
}

void* BOMCRC32ForBufferSegmentFinal(void)
{
    if (verbose) puts("STUB: BOMCRC32ForBufferSegmentFinal called");
    return NULL;
}

void* BOMCRC32ForFile(void)
{
    if (verbose) puts("STUB: BOMCRC32ForFile called");
    return NULL;
}

void* BOMCRC32ForFileDesc(void)
{
    if (verbose) puts("STUB: BOMCRC32ForFileDesc called");
    return NULL;
}

void* BOMCRC32ForFileDescWithSys(void)
{
    if (verbose) puts("STUB: BOMCRC32ForFileDescWithSys called");
    return NULL;
}

void* BOMCRC32ForFileWithSys(void)
{
    if (verbose) puts("STUB: BOMCRC32ForFileWithSys called");
    return NULL;
}

void* BOMCopierCancelCopy(void)
{
    if (verbose) puts("STUB: BOMCopierCancelCopy called");
    return NULL;
}

void* BOMCopierCopy(void)
{
    if (verbose) puts("STUB: BOMCopierCopy called");
    return NULL;
}

void* BOMCopierCopyWithOptions(void)
{
    if (verbose) puts("STUB: BOMCopierCopyWithOptions called");
    return NULL;
}

void* BOMCopierCountFilesInArchive(void)
{
    if (verbose) puts("STUB: BOMCopierCountFilesInArchive called");
    return NULL;
}

void* BOMCopierCountFilesInDirectory(void)
{
    if (verbose) puts("STUB: BOMCopierCountFilesInDirectory called");
    return NULL;
}

void* BOMCopierFree(void)
{
    if (verbose) puts("STUB: BOMCopierFree called");
    return NULL;
}

void* BOMCopierGetArchiveFileDescriptor(void)
{
    if (verbose) puts("STUB: BOMCopierGetArchiveFileDescriptor called");
    return NULL;
}

void* BOMCopierMultiCopy(void)
{
    if (verbose) puts("STUB: BOMCopierMultiCopy called");
    return NULL;
}

void* BOMCopierMultiCopyWithOptions(void)
{
    if (verbose) puts("STUB: BOMCopierMultiCopyWithOptions called");
    return NULL;
}

void* BOMCopierNew(void)
{
    if (verbose) puts("STUB: BOMCopierNew called");
    return NULL;
}

void* BOMCopierNewWithSys(void)
{
    if (verbose) puts("STUB: BOMCopierNewWithSys called");
    return NULL;
}

void* BOMCopierRedirectPath(void)
{
    if (verbose) puts("STUB: BOMCopierRedirectPath called");
    return NULL;
}

void* BOMCopierRedirectSourcePath(void)
{
    if (verbose) puts("STUB: BOMCopierRedirectSourcePath called");
    return NULL;
}

void* BOMCopierSetCopyFileFinishedHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetCopyFileFinishedHandler called");
    return NULL;
}

void* BOMCopierSetCopyFileStartedHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetCopyFileStartedHandler called");
    return NULL;
}

void* BOMCopierSetCopyFileUpdateHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetCopyFileUpdateHandler called");
    return NULL;
}

void* BOMCopierSetCountFilesUpdateHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetCountFilesUpdateHandler called");
    return NULL;
}

void* BOMCopierSetFatalErrorHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetFatalErrorHandler called");
    return NULL;
}

void* BOMCopierSetFatalFileErrorHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetFatalFileErrorHandler called");
    return NULL;
}

void* BOMCopierSetFileConflictErrorHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetFileConflictErrorHandler called");
    return NULL;
}

void* BOMCopierSetFileErrorHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetFileErrorHandler called");
    return NULL;
}

void* BOMCopierSetMultiCopySourceStartedHandler(void)
{
    if (verbose) puts("STUB: BOMCopierSetMultiCopySourceStartedHandler called");
    return NULL;
}

void* BOMCopierSetPKZipPasswordRequester(void)
{
    if (verbose) puts("STUB: BOMCopierSetPKZipPasswordRequester called");
    return NULL;
}

void* BOMCopierSetUserData(void)
{
    if (verbose) puts("STUB: BOMCopierSetUserData called");
    return NULL;
}

void* BOMCopierUserData(void)
{
    if (verbose) puts("STUB: BOMCopierUserData called");
    return NULL;
}

void* BOMExceptionHandlerMessage(void)
{
    if (verbose) puts("STUB: BOMExceptionHandlerMessage called");
    return NULL;
}

void* BOMExceptionHandlerSet(void)
{
    if (verbose) puts("STUB: BOMExceptionHandlerSet called");
    return NULL;
}

void* BOMFSEnumeratorFree(void)
{
    if (verbose) puts("STUB: BOMFSEnumeratorFree called");
    return NULL;
}

void* BOMFSEnumeratorNew(void)
{
    if (verbose) puts("STUB: BOMFSEnumeratorNew called");
    return NULL;
}

void* BOMFSEnumeratorNewWithSys(void)
{
    if (verbose) puts("STUB: BOMFSEnumeratorNewWithSys called");
    return NULL;
}

void* BOMFSEnumeratorNext(void)
{
    if (verbose) puts("STUB: BOMFSEnumeratorNext called");
    return NULL;
}

void* BOMFSObjectArchCount(void)
{
    if (verbose) puts("STUB: BOMFSObjectArchCount called");
    return NULL;
}

void* BOMFSObjectBlockID(void)
{
    if (verbose) puts("STUB: BOMFSObjectBlockID called");
    return NULL;
}

void* BOMFSObjectChecksum(void)
{
    if (verbose) puts("STUB: BOMFSObjectChecksum called");
    return NULL;
}

void* BOMFSObjectContainsArchitecture(void)
{
    if (verbose) puts("STUB: BOMFSObjectContainsArchitecture called");
    return NULL;
}

void* BOMFSObjectCopy(void)
{
    if (verbose) puts("STUB: BOMFSObjectCopy called");
    return NULL;
}

void* BOMFSObjectDeviceID(void)
{
    if (verbose) puts("STUB: BOMFSObjectDeviceID called");
    return NULL;
}

void* BOMFSObjectENewFromPath(void)
{
    if (verbose) puts("STUB: BOMFSObjectENewFromPath called");
    return NULL;
}

void* BOMFSObjectENewFromPathWithSys(void)
{
    if (verbose) puts("STUB: BOMFSObjectENewFromPathWithSys called");
    return NULL;
}

void* BOMFSObjectFree(void)
{
    if (verbose) puts("STUB: BOMFSObjectFree called");
    return NULL;
}

void* BOMFSObjectGetArch(void)
{
    if (verbose) puts("STUB: BOMFSObjectGetArch called");
    return NULL;
}

void* BOMFSObjectGetArchSize(void)
{
    if (verbose) puts("STUB: BOMFSObjectGetArchSize called");
    return NULL;
}

void* BOMFSObjectGetArchSubtype(void)
{
    if (verbose) puts("STUB: BOMFSObjectGetArchSubtype called");
    return NULL;
}

void* BOMFSObjectGetHardlinkCount(void)
{
    if (verbose) puts("STUB: BOMFSObjectGetHardlinkCount called");
    return NULL;
}

void* BOMFSObjectGetHardlinkPath(void)
{
    if (verbose) puts("STUB: BOMFSObjectGetHardlinkPath called");
    return NULL;
}

void* BOMFSObjectGroupID(void)
{
    if (verbose) puts("STUB: BOMFSObjectGroupID called");
    return NULL;
}

void* BOMFSObjectIsBinaryObject(void)
{
    if (verbose) puts("STUB: BOMFSObjectIsBinaryObject called");
    return NULL;
}

void* BOMFSObjectIsHardlink(void)
{
    if (verbose) puts("STUB: BOMFSObjectIsHardlink called");
    return NULL;
}

void* BOMFSObjectModTime(void)
{
    if (verbose) puts("STUB: BOMFSObjectModTime called");
    return NULL;
}

void* BOMFSObjectMode(void)
{
    if (verbose) puts("STUB: BOMFSObjectMode called");
    return NULL;
}

void* BOMFSObjectNew(void)
{
    if (verbose) puts("STUB: BOMFSObjectNew called");
    return NULL;
}

void* BOMFSObjectNewFromPath(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewFromPath called");
    return NULL;
}

void* BOMFSObjectNewFromPathDeferred(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewFromPathDeferred called");
    return NULL;
}

void* BOMFSObjectNewFromPathDeferredWithSys(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewFromPathDeferredWithSys called");
    return NULL;
}

void* BOMFSObjectNewFromPathString(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewFromPathString called");
    return NULL;
}

void* BOMFSObjectNewFromPathStringWithSys(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewFromPathStringWithSys called");
    return NULL;
}

void* BOMFSObjectNewFromPathWithSys(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewFromPathWithSys called");
    return NULL;
}

void* BOMFSObjectNewWithSys(void)
{
    if (verbose) puts("STUB: BOMFSObjectNewWithSys called");
    return NULL;
}

void* BOMFSObjectOpaqueData(void)
{
    if (verbose) puts("STUB: BOMFSObjectOpaqueData called");
    return NULL;
}

void* BOMFSObjectOpaqueDataSize(void)
{
    if (verbose) puts("STUB: BOMFSObjectOpaqueDataSize called");
    return NULL;
}

void* BOMFSObjectParentPathID(void)
{
    if (verbose) puts("STUB: BOMFSObjectParentPathID called");
    return NULL;
}

void* BOMFSObjectParseSummary(void)
{
    if (verbose) puts("STUB: BOMFSObjectParseSummary called");
    return NULL;
}

void* BOMFSObjectParseSummaryWithSys(void)
{
    if (verbose) puts("STUB: BOMFSObjectParseSummaryWithSys called");
    return NULL;
}

void* BOMFSObjectPathID(void)
{
    if (verbose) puts("STUB: BOMFSObjectPathID called");
    return NULL;
}

void* BOMFSObjectPathName(void)
{
    if (verbose) puts("STUB: BOMFSObjectPathName called");
    return NULL;
}

void* BOMFSObjectPathNameString(void)
{
    if (verbose) puts("STUB: BOMFSObjectPathNameString called");
    return NULL;
}

void* BOMFSObjectSetBlockID(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetBlockID called");
    return NULL;
}

void* BOMFSObjectSetChecksum(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetChecksum called");
    return NULL;
}

void* BOMFSObjectSetDeviceID(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetDeviceID called");
    return NULL;
}

void* BOMFSObjectSetFlags(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetFlags called");
    return NULL;
}

void* BOMFSObjectSetGroupID(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetGroupID called");
    return NULL;
}

void* BOMFSObjectSetModTime(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetModTime called");
    return NULL;
}

void* BOMFSObjectSetMode(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetMode called");
    return NULL;
}

void* BOMFSObjectSetOpaqueData(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetOpaqueData called");
    return NULL;
}

void* BOMFSObjectSetParentPathID(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetParentPathID called");
    return NULL;
}

void* BOMFSObjectSetPathID(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetPathID called");
    return NULL;
}

void* BOMFSObjectSetPathName(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetPathName called");
    return NULL;
}

void* BOMFSObjectSetShortName(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetShortName called");
    return NULL;
}

void* BOMFSObjectSetSize(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetSize called");
    return NULL;
}

void* BOMFSObjectSetSymlinkTarget(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetSymlinkTarget called");
    return NULL;
}

void* BOMFSObjectSetUserID(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetUserID called");
    return NULL;
}

void* BOMFSObjectSetVisitOrder(void)
{
    if (verbose) puts("STUB: BOMFSObjectSetVisitOrder called");
    return NULL;
}

void* BOMFSObjectShortName(void)
{
    if (verbose) puts("STUB: BOMFSObjectShortName called");
    return NULL;
}

void* BOMFSObjectShortNameString(void)
{
    if (verbose) puts("STUB: BOMFSObjectShortNameString called");
    return NULL;
}

void* BOMFSObjectSize(void)
{
    if (verbose) puts("STUB: BOMFSObjectSize called");
    return NULL;
}

void* BOMFSObjectSummary(void)
{
    if (verbose) puts("STUB: BOMFSObjectSummary called");
    return NULL;
}

void* BOMFSObjectSummaryWithFormat(void)
{
    if (verbose) puts("STUB: BOMFSObjectSummaryWithFormat called");
    return NULL;
}

void* BOMFSObjectSymlinkTarget(void)
{
    if (verbose) puts("STUB: BOMFSObjectSymlinkTarget called");
    return NULL;
}

void* BOMFSObjectSymlinkTargetString(void)
{
    if (verbose) puts("STUB: BOMFSObjectSymlinkTargetString called");
    return NULL;
}

void* BOMFSObjectThinKeepingArchs(void)
{
    if (verbose) puts("STUB: BOMFSObjectThinKeepingArchs called");
    return NULL;
}

void* BOMFSObjectType(void)
{
    if (verbose) puts("STUB: BOMFSObjectType called");
    return NULL;
}

void* BOMFSObjectTypeForMode(void)
{
    if (verbose) puts("STUB: BOMFSObjectTypeForMode called");
    return NULL;
}

void* BOMFSObjectTypeFromRawData(void)
{
    if (verbose) puts("STUB: BOMFSObjectTypeFromRawData called");
    return NULL;
}

void* BOMFSObjectUserID(void)
{
    if (verbose) puts("STUB: BOMFSObjectUserID called");
    return NULL;
}

void* BOMFSObjectVisitOrder(void)
{
    if (verbose) puts("STUB: BOMFSObjectVisitOrder called");
    return NULL;
}

void* BOMFileClose(void)
{
    if (verbose) puts("STUB: BOMFileClose called");
    return NULL;
}

void* BOMFileNewFromFD(void)
{
    if (verbose) puts("STUB: BOMFileNewFromFD called");
    return NULL;
}

void* BOMFileNewFromFDWithSys(void)
{
    if (verbose) puts("STUB: BOMFileNewFromFDWithSys called");
    return NULL;
}

void* BOMFileNewMirror(void)
{
    if (verbose) puts("STUB: BOMFileNewMirror called");
    return NULL;
}

void* BOMFileNewMirrorWithSys(void)
{
    if (verbose) puts("STUB: BOMFileNewMirrorWithSys called");
    return NULL;
}

void* BOMFileOffset(void)
{
    if (verbose) puts("STUB: BOMFileOffset called");
    return NULL;
}

void* BOMFileOpen(void)
{
    if (verbose) puts("STUB: BOMFileOpen called");
    return NULL;
}

void* BOMFileOpenSTDIN(void)
{
    if (verbose) puts("STUB: BOMFileOpenSTDIN called");
    return NULL;
}

void* BOMFileOpenSTDOUT(void)
{
    if (verbose) puts("STUB: BOMFileOpenSTDOUT called");
    return NULL;
}

void* BOMFileOpenWithSys(void)
{
    if (verbose) puts("STUB: BOMFileOpenWithSys called");
    return NULL;
}

void* BOMFileRead(void)
{
    if (verbose) puts("STUB: BOMFileRead called");
    return NULL;
}

void* BOMFileSeek(void)
{
    if (verbose) puts("STUB: BOMFileSeek called");
    return NULL;
}

void* BOMFileUncompressedOffset(void)
{
    if (verbose) puts("STUB: BOMFileUncompressedOffset called");
    return NULL;
}

void* BOMFileWrite(void)
{
    if (verbose) puts("STUB: BOMFileWrite called");
    return NULL;
}

void* BOMFilesystemInfoCreate(void)
{
    if (verbose) puts("STUB: BOMFilesystemInfoCreate called");
    return NULL;
}

void* BOMFilesystemInfoDestroy(void)
{
    if (verbose) puts("STUB: BOMFilesystemInfoDestroy called");
    return NULL;
}

void* BOMFilesystemInfoQuery(void)
{
    if (verbose) puts("STUB: BOMFilesystemInfoQuery called");
    return NULL;
}

void* BOMHardLinkTableFree(void)
{
    if (verbose) puts("STUB: BOMHardLinkTableFree called");
    return NULL;
}

void* BOMHardLinkTableGet(void)
{
    if (verbose) puts("STUB: BOMHardLinkTableGet called");
    return NULL;
}

void* BOMHardLinkTableGetPathAndData(void)
{
    if (verbose) puts("STUB: BOMHardLinkTableGetPathAndData called");
    return NULL;
}

void* BOMHardLinkTableNew(void)
{
    if (verbose) puts("STUB: BOMHardLinkTableNew called");
    return NULL;
}

void* BOMHardLinkTableSet(void)
{
    if (verbose) puts("STUB: BOMHardLinkTableSet called");
    return NULL;
}

void* BOMHardLinkTableSetPathAndData(void)
{
    if (verbose) puts("STUB: BOMHardLinkTableSetPathAndData called");
    return NULL;
}

void* BOMMemoryDump(void)
{
    if (verbose) puts("STUB: BOMMemoryDump called");
    return NULL;
}

void* BOMNameForFSObjectType(void)
{
    if (verbose) puts("STUB: BOMNameForFSObjectType called");
    return NULL;
}

void* BOMPatternCompileString(void)
{
    if (verbose) puts("STUB: BOMPatternCompileString called");
    return NULL;
}

void* BOMPatternFree(void)
{
    if (verbose) puts("STUB: BOMPatternFree called");
    return NULL;
}

void* BOMPatternListExtractFromFile(void)
{
    if (verbose) puts("STUB: BOMPatternListExtractFromFile called");
    return NULL;
}

void* BOMPatternListExtractFromStrings(void)
{
    if (verbose) puts("STUB: BOMPatternListExtractFromStrings called");
    return NULL;
}

void* BOMPatternListFree(void)
{
    if (verbose) puts("STUB: BOMPatternListFree called");
    return NULL;
}

void* BOMPatternListFromStringList(void)
{
    if (verbose) puts("STUB: BOMPatternListFromStringList called");
    return NULL;
}

void* BOMPatternListMatch(void)
{
    if (verbose) puts("STUB: BOMPatternListMatch called");
    return NULL;
}

void* BOMPatternListNew(void)
{
    if (verbose) puts("STUB: BOMPatternListNew called");
    return NULL;
}

void* BOMPatternMatch(void)
{
    if (verbose) puts("STUB: BOMPatternMatch called");
    return NULL;
}

void* BOMStackCount(void)
{
    if (verbose) puts("STUB: BOMStackCount called");
    return NULL;
}

void* BOMStackFree(void)
{
    if (verbose) puts("STUB: BOMStackFree called");
    return NULL;
}

void* BOMStackIsEmpty(void)
{
    if (verbose) puts("STUB: BOMStackIsEmpty called");
    return NULL;
}

void* BOMStackNew(void)
{
    if (verbose) puts("STUB: BOMStackNew called");
    return NULL;
}

void* BOMStackPeek(void)
{
    if (verbose) puts("STUB: BOMStackPeek called");
    return NULL;
}

void* BOMStackPoke(void)
{
    if (verbose) puts("STUB: BOMStackPoke called");
    return NULL;
}

void* BOMStackPop(void)
{
    if (verbose) puts("STUB: BOMStackPop called");
    return NULL;
}

void* BOMStackPush(void)
{
    if (verbose) puts("STUB: BOMStackPush called");
    return NULL;
}

void* BOMStorageCommit(void)
{
    if (verbose) puts("STUB: BOMStorageCommit called");
    return NULL;
}

void* BOMStorageCompact(void)
{
    if (verbose) puts("STUB: BOMStorageCompact called");
    return NULL;
}

void* BOMStorageCopyFromBlock(void)
{
    if (verbose) puts("STUB: BOMStorageCopyFromBlock called");
    return NULL;
}

void* BOMStorageCopyFromBlockRange(void)
{
    if (verbose) puts("STUB: BOMStorageCopyFromBlockRange called");
    return NULL;
}

void* BOMStorageCopyToBlock(void)
{
    if (verbose) puts("STUB: BOMStorageCopyToBlock called");
    return NULL;
}

void* BOMStorageCopyToBlockRange(void)
{
    if (verbose) puts("STUB: BOMStorageCopyToBlockRange called");
    return NULL;
}

void* BOMStorageCount(void)
{
    if (verbose) puts("STUB: BOMStorageCount called");
    return NULL;
}

void* BOMStorageDump(void)
{
    if (verbose) puts("STUB: BOMStorageDump called");
    return NULL;
}

void* BOMStorageDumpFreeList(void)
{
    if (verbose) puts("STUB: BOMStorageDumpFreeList called");
    return NULL;
}

void* BOMStorageFileName(void)
{
    if (verbose) puts("STUB: BOMStorageFileName called");
    return NULL;
}

void* BOMStorageFindActualFreeSpace(void)
{
    if (verbose) puts("STUB: BOMStorageFindActualFreeSpace called");
    return NULL;
}

void* BOMStorageFree(void)
{
    if (verbose) puts("STUB: BOMStorageFree called");
    return NULL;
}

void* BOMStorageFreeBlock(void)
{
    if (verbose) puts("STUB: BOMStorageFreeBlock called");
    return NULL;
}

void* BOMStorageFreeNamedBlock(void)
{
    if (verbose) puts("STUB: BOMStorageFreeNamedBlock called");
    return NULL;
}

void* BOMStorageGetNamedBlock(void)
{
    if (verbose) puts("STUB: BOMStorageGetNamedBlock called");
    return NULL;
}

void* BOMStorageGetSys(void)
{
    if (verbose) puts("STUB: BOMStorageGetSys called");
    return NULL;
}

void* BOMStorageIsOpenForWriting(void)
{
    if (verbose) puts("STUB: BOMStorageIsOpenForWriting called");
    return NULL;
}

void* BOMStorageIsStorageFile(void)
{
    if (verbose) puts("STUB: BOMStorageIsStorageFile called");
    return NULL;
}

void* BOMStorageIsStorageFileWithSys(void)
{
    if (verbose) puts("STUB: BOMStorageIsStorageFileWithSys called");
    return NULL;
}

void* BOMStorageNew(void)
{
    if (verbose) puts("STUB: BOMStorageNew called");
    return NULL;
}

void* BOMStorageNewBlock(void)
{
    if (verbose) puts("STUB: BOMStorageNewBlock called");
    return NULL;
}

void* BOMStorageNewInRAM(void)
{
    if (verbose) puts("STUB: BOMStorageNewInRAM called");
    return NULL;
}

void* BOMStorageNewNamedBlock(void)
{
    if (verbose) puts("STUB: BOMStorageNewNamedBlock called");
    return NULL;
}

void* BOMStorageNewWithOptions(void)
{
    if (verbose) puts("STUB: BOMStorageNewWithOptions called");
    return NULL;
}

void* BOMStorageNewWithOptionsAndSys(void)
{
    if (verbose) puts("STUB: BOMStorageNewWithOptionsAndSys called");
    return NULL;
}

void* BOMStorageNewWithSys(void)
{
    if (verbose) puts("STUB: BOMStorageNewWithSys called");
    return NULL;
}

void* BOMStorageOpen(void)
{
    if (verbose) puts("STUB: BOMStorageOpen called");
    return NULL;
}

void* BOMStorageOpenInRAM(void)
{
    if (verbose) puts("STUB: BOMStorageOpenInRAM called");
    return NULL;
}

void* BOMStorageOpenWithSys(void)
{
    if (verbose) puts("STUB: BOMStorageOpenWithSys called");
    return NULL;
}

void* BOMStorageSetBlockData(void)
{
    if (verbose) puts("STUB: BOMStorageSetBlockData called");
    return NULL;
}

void* BOMStorageSizeOfBlock(void)
{
    if (verbose) puts("STUB: BOMStorageSizeOfBlock called");
    return NULL;
}

void* BOMStreamFlush(void)
{
    if (verbose) puts("STUB: BOMStreamFlush called");
    return NULL;
}

void* BOMStreamFree(void)
{
    if (verbose) puts("STUB: BOMStreamFree called");
    return NULL;
}

void* BOMStreamGetByteOrder(void)
{
    if (verbose) puts("STUB: BOMStreamGetByteOrder called");
    return NULL;
}

void* BOMStreamGetSys(void)
{
    if (verbose) puts("STUB: BOMStreamGetSys called");
    return NULL;
}

void* BOMStreamReadAddress(void)
{
    if (verbose) puts("STUB: BOMStreamReadAddress called");
    return NULL;
}

void* BOMStreamReadBuffer(void)
{
    if (verbose) puts("STUB: BOMStreamReadBuffer called");
    return NULL;
}

void* BOMStreamReadUInt16(void)
{
    if (verbose) puts("STUB: BOMStreamReadUInt16 called");
    return NULL;
}

void* BOMStreamReadUInt32(void)
{
    if (verbose) puts("STUB: BOMStreamReadUInt32 called");
    return NULL;
}

void* BOMStreamReadUInt8(void)
{
    if (verbose) puts("STUB: BOMStreamReadUInt8 called");
    return NULL;
}

void* BOMStreamSetByteOrder(void)
{
    if (verbose) puts("STUB: BOMStreamSetByteOrder called");
    return NULL;
}

void* BOMStreamWithAddress(void)
{
    if (verbose) puts("STUB: BOMStreamWithAddress called");
    return NULL;
}

void* BOMStreamWithBlockID(void)
{
    if (verbose) puts("STUB: BOMStreamWithBlockID called");
    return NULL;
}

void* BOMStreamWithFile(void)
{
    if (verbose) puts("STUB: BOMStreamWithFile called");
    return NULL;
}

void* BOMStreamWithFileAndSys(void)
{
    if (verbose) puts("STUB: BOMStreamWithFileAndSys called");
    return NULL;
}

void* BOMStreamWriteBuffer(void)
{
    if (verbose) puts("STUB: BOMStreamWriteBuffer called");
    return NULL;
}

void* BOMStreamWriteUInt16(void)
{
    if (verbose) puts("STUB: BOMStreamWriteUInt16 called");
    return NULL;
}

void* BOMStreamWriteUInt32(void)
{
    if (verbose) puts("STUB: BOMStreamWriteUInt32 called");
    return NULL;
}

void* BOMStreamWriteUInt8(void)
{
    if (verbose) puts("STUB: BOMStreamWriteUInt8 called");
    return NULL;
}

void* BOMTreeCommit(void)
{
    if (verbose) puts("STUB: BOMTreeCommit called");
    return NULL;
}

void* BOMTreeCopyToTree(void)
{
    if (verbose) puts("STUB: BOMTreeCopyToTree called");
    return NULL;
}

void* BOMTreeCount(void)
{
    if (verbose) puts("STUB: BOMTreeCount called");
    return NULL;
}

void* BOMTreeFree(void)
{
    if (verbose) puts("STUB: BOMTreeFree called");
    return NULL;
}

void* BOMTreeGetValue(void)
{
    if (verbose) puts("STUB: BOMTreeGetValue called");
    return NULL;
}

void* BOMTreeGetValueSize(void)
{
    if (verbose) puts("STUB: BOMTreeGetValueSize called");
    return NULL;
}

void* BOMTreeIteratorFree(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorFree called");
    return NULL;
}

void* BOMTreeIteratorIsAtEnd(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorIsAtEnd called");
    return NULL;
}

void* BOMTreeIteratorKey(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorKey called");
    return NULL;
}

void* BOMTreeIteratorKeySize(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorKeySize called");
    return NULL;
}

void* BOMTreeIteratorNew(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorNew called");
    return NULL;
}

void* BOMTreeIteratorNext(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorNext called");
    return NULL;
}

void* BOMTreeIteratorSet(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorSet called");
    return NULL;
}

void* BOMTreeIteratorValue(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorValue called");
    return NULL;
}

void* BOMTreeIteratorValueSize(void)
{
    if (verbose) puts("STUB: BOMTreeIteratorValueSize called");
    return NULL;
}

void* BOMTreeNew(void)
{
    if (verbose) puts("STUB: BOMTreeNew called");
    return NULL;
}

void* BOMTreeNewWithName(void)
{
    if (verbose) puts("STUB: BOMTreeNewWithName called");
    return NULL;
}

void* BOMTreeNewWithOptions(void)
{
    if (verbose) puts("STUB: BOMTreeNewWithOptions called");
    return NULL;
}

void* BOMTreeOpen(void)
{
    if (verbose) puts("STUB: BOMTreeOpen called");
    return NULL;
}

void* BOMTreeOpenWithName(void)
{
    if (verbose) puts("STUB: BOMTreeOpenWithName called");
    return NULL;
}

void* BOMTreeRemoveAndFree(void)
{
    if (verbose) puts("STUB: BOMTreeRemoveAndFree called");
    return NULL;
}

void* BOMTreeRemoveValue(void)
{
    if (verbose) puts("STUB: BOMTreeRemoveValue called");
    return NULL;
}

void* BOMTreeSetCompareFunc(void)
{
    if (verbose) puts("STUB: BOMTreeSetCompareFunc called");
    return NULL;
}

void* BOMTreeSetValue(void)
{
    if (verbose) puts("STUB: BOMTreeSetValue called");
    return NULL;
}

void* BOMTreeStorage(void)
{
    if (verbose) puts("STUB: BOMTreeStorage called");
    return NULL;
}

void* BOM_calloc(void)
{
    if (verbose) puts("STUB: BOM_calloc called");
    return NULL;
}

void* BOM_malloc(void)
{
    if (verbose) puts("STUB: BOM_malloc called");
    return NULL;
}

void* BOM_malloczero(void)
{
    if (verbose) puts("STUB: BOM_malloczero called");
    return NULL;
}

void* BOM_realloc(void)
{
    if (verbose) puts("STUB: BOM_realloc called");
    return NULL;
}

void* BOM_realloczero(void)
{
    if (verbose) puts("STUB: BOM_realloczero called");
    return NULL;
}

void* BOM_strrncmp(void)
{
    if (verbose) puts("STUB: BOM_strrncmp called");
    return NULL;
}

void* BomSys_access(void)
{
    if (verbose) puts("STUB: BomSys_access called");
    return NULL;
}

void* BomSys_chdir(void)
{
    if (verbose) puts("STUB: BomSys_chdir called");
    return NULL;
}

void* BomSys_chflags(void)
{
    if (verbose) puts("STUB: BomSys_chflags called");
    return NULL;
}

void* BomSys_chmod(void)
{
    if (verbose) puts("STUB: BomSys_chmod called");
    return NULL;
}

void* BomSys_close(void)
{
    if (verbose) puts("STUB: BomSys_close called");
    return NULL;
}

void* BomSys_closedir(void)
{
    if (verbose) puts("STUB: BomSys_closedir called");
    return NULL;
}

void* BomSys_default(void)
{
    if (verbose) puts("STUB: BomSys_default called");
    return NULL;
}

void* BomSys_fcntl(void)
{
    if (verbose) puts("STUB: BomSys_fcntl called");
    return NULL;
}

void* BomSys_free(void)
{
    if (verbose) puts("STUB: BomSys_free called");
    return NULL;
}

void* BomSys_fstat(void)
{
    if (verbose) puts("STUB: BomSys_fstat called");
    return NULL;
}

void* BomSys_fstatfs(void)
{
    if (verbose) puts("STUB: BomSys_fstatfs called");
    return NULL;
}

void* BomSys_fsync(void)
{
    if (verbose) puts("STUB: BomSys_fsync called");
    return NULL;
}

void* BomSys_getattrlist(void)
{
    if (verbose) puts("STUB: BomSys_getattrlist called");
    return NULL;
}

void* BomSys_getcwd(void)
{
    if (verbose) puts("STUB: BomSys_getcwd called");
    return NULL;
}

void* BomSys_getxattr(void)
{
    if (verbose) puts("STUB: BomSys_getxattr called");
    return NULL;
}

void* BomSys_init(void)
{
    if (verbose) puts("STUB: BomSys_init called");
    return NULL;
}

void* BomSys_lchown(void)
{
    if (verbose) puts("STUB: BomSys_lchown called");
    return NULL;
}

void* BomSys_link(void)
{
    if (verbose) puts("STUB: BomSys_link called");
    return NULL;
}

void* BomSys_listxattr(void)
{
    if (verbose) puts("STUB: BomSys_listxattr called");
    return NULL;
}

void* BomSys_log_attach(void)
{
    if (verbose) puts("STUB: BomSys_log_attach called");
    return NULL;
}

void* BomSys_log_attached(void)
{
    if (verbose) puts("STUB: BomSys_log_attached called");
    return NULL;
}

void* BomSys_log_config(void)
{
    if (verbose) puts("STUB: BomSys_log_config called");
    return NULL;
}

void* BomSys_log_detach(void)
{
    if (verbose) puts("STUB: BomSys_log_detach called");
    return NULL;
}

void* BomSys_log_pause(void)
{
    if (verbose) puts("STUB: BomSys_log_pause called");
    return NULL;
}

void* BomSys_log_resume(void)
{
    if (verbose) puts("STUB: BomSys_log_resume called");
    return NULL;
}

void* BomSys_lseek(void)
{
    if (verbose) puts("STUB: BomSys_lseek called");
    return NULL;
}

void* BomSys_lstat(void)
{
    if (verbose) puts("STUB: BomSys_lstat called");
    return NULL;
}

void* BomSys_mkdir(void)
{
    if (verbose) puts("STUB: BomSys_mkdir called");
    return NULL;
}

void* BomSys_mknod(void)
{
    if (verbose) puts("STUB: BomSys_mknod called");
    return NULL;
}

void* BomSys_mktemp(void)
{
    if (verbose) puts("STUB: BomSys_mktemp called");
    return NULL;
}

void* BomSys_mmap(void)
{
    if (verbose) puts("STUB: BomSys_mmap called");
    return NULL;
}

void* BomSys_munmap(void)
{
    if (verbose) puts("STUB: BomSys_munmap called");
    return NULL;
}

void* BomSys_open(void)
{
    if (verbose) puts("STUB: BomSys_open called");
    return NULL;
}

void* BomSys_openFor(void)
{
    if (verbose) puts("STUB: BomSys_openFor called");
    return NULL;
}

void* BomSys_opendir(void)
{
    if (verbose) puts("STUB: BomSys_opendir called");
    return NULL;
}

void* BomSys_read(void)
{
    if (verbose) puts("STUB: BomSys_read called");
    return NULL;
}

void* BomSys_readdir(void)
{
    if (verbose) puts("STUB: BomSys_readdir called");
    return NULL;
}

void* BomSys_readdir_r(void)
{
    if (verbose) puts("STUB: BomSys_readdir_r called");
    return NULL;
}

void* BomSys_readlink(void)
{
    if (verbose) puts("STUB: BomSys_readlink called");
    return NULL;
}

void* BomSys_realpath(void)
{
    if (verbose) puts("STUB: BomSys_realpath called");
    return NULL;
}

void* BomSys_removexattr(void)
{
    if (verbose) puts("STUB: BomSys_removexattr called");
    return NULL;
}

void* BomSys_rename(void)
{
    if (verbose) puts("STUB: BomSys_rename called");
    return NULL;
}

void* BomSys_rmdir(void)
{
    if (verbose) puts("STUB: BomSys_rmdir called");
    return NULL;
}

void* BomSys_set_access(void)
{
    if (verbose) puts("STUB: BomSys_set_access called");
    return NULL;
}

void* BomSys_set_chdir(void)
{
    if (verbose) puts("STUB: BomSys_set_chdir called");
    return NULL;
}

void* BomSys_set_chflags(void)
{
    if (verbose) puts("STUB: BomSys_set_chflags called");
    return NULL;
}

void* BomSys_set_chmod(void)
{
    if (verbose) puts("STUB: BomSys_set_chmod called");
    return NULL;
}

void* BomSys_set_close(void)
{
    if (verbose) puts("STUB: BomSys_set_close called");
    return NULL;
}

void* BomSys_set_closedir(void)
{
    if (verbose) puts("STUB: BomSys_set_closedir called");
    return NULL;
}

void* BomSys_set_fcntl(void)
{
    if (verbose) puts("STUB: BomSys_set_fcntl called");
    return NULL;
}

void* BomSys_set_fstat(void)
{
    if (verbose) puts("STUB: BomSys_set_fstat called");
    return NULL;
}

void* BomSys_set_fstatfs(void)
{
    if (verbose) puts("STUB: BomSys_set_fstatfs called");
    return NULL;
}

void* BomSys_set_fsync(void)
{
    if (verbose) puts("STUB: BomSys_set_fsync called");
    return NULL;
}

void* BomSys_set_getattrlist(void)
{
    if (verbose) puts("STUB: BomSys_set_getattrlist called");
    return NULL;
}

void* BomSys_set_getcwd(void)
{
    if (verbose) puts("STUB: BomSys_set_getcwd called");
    return NULL;
}

void* BomSys_set_getxattr(void)
{
    if (verbose) puts("STUB: BomSys_set_getxattr called");
    return NULL;
}

void* BomSys_set_lchown(void)
{
    if (verbose) puts("STUB: BomSys_set_lchown called");
    return NULL;
}

void* BomSys_set_link(void)
{
    if (verbose) puts("STUB: BomSys_set_link called");
    return NULL;
}

void* BomSys_set_listxattr(void)
{
    if (verbose) puts("STUB: BomSys_set_listxattr called");
    return NULL;
}

void* BomSys_set_lseek(void)
{
    if (verbose) puts("STUB: BomSys_set_lseek called");
    return NULL;
}

void* BomSys_set_lstat(void)
{
    if (verbose) puts("STUB: BomSys_set_lstat called");
    return NULL;
}

void* BomSys_set_mkdir(void)
{
    if (verbose) puts("STUB: BomSys_set_mkdir called");
    return NULL;
}

void* BomSys_set_mknod(void)
{
    if (verbose) puts("STUB: BomSys_set_mknod called");
    return NULL;
}

void* BomSys_set_mktemp(void)
{
    if (verbose) puts("STUB: BomSys_set_mktemp called");
    return NULL;
}

void* BomSys_set_mmap(void)
{
    if (verbose) puts("STUB: BomSys_set_mmap called");
    return NULL;
}

void* BomSys_set_munmap(void)
{
    if (verbose) puts("STUB: BomSys_set_munmap called");
    return NULL;
}

void* BomSys_set_open(void)
{
    if (verbose) puts("STUB: BomSys_set_open called");
    return NULL;
}

void* BomSys_set_openFor(void)
{
    if (verbose) puts("STUB: BomSys_set_openFor called");
    return NULL;
}

void* BomSys_set_opendir(void)
{
    if (verbose) puts("STUB: BomSys_set_opendir called");
    return NULL;
}

void* BomSys_set_read(void)
{
    if (verbose) puts("STUB: BomSys_set_read called");
    return NULL;
}

void* BomSys_set_readdir(void)
{
    if (verbose) puts("STUB: BomSys_set_readdir called");
    return NULL;
}

void* BomSys_set_readdir_r(void)
{
    if (verbose) puts("STUB: BomSys_set_readdir_r called");
    return NULL;
}

void* BomSys_set_readlink(void)
{
    if (verbose) puts("STUB: BomSys_set_readlink called");
    return NULL;
}

void* BomSys_set_realpath(void)
{
    if (verbose) puts("STUB: BomSys_set_realpath called");
    return NULL;
}

void* BomSys_set_removexattr(void)
{
    if (verbose) puts("STUB: BomSys_set_removexattr called");
    return NULL;
}

void* BomSys_set_rename(void)
{
    if (verbose) puts("STUB: BomSys_set_rename called");
    return NULL;
}

void* BomSys_set_rmdir(void)
{
    if (verbose) puts("STUB: BomSys_set_rmdir called");
    return NULL;
}

void* BomSys_set_setxattr(void)
{
    if (verbose) puts("STUB: BomSys_set_setxattr called");
    return NULL;
}

void* BomSys_set_stat(void)
{
    if (verbose) puts("STUB: BomSys_set_stat called");
    return NULL;
}

void* BomSys_set_statfs(void)
{
    if (verbose) puts("STUB: BomSys_set_statfs called");
    return NULL;
}

void* BomSys_set_symlink(void)
{
    if (verbose) puts("STUB: BomSys_set_symlink called");
    return NULL;
}

void* BomSys_set_unlink(void)
{
    if (verbose) puts("STUB: BomSys_set_unlink called");
    return NULL;
}

void* BomSys_set_utimes(void)
{
    if (verbose) puts("STUB: BomSys_set_utimes called");
    return NULL;
}

void* BomSys_set_write(void)
{
    if (verbose) puts("STUB: BomSys_set_write called");
    return NULL;
}

void* BomSys_setxattr(void)
{
    if (verbose) puts("STUB: BomSys_setxattr called");
    return NULL;
}

void* BomSys_stat(void)
{
    if (verbose) puts("STUB: BomSys_stat called");
    return NULL;
}

void* BomSys_statfs(void)
{
    if (verbose) puts("STUB: BomSys_statfs called");
    return NULL;
}

void* BomSys_symlink(void)
{
    if (verbose) puts("STUB: BomSys_symlink called");
    return NULL;
}

void* BomSys_unlink(void)
{
    if (verbose) puts("STUB: BomSys_unlink called");
    return NULL;
}

void* BomSys_utimes(void)
{
    if (verbose) puts("STUB: BomSys_utimes called");
    return NULL;
}

void* BomSys_write(void)
{
    if (verbose) puts("STUB: BomSys_write called");
    return NULL;
}

void* _BOMBomPathTreePrint(void)
{
    if (verbose) puts("STUB: _BOMBomPathTreePrint called");
    return NULL;
}

void* _BOMBomPrint(void)
{
    if (verbose) puts("STUB: _BOMBomPrint called");
    return NULL;
}

void* _BOMCFArrayPrint(void)
{
    if (verbose) puts("STUB: _BOMCFArrayPrint called");
    return NULL;
}

void* _BOMExceptionHandlerCall(void)
{
    if (verbose) puts("STUB: _BOMExceptionHandlerCall called");
    return NULL;
}

void* _BOMFatalException(void)
{
    if (verbose) puts("STUB: _BOMFatalException called");
    return NULL;
}
