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


#include <DeviceLink/DeviceLink.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CFPrintf(void)
{
    if (verbose) puts("STUB: CFPrintf called");
    return NULL;
}

void* DLConnect(void)
{
    if (verbose) puts("STUB: DLConnect called");
    return NULL;
}

void* DLConnectToDeviceWithPort(void)
{
    if (verbose) puts("STUB: DLConnectToDeviceWithPort called");
    return NULL;
}

void* DLConnectToServiceOnDevice(void)
{
    if (verbose) puts("STUB: DLConnectToServiceOnDevice called");
    return NULL;
}

void* DLConnectToServiceOnDeviceWithOptions(void)
{
    if (verbose) puts("STUB: DLConnectToServiceOnDeviceWithOptions called");
    return NULL;
}

void* DLConnectionInfoCreateForEndpoint(void)
{
    if (verbose) puts("STUB: DLConnectionInfoCreateForEndpoint called");
    return NULL;
}

void* DLContentsOfDirectory(void)
{
    if (verbose) puts("STUB: DLContentsOfDirectory called");
    return NULL;
}

void* DLCopyConnectedDeviceArray(void)
{
    if (verbose) puts("STUB: DLCopyConnectedDeviceArray called");
    return NULL;
}

void* DLCopyFileAttributes(void)
{
    if (verbose) puts("STUB: DLCopyFileAttributes called");
    return NULL;
}

void* DLCopyHomeDirPath(void)
{
    if (verbose) puts("STUB: DLCopyHomeDirPath called");
    return NULL;
}

void* DLCopyItem(void)
{
    if (verbose) puts("STUB: DLCopyItem called");
    return NULL;
}

void* DLCopyTempDir(void)
{
    if (verbose) puts("STUB: DLCopyTempDir called");
    return NULL;
}

void* DLCreateCStringFromCFString(void)
{
    if (verbose) puts("STUB: DLCreateCStringFromCFString called");
    return NULL;
}

void* DLCreateCondition(void)
{
    if (verbose) puts("STUB: DLCreateCondition called");
    return NULL;
}

void* DLCreateDataFromString(void)
{
    if (verbose) puts("STUB: DLCreateDataFromString called");
    return NULL;
}

void* DLCreateDeviceLinkConnectionForComputer(void)
{
    if (verbose) puts("STUB: DLCreateDeviceLinkConnectionForComputer called");
    return NULL;
}

void* DLCreateDeviceLinkConnectionForDevice(void)
{
    if (verbose) puts("STUB: DLCreateDeviceLinkConnectionForDevice called");
    return NULL;
}

void* DLCreateDirectory(void)
{
    if (verbose) puts("STUB: DLCreateDirectory called");
    return NULL;
}

void* DLCreateMutex(void)
{
    if (verbose) puts("STUB: DLCreateMutex called");
    return NULL;
}

void* DLCreatePropertyListFromFile(void)
{
    if (verbose) puts("STUB: DLCreatePropertyListFromFile called");
    return NULL;
}

void* DLCreateStringFromData(void)
{
    if (verbose) puts("STUB: DLCreateStringFromData called");
    return NULL;
}

void* DLCreateUUID(void)
{
    if (verbose) puts("STUB: DLCreateUUID called");
    return NULL;
}

void* DLDataCreateWithMallocedBytesNoCopy(void)
{
    if (verbose) puts("STUB: DLDataCreateWithMallocedBytesNoCopy called");
    return NULL;
}

void* DLDeleteCondition(void)
{
    if (verbose) puts("STUB: DLDeleteCondition called");
    return NULL;
}

void* DLDeleteDeviceLinkConnection(void)
{
    if (verbose) puts("STUB: DLDeleteDeviceLinkConnection called");
    return NULL;
}

void* DLDeleteMutex(void)
{
    if (verbose) puts("STUB: DLDeleteMutex called");
    return NULL;
}

void* DLDeviceArrayCallbacks(void)
{
    if (verbose) puts("STUB: DLDeviceArrayCallbacks called");
    return NULL;
}

void* DLDeviceCopyAMDValue(void)
{
    if (verbose) puts("STUB: DLDeviceCopyAMDValue called");
    return NULL;
}

void* DLDeviceCreateDescription(void)
{
    if (verbose) puts("STUB: DLDeviceCreateDescription called");
    return NULL;
}

void* DLDeviceGetAMDevice(void)
{
    if (verbose) puts("STUB: DLDeviceGetAMDevice called");
    return NULL;
}

void* DLDeviceGetEndpointForType(void)
{
    if (verbose) puts("STUB: DLDeviceGetEndpointForType called");
    return NULL;
}

void* DLDeviceGetEndpoints(void)
{
    if (verbose) puts("STUB: DLDeviceGetEndpoints called");
    return NULL;
}

void* DLDeviceGetName(void)
{
    if (verbose) puts("STUB: DLDeviceGetName called");
    return NULL;
}

void* DLDeviceGetUDID(void)
{
    if (verbose) puts("STUB: DLDeviceGetUDID called");
    return NULL;
}

void* DLDeviceGetUID(void)
{
    if (verbose) puts("STUB: DLDeviceGetUID called");
    return NULL;
}

void* DLDeviceGetWithName(void)
{
    if (verbose) puts("STUB: DLDeviceGetWithName called");
    return NULL;
}

void* DLDeviceGetWithUDID(void)
{
    if (verbose) puts("STUB: DLDeviceGetWithUDID called");
    return NULL;
}

void* DLDeviceGetWithUID(void)
{
    if (verbose) puts("STUB: DLDeviceGetWithUID called");
    return NULL;
}

void* DLDeviceListenerCreate(void)
{
    if (verbose) puts("STUB: DLDeviceListenerCreate called");
    return NULL;
}

void* DLDeviceListenerCreateWithCallbacks(void)
{
    if (verbose) puts("STUB: DLDeviceListenerCreateWithCallbacks called");
    return NULL;
}

void* DLDeviceListenerDestroy(void)
{
    if (verbose) puts("STUB: DLDeviceListenerDestroy called");
    return NULL;
}

void* DLDeviceListenerSetAttachedCallback(void)
{
    if (verbose) puts("STUB: DLDeviceListenerSetAttachedCallback called");
    return NULL;
}

void* DLDeviceListenerSetContext(void)
{
    if (verbose) puts("STUB: DLDeviceListenerSetContext called");
    return NULL;
}

void* DLDeviceListenerSetDetachedCallback(void)
{
    if (verbose) puts("STUB: DLDeviceListenerSetDetachedCallback called");
    return NULL;
}

void* DLDeviceListenerSetStoppedCallback(void)
{
    if (verbose) puts("STUB: DLDeviceListenerSetStoppedCallback called");
    return NULL;
}

void* DLDevicePair(void)
{
    if (verbose) puts("STUB: DLDevicePair called");
    return NULL;
}

void* DLDeviceReady(void)
{
    if (verbose) puts("STUB: DLDeviceReady called");
    return NULL;
}

void* DLDeviceRelease(void)
{
    if (verbose) puts("STUB: DLDeviceRelease called");
    return NULL;
}

void* DLDeviceRetain(void)
{
    if (verbose) puts("STUB: DLDeviceRetain called");
    return NULL;
}

void* DLDeviceSetAMDValue(void)
{
    if (verbose) puts("STUB: DLDeviceSetAMDValue called");
    return NULL;
}

void* DLDeviceSetName(void)
{
    if (verbose) puts("STUB: DLDeviceSetName called");
    return NULL;
}

void* DLDeviceValidatePairing(void)
{
    if (verbose) puts("STUB: DLDeviceValidatePairing called");
    return NULL;
}

void* DLDeviceWaitForAttachedDevice(void)
{
    if (verbose) puts("STUB: DLDeviceWaitForAttachedDevice called");
    return NULL;
}

void* DLDisconnect(void)
{
    if (verbose) puts("STUB: DLDisconnect called");
    return NULL;
}

void* DLDownloadFile(void)
{
    if (verbose) puts("STUB: DLDownloadFile called");
    return NULL;
}

void* DLDownloadFiles(void)
{
    if (verbose) puts("STUB: DLDownloadFiles called");
    return NULL;
}

void* DLEndpointCreateDescription(void)
{
    if (verbose) puts("STUB: DLEndpointCreateDescription called");
    return NULL;
}

void* DLEnsureDirectoryExists(void)
{
    if (verbose) puts("STUB: DLEnsureDirectoryExists called");
    return NULL;
}

void* DLGetDeviceLinkConnectionContext(void)
{
    if (verbose) puts("STUB: DLGetDeviceLinkConnectionContext called");
    return NULL;
}

void* DLGetDeviceLinkConnectionInfo(void)
{
    if (verbose) puts("STUB: DLGetDeviceLinkConnectionInfo called");
    return NULL;
}

void* DLGetFileSystemRepresentation(void)
{
    if (verbose) puts("STUB: DLGetFileSystemRepresentation called");
    return NULL;
}

void* DLGetFlockForFile(void)
{
    if (verbose) puts("STUB: DLGetFlockForFile called");
    return NULL;
}

void* DLGetFlockForFileWithCancel(void)
{
    if (verbose) puts("STUB: DLGetFlockForFileWithCancel called");
    return NULL;
}

void* DLGetFreeDiskSpace(void)
{
    if (verbose) puts("STUB: DLGetFreeDiskSpace called");
    return NULL;
}

void* DLGetListenerSocketFromLaunchd(void)
{
    if (verbose) puts("STUB: DLGetListenerSocketFromLaunchd called");
    return NULL;
}

void* DLGetProcessName(void)
{
    if (verbose) puts("STUB: DLGetProcessName called");
    return NULL;
}

void* DLGetProgressPercentage(void)
{
    if (verbose) puts("STUB: DLGetProgressPercentage called");
    return NULL;
}

void* DLGetProgressSize(void)
{
    if (verbose) puts("STUB: DLGetProgressSize called");
    return NULL;
}

void* DLGetUniqueProcessName(void)
{
    if (verbose) puts("STUB: DLGetUniqueProcessName called");
    return NULL;
}

void* DLHandleSentFilePiece(void)
{
    if (verbose) puts("STUB: DLHandleSentFilePiece called");
    return NULL;
}

void* DLHandleSentFilePieceEncrypted(void)
{
    if (verbose) puts("STUB: DLHandleSentFilePieceEncrypted called");
    return NULL;
}

void* DLIsBlock(void)
{
    if (verbose) puts("STUB: DLIsBlock called");
    return NULL;
}

void* DLIsChar(void)
{
    if (verbose) puts("STUB: DLIsChar called");
    return NULL;
}

void* DLIsDir(void)
{
    if (verbose) puts("STUB: DLIsDir called");
    return NULL;
}

void* DLIsFIFO(void)
{
    if (verbose) puts("STUB: DLIsFIFO called");
    return NULL;
}

void* DLIsRegular(void)
{
    if (verbose) puts("STUB: DLIsRegular called");
    return NULL;
}

void* DLIsSocket(void)
{
    if (verbose) puts("STUB: DLIsSocket called");
    return NULL;
}

void* DLIsSymlink(void)
{
    if (verbose) puts("STUB: DLIsSymlink called");
    return NULL;
}

void* DLKillConnection(void)
{
    if (verbose) puts("STUB: DLKillConnection called");
    return NULL;
}

void* DLListenerCopyErrorString(void)
{
    if (verbose) puts("STUB: DLListenerCopyErrorString called");
    return NULL;
}

void* DLListenerGetErrorString(void)
{
    if (verbose) puts("STUB: DLListenerGetErrorString called");
    return NULL;
}

void* DLLock(void)
{
    if (verbose) puts("STUB: DLLock called");
    return NULL;
}

void* DLLoggingEnabled(void)
{
    if (verbose) puts("STUB: DLLoggingEnabled called");
    return NULL;
}

void* DLMemoryPoolAddObject(void)
{
    if (verbose) puts("STUB: DLMemoryPoolAddObject called");
    return NULL;
}

void* DLMemoryPoolCreate(void)
{
    if (verbose) puts("STUB: DLMemoryPoolCreate called");
    return NULL;
}

void* DLMoveItem(void)
{
    if (verbose) puts("STUB: DLMoveItem called");
    return NULL;
}

void* DLMoveItems(void)
{
    if (verbose) puts("STUB: DLMoveItems called");
    return NULL;
}

void* DLOutputEnabled(void)
{
    if (verbose) puts("STUB: DLOutputEnabled called");
    return NULL;
}

void* DLPing(void)
{
    if (verbose) puts("STUB: DLPing called");
    return NULL;
}

void* DLProcessMessage(void)
{
    if (verbose) puts("STUB: DLProcessMessage called");
    return NULL;
}

void* DLRegenerateUniqueProcessName(void)
{
    if (verbose) puts("STUB: DLRegenerateUniqueProcessName called");
    return NULL;
}

void* DLReleaseArgsAndReturn(void)
{
    if (verbose) puts("STUB: DLReleaseArgsAndReturn called");
    return NULL;
}

void* DLReleaseFlockForFile(void)
{
    if (verbose) puts("STUB: DLReleaseFlockForFile called");
    return NULL;
}

void* DLRemoveItem(void)
{
    if (verbose) puts("STUB: DLRemoveItem called");
    return NULL;
}

void* DLRemoveItems(void)
{
    if (verbose) puts("STUB: DLRemoveItems called");
    return NULL;
}

void* DLRequestFile(void)
{
    if (verbose) puts("STUB: DLRequestFile called");
    return NULL;
}

void* DLSendFile(void)
{
    if (verbose) puts("STUB: DLSendFile called");
    return NULL;
}

void* DLSetLogDirectoryName(void)
{
    if (verbose) puts("STUB: DLSetLogDirectoryName called");
    return NULL;
}

void* DLSetLogLevel(void)
{
    if (verbose) puts("STUB: DLSetLogLevel called");
    return NULL;
}

void* DLSetOutputLevel(void)
{
    if (verbose) puts("STUB: DLSetOutputLevel called");
    return NULL;
}

void* DLSetProcessName(void)
{
    if (verbose) puts("STUB: DLSetProcessName called");
    return NULL;
}

void* DLSetProgress(void)
{
    if (verbose) puts("STUB: DLSetProgress called");
    return NULL;
}

void* DLSetRootDirectory(void)
{
    if (verbose) puts("STUB: DLSetRootDirectory called");
    return NULL;
}

void* DLSetStatus(void)
{
    if (verbose) puts("STUB: DLSetStatus called");
    return NULL;
}

void* DLShouldLog(void)
{
    if (verbose) puts("STUB: DLShouldLog called");
    return NULL;
}

void* DLSignal(void)
{
    if (verbose) puts("STUB: DLSignal called");
    return NULL;
}

void* DLStatusFromOSStatus(void)
{
    if (verbose) puts("STUB: DLStatusFromOSStatus called");
    return NULL;
}

void* DLStringByAppendingPathComponent(void)
{
    if (verbose) puts("STUB: DLStringByAppendingPathComponent called");
    return NULL;
}

void* DLStringByRemovingLastPathComponent(void)
{
    if (verbose) puts("STUB: DLStringByRemovingLastPathComponent called");
    return NULL;
}

void* DLThreadAddMessagePortToRunloop(void)
{
    if (verbose) puts("STUB: DLThreadAddMessagePortToRunloop called");
    return NULL;
}

void* DLThreadCreateReceivePort(void)
{
    if (verbose) puts("STUB: DLThreadCreateReceivePort called");
    return NULL;
}

void* DLThreadCreateSendPort(void)
{
    if (verbose) puts("STUB: DLThreadCreateSendPort called");
    return NULL;
}

void* DLThreadCreateThread(void)
{
    if (verbose) puts("STUB: DLThreadCreateThread called");
    return NULL;
}

void* DLThreadDeleteReceivePort(void)
{
    if (verbose) puts("STUB: DLThreadDeleteReceivePort called");
    return NULL;
}

void* DLThreadDeleteSendPort(void)
{
    if (verbose) puts("STUB: DLThreadDeleteSendPort called");
    return NULL;
}

void* DLThreadRun(void)
{
    if (verbose) puts("STUB: DLThreadRun called");
    return NULL;
}

void* DLThreadSendMessage(void)
{
    if (verbose) puts("STUB: DLThreadSendMessage called");
    return NULL;
}

void* DLThreadSendSyncMessage(void)
{
    if (verbose) puts("STUB: DLThreadSendSyncMessage called");
    return NULL;
}

void* DLUnlock(void)
{
    if (verbose) puts("STUB: DLUnlock called");
    return NULL;
}

void* DLUploadFile(void)
{
    if (verbose) puts("STUB: DLUploadFile called");
    return NULL;
}

void* DLUploadFiles(void)
{
    if (verbose) puts("STUB: DLUploadFiles called");
    return NULL;
}

void* DLWait(void)
{
    if (verbose) puts("STUB: DLWait called");
    return NULL;
}

void* DLWaitForConnection(void)
{
    if (verbose) puts("STUB: DLWaitForConnection called");
    return NULL;
}

void* DLWaitForMessage(void)
{
    if (verbose) puts("STUB: DLWaitForMessage called");
    return NULL;
}

void* DLWritePropertyListToFile(void)
{
    if (verbose) puts("STUB: DLWritePropertyListToFile called");
    return NULL;
}

void* DLWritePropertyListToFileWithFormat(void)
{
    if (verbose) puts("STUB: DLWritePropertyListToFileWithFormat called");
    return NULL;
}

void* SetSocketLogCallback(void)
{
    if (verbose) puts("STUB: SetSocketLogCallback called");
    return NULL;
}

void* SocketAccept(void)
{
    if (verbose) puts("STUB: SocketAccept called");
    return NULL;
}

void* SocketConnect(void)
{
    if (verbose) puts("STUB: SocketConnect called");
    return NULL;
}

void* SocketConnectLocal(void)
{
    if (verbose) puts("STUB: SocketConnectLocal called");
    return NULL;
}

void* SocketConnectLockdownService(void)
{
    if (verbose) puts("STUB: SocketConnectLockdownService called");
    return NULL;
}

void* SocketCreateLocalServer(void)
{
    if (verbose) puts("STUB: SocketCreateLocalServer called");
    return NULL;
}

void* SocketCreateServer(void)
{
    if (verbose) puts("STUB: SocketCreateServer called");
    return NULL;
}

void* SocketCreateServerWithSocket(void)
{
    if (verbose) puts("STUB: SocketCreateServerWithSocket called");
    return NULL;
}

void* SocketDelete(void)
{
    if (verbose) puts("STUB: SocketDelete called");
    return NULL;
}

void* SocketRecv(void)
{
    if (verbose) puts("STUB: SocketRecv called");
    return NULL;
}

void* SocketSend(void)
{
    if (verbose) puts("STUB: SocketSend called");
    return NULL;
}

void* SocketStreamHandlerRegisterCreator(void)
{
    if (verbose) puts("STUB: SocketStreamHandlerRegisterCreator called");
    return NULL;
}

void* StreamHandlerAccept(void)
{
    if (verbose) puts("STUB: StreamHandlerAccept called");
    return NULL;
}

void* StreamHandlerClose(void)
{
    if (verbose) puts("STUB: StreamHandlerClose called");
    return NULL;
}

void* StreamHandlerConnect(void)
{
    if (verbose) puts("STUB: StreamHandlerConnect called");
    return NULL;
}

void* StreamHandlerDeleteStreamHandler(void)
{
    if (verbose) puts("STUB: StreamHandlerDeleteStreamHandler called");
    return NULL;
}

void* StreamHandlerForType(void)
{
    if (verbose) puts("STUB: StreamHandlerForType called");
    return NULL;
}

void* StreamHandlerReceive(void)
{
    if (verbose) puts("STUB: StreamHandlerReceive called");
    return NULL;
}

void* StreamHandlerRegisterCreatorForType(void)
{
    if (verbose) puts("STUB: StreamHandlerRegisterCreatorForType called");
    return NULL;
}

void* StreamHandlerSend(void)
{
    if (verbose) puts("STUB: StreamHandlerSend called");
    return NULL;
}

void* _DLBulkOperationContextAddResult(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextAddResult called");
    return NULL;
}

void* _DLBulkOperationContextCreateResult(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextCreateResult called");
    return NULL;
}

void* _DLBulkOperationContextDealloc(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextDealloc called");
    return NULL;
}

void* _DLBulkOperationContextInit(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextInit called");
    return NULL;
}

void* _DLBulkOperationContextMergeResults(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextMergeResults called");
    return NULL;
}

void* _DLBulkOperationContextMergeResultsCallback(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextMergeResultsCallback called");
    return NULL;
}

void* _DLBulkOperationContextSendStatusResponse(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextSendStatusResponse called");
    return NULL;
}

void* _DLBulkOperationContextSetStatus(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextSetStatus called");
    return NULL;
}

void* _DLBulkOperationContextShouldContinue(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextShouldContinue called");
    return NULL;
}

void* _DLBulkOperationContextUpdate(void)
{
    if (verbose) puts("STUB: _DLBulkOperationContextUpdate called");
    return NULL;
}

void* _DLBulkOperationSendChunk(void)
{
    if (verbose) puts("STUB: _DLBulkOperationSendChunk called");
    return NULL;
}

void* _DLBulkOperationSendFinalChunk(void)
{
    if (verbose) puts("STUB: _DLBulkOperationSendFinalChunk called");
    return NULL;
}

void* _DLCheckForDisconnect(void)
{
    if (verbose) puts("STUB: _DLCheckForDisconnect called");
    return NULL;
}

void* _DLContentsOfDirectoryOnComputerCallback(void)
{
    if (verbose) puts("STUB: _DLContentsOfDirectoryOnComputerCallback called");
    return NULL;
}

void* _DLCopyFileOnComputer(void)
{
    if (verbose) puts("STUB: _DLCopyFileOnComputer called");
    return NULL;
}

void* _DLCopyItemOnComputerCallback(void)
{
    if (verbose) puts("STUB: _DLCopyItemOnComputerCallback called");
    return NULL;
}

void* _DLCreateBulkOperationContextStatusResponseData(void)
{
    if (verbose) puts("STUB: _DLCreateBulkOperationContextStatusResponseData called");
    return NULL;
}

void* _DLCreateDeviceLinkConnectionForComputer(void)
{
    if (verbose) puts("STUB: _DLCreateDeviceLinkConnectionForComputer called");
    return NULL;
}

void* _DLCreateDeviceLinkConnectionForDevice(void)
{
    if (verbose) puts("STUB: _DLCreateDeviceLinkConnectionForDevice called");
    return NULL;
}

void* _DLCreateItemAttributes(void)
{
    if (verbose) puts("STUB: _DLCreateItemAttributes called");
    return NULL;
}

void* _DLCreateStatusResponseData(void)
{
    if (verbose) puts("STUB: _DLCreateStatusResponseData called");
    return NULL;
}

void* _DLDeviceCreateDescription(void)
{
    if (verbose) puts("STUB: _DLDeviceCreateDescription called");
    return NULL;
}

void* _DLDeviceGetDeviceIdentifier(void)
{
    if (verbose) puts("STUB: _DLDeviceGetDeviceIdentifier called");
    return NULL;
}

void* _DLDownloadFileFromComputer(void)
{
    if (verbose) puts("STUB: _DLDownloadFileFromComputer called");
    return NULL;
}

void* _DLDownloadFileToDevice(void)
{
    if (verbose) puts("STUB: _DLDownloadFileToDevice called");
    return NULL;
}

void* _DLEnumerateContentsOfDirectoryOnComputer(void)
{
    if (verbose) puts("STUB: _DLEnumerateContentsOfDirectoryOnComputer called");
    return NULL;
}

void* _DLGetParameterAtIndex(void)
{
    if (verbose) puts("STUB: _DLGetParameterAtIndex called");
    return NULL;
}

void* _DLGetParameterAtIndexWithDefaultValue(void)
{
    if (verbose) puts("STUB: _DLGetParameterAtIndexWithDefaultValue called");
    return NULL;
}

void* _DLGetStatusResponseFromData(void)
{
    if (verbose) puts("STUB: _DLGetStatusResponseFromData called");
    return NULL;
}

void* _DLHandlerThreadReadMessage(void)
{
    if (verbose) puts("STUB: _DLHandlerThreadReadMessage called");
    return NULL;
}

void* _DLHandlerThreadWriteMessage(void)
{
    if (verbose) puts("STUB: _DLHandlerThreadWriteMessage called");
    return NULL;
}

void* _DLLog(void)
{
    if (verbose) puts("STUB: _DLLog called");
    return NULL;
}

void* _DLMoveItemsOnComputerCallback(void)
{
    if (verbose) puts("STUB: _DLMoveItemsOnComputerCallback called");
    return NULL;
}

void* _DLOperationProgressContextInit(void)
{
    if (verbose) puts("STUB: _DLOperationProgressContextInit called");
    return NULL;
}

void* _DLOperationProgressContextUpdate(void)
{
    if (verbose) puts("STUB: _DLOperationProgressContextUpdate called");
    return NULL;
}

void* _DLProcessSyncMessageWithStatusResponse(void)
{
    if (verbose) puts("STUB: _DLProcessSyncMessageWithStatusResponse called");
    return NULL;
}

void* _DLProcessSyncMessageWithStatusResponseV(void)
{
    if (verbose) puts("STUB: _DLProcessSyncMessageWithStatusResponseV called");
    return NULL;
}

void* _DLReceiveFileForBulkOperation(void)
{
    if (verbose) puts("STUB: _DLReceiveFileForBulkOperation called");
    return NULL;
}

void* _DLRemoveDirectory(void)
{
    if (verbose) puts("STUB: _DLRemoveDirectory called");
    return NULL;
}

void* _DLRemoveItemOnComputerCallback(void)
{
    if (verbose) puts("STUB: _DLRemoveItemOnComputerCallback called");
    return NULL;
}

void* _DLRemoveItemUsingOldProtocolCallback(void)
{
    if (verbose) puts("STUB: _DLRemoveItemUsingOldProtocolCallback called");
    return NULL;
}

void* _DLRemoveItemsOnComputerCallback(void)
{
    if (verbose) puts("STUB: _DLRemoveItemsOnComputerCallback called");
    return NULL;
}

void* _DLRemoveItemsUsingOldProtocol(void)
{
    if (verbose) puts("STUB: _DLRemoveItemsUsingOldProtocol called");
    return NULL;
}

void* _DLRemoveItemsUsingOldProtocolCallback(void)
{
    if (verbose) puts("STUB: _DLRemoveItemsUsingOldProtocolCallback called");
    return NULL;
}

void* _DLRequestRead(void)
{
    if (verbose) puts("STUB: _DLRequestRead called");
    return NULL;
}

void* _DLRequestWrite(void)
{
    if (verbose) puts("STUB: _DLRequestWrite called");
    return NULL;
}

void* _DLRequestWriteV(void)
{
    if (verbose) puts("STUB: _DLRequestWriteV called");
    return NULL;
}

void* _DLResolveAndValidatePath(void)
{
    if (verbose) puts("STUB: _DLResolveAndValidatePath called");
    return NULL;
}

void* _DLSendFileForBulkOperation(void)
{
    if (verbose) puts("STUB: _DLSendFileForBulkOperation called");
    return NULL;
}

void* _DLSingleFromMultiError(void)
{
    if (verbose) puts("STUB: _DLSingleFromMultiError called");
    return NULL;
}

void* _DLThreadSendMessage(void)
{
    if (verbose) puts("STUB: _DLThreadSendMessage called");
    return NULL;
}

void* _DLUploadFileToComputer(void)
{
    if (verbose) puts("STUB: _DLUploadFileToComputer called");
    return NULL;
}

void* _DLUploadFilesFromDeviceCallback(void)
{
    if (verbose) puts("STUB: _DLUploadFilesFromDeviceCallback called");
    return NULL;
}

void* _DLWarnAboutUsingCopyHomeDirPathAndCallDLCopyHomeDirPath(void)
{
    if (verbose) puts("STUB: _DLWarnAboutUsingCopyHomeDirPathAndCallDLCopyHomeDirPath called");
    return NULL;
}

void* _RunLoopReadSourceCallback(void)
{
    if (verbose) puts("STUB: _RunLoopReadSourceCallback called");
    return NULL;
}

void* __DLCopyItemOnComputer(void)
{
    if (verbose) puts("STUB: __DLCopyItemOnComputer called");
    return NULL;
}

void* __DLCreateDirectoryOnComputer(void)
{
    if (verbose) puts("STUB: __DLCreateDirectoryOnComputer called");
    return NULL;
}

void* _deviceWithBetterEndpoint(void)
{
    if (verbose) puts("STUB: _deviceWithBetterEndpoint called");
    return NULL;
}

void* copyHomeDirPath(void)
{
    if (verbose) puts("STUB: copyHomeDirPath called");
    return NULL;
}

void* copyKeysAndValuesFromDictionary(void)
{
    if (verbose) puts("STUB: copyKeysAndValuesFromDictionary called");
    return NULL;
}

void* copyKeysAndValuesFromDictionaryNoRetain(void)
{
    if (verbose) puts("STUB: copyKeysAndValuesFromDictionaryNoRetain called");
    return NULL;
}

void* copyKeysFromDictionary(void)
{
    if (verbose) puts("STUB: copyKeysFromDictionary called");
    return NULL;
}

void* copyKeysFromDictionaryNoRetain(void)
{
    if (verbose) puts("STUB: copyKeysFromDictionaryNoRetain called");
    return NULL;
}

void* copyValuesFromDictionary(void)
{
    if (verbose) puts("STUB: copyValuesFromDictionary called");
    return NULL;
}

void* copyValuesFromDictionaryNoRetain(void)
{
    if (verbose) puts("STUB: copyValuesFromDictionaryNoRetain called");
    return NULL;
}

void* createDecryptedData(void)
{
    if (verbose) puts("STUB: createDecryptedData called");
    return NULL;
}

void* createEncryptedData(void)
{
    if (verbose) puts("STUB: createEncryptedData called");
    return NULL;
}

void* createFileAuthBlob(void)
{
    if (verbose) puts("STUB: createFileAuthBlob called");
    return NULL;
}

void* createFileKeyFromAuthBlob(void)
{
    if (verbose) puts("STUB: createFileKeyFromAuthBlob called");
    return NULL;
}

void* createRandomBytes(void)
{
    if (verbose) puts("STUB: createRandomBytes called");
    return NULL;
}

void* genericAcceptFailedCallback(void)
{
    if (verbose) puts("STUB: genericAcceptFailedCallback called");
    return NULL;
}

void* genericConnectionFailedCallback(void)
{
    if (verbose) puts("STUB: genericConnectionFailedCallback called");
    return NULL;
}

void* genericConnectionLostCallback(void)
{
    if (verbose) puts("STUB: genericConnectionLostCallback called");
    return NULL;
}

void* genericConnectionMadeCallback(void)
{
    if (verbose) puts("STUB: genericConnectionMadeCallback called");
    return NULL;
}

void* genericDisconnectCallback(void)
{
    if (verbose) puts("STUB: genericDisconnectCallback called");
    return NULL;
}

void* genericIncomingConnectionCallback(void)
{
    if (verbose) puts("STUB: genericIncomingConnectionCallback called");
    return NULL;
}

void* genericPingCallback(void)
{
    if (verbose) puts("STUB: genericPingCallback called");
    return NULL;
}

void* genericProcessMessageCallback(void)
{
    if (verbose) puts("STUB: genericProcessMessageCallback called");
    return NULL;
}

void* genericProgressCallback(void)
{
    if (verbose) puts("STUB: genericProgressCallback called");
    return NULL;
}

void* genericRequestFileCallback(void)
{
    if (verbose) puts("STUB: genericRequestFileCallback called");
    return NULL;
}

void* genericSendFileCallback(void)
{
    if (verbose) puts("STUB: genericSendFileCallback called");
    return NULL;
}

void* printFileTransferStatus(void)
{
    if (verbose) puts("STUB: printFileTransferStatus called");
    return NULL;
}
