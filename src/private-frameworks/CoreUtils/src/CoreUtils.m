/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#include <CoreUtils/CoreUtils.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AES_CBCFrame_Final(void)
{
    if (verbose) puts("STUB: AES_CBCFrame_Final called");
    return NULL;
}

void* AES_CBCFrame_Init(void)
{
    if (verbose) puts("STUB: AES_CBCFrame_Init called");
    return NULL;
}

void* AES_CBCFrame_Update(void)
{
    if (verbose) puts("STUB: AES_CBCFrame_Update called");
    return NULL;
}

void* AES_CTR_Final(void)
{
    if (verbose) puts("STUB: AES_CTR_Final called");
    return NULL;
}

void* AES_CTR_Init(void)
{
    if (verbose) puts("STUB: AES_CTR_Init called");
    return NULL;
}

void* AES_CTR_Update(void)
{
    if (verbose) puts("STUB: AES_CTR_Update called");
    return NULL;
}

void* AES_GCM_AddAAD(void)
{
    if (verbose) puts("STUB: AES_GCM_AddAAD called");
    return NULL;
}

void* AES_GCM_Decrypt(void)
{
    if (verbose) puts("STUB: AES_GCM_Decrypt called");
    return NULL;
}

void* AES_GCM_Encrypt(void)
{
    if (verbose) puts("STUB: AES_GCM_Encrypt called");
    return NULL;
}

void* AES_GCM_Final(void)
{
    if (verbose) puts("STUB: AES_GCM_Final called");
    return NULL;
}

void* AES_GCM_FinalizeMessage(void)
{
    if (verbose) puts("STUB: AES_GCM_FinalizeMessage called");
    return NULL;
}

void* AES_GCM_Init(void)
{
    if (verbose) puts("STUB: AES_GCM_Init called");
    return NULL;
}

void* AES_GCM_InitEx(void)
{
    if (verbose) puts("STUB: AES_GCM_InitEx called");
    return NULL;
}

void* AES_GCM_InitEx2(void)
{
    if (verbose) puts("STUB: AES_GCM_InitEx2 called");
    return NULL;
}

void* AES_GCM_InitMessage(void)
{
    if (verbose) puts("STUB: AES_GCM_InitMessage called");
    return NULL;
}

void* AES_GCM_InitMessageEx(void)
{
    if (verbose) puts("STUB: AES_GCM_InitMessageEx called");
    return NULL;
}

void* AES_GCM_VerifyMessage(void)
{
    if (verbose) puts("STUB: AES_GCM_VerifyMessage called");
    return NULL;
}

void* ASL_LOG(void)
{
    if (verbose) puts("STUB: ASL_LOG called");
    return NULL;
}

void* ASPrintF(void)
{
    if (verbose) puts("STUB: ASPrintF called");
    return NULL;
}

void* AWDLTrafficRegister(void)
{
    if (verbose) puts("STUB: AWDLTrafficRegister called");
    return NULL;
}

void* AppendPrintF(void)
{
    if (verbose) puts("STUB: AppendPrintF called");
    return NULL;
}

void* Apple80211Associate(void)
{
    if (verbose) puts("STUB: Apple80211Associate called");
    return NULL;
}

void* Apple80211Associate2(void)
{
    if (verbose) puts("STUB: Apple80211Associate2 called");
    return NULL;
}

void* Apple80211BindToInterface(void)
{
    if (verbose) puts("STUB: Apple80211BindToInterface called");
    return NULL;
}

void* Apple80211Close(void)
{
    if (verbose) puts("STUB: Apple80211Close called");
    return NULL;
}

void* Apple80211CopyAwdlPreferredChannels(void)
{
    if (verbose) puts("STUB: Apple80211CopyAwdlPreferredChannels called");
    return NULL;
}

void* Apple80211CopyLeakyAPStatus(void)
{
    if (verbose) puts("STUB: Apple80211CopyLeakyAPStatus called");
    return NULL;
}

void* Apple80211CopyRangingCapabilities(void)
{
    if (verbose) puts("STUB: Apple80211CopyRangingCapabilities called");
    return NULL;
}

void* Apple80211CopyValue(void)
{
    if (verbose) puts("STUB: Apple80211CopyValue called");
    return NULL;
}

void* Apple80211Disassociate(void)
{
    if (verbose) puts("STUB: Apple80211Disassociate called");
    return NULL;
}

void* Apple80211ErrToStr(void)
{
    if (verbose) puts("STUB: Apple80211ErrToStr called");
    return NULL;
}

void* Apple80211EventMonitoringHalt(void)
{
    if (verbose) puts("STUB: Apple80211EventMonitoringHalt called");
    return NULL;
}

void* Apple80211EventMonitoringInit(void)
{
    if (verbose) puts("STUB: Apple80211EventMonitoringInit called");
    return NULL;
}

void* Apple80211EventMonitoringInit2(void)
{
    if (verbose) puts("STUB: Apple80211EventMonitoringInit2 called");
    return NULL;
}

void* Apple80211EventSocketRef(void)
{
    if (verbose) puts("STUB: Apple80211EventSocketRef called");
    return NULL;
}

void* Apple80211GasRequest(void)
{
    if (verbose) puts("STUB: Apple80211GasRequest called");
    return NULL;
}

void* Apple80211Get(void)
{
    if (verbose) puts("STUB: Apple80211Get called");
    return NULL;
}

void* Apple80211GetIfListCopy(void)
{
    if (verbose) puts("STUB: Apple80211GetIfListCopy called");
    return NULL;
}

void* Apple80211GetInfoCopy(void)
{
    if (verbose) puts("STUB: Apple80211GetInfoCopy called");
    return NULL;
}

void* Apple80211GetInterfaceNameCopy(void)
{
    if (verbose) puts("STUB: Apple80211GetInterfaceNameCopy called");
    return NULL;
}

void* Apple80211GetPower(void)
{
    if (verbose) puts("STUB: Apple80211GetPower called");
    return NULL;
}

void* Apple80211GetRangingPhyErrorString(void)
{
    if (verbose) puts("STUB: Apple80211GetRangingPhyErrorString called");
    return NULL;
}

void* Apple80211GetVirtualIfListCopy(void)
{
    if (verbose) puts("STUB: Apple80211GetVirtualIfListCopy called");
    return NULL;
}

void* Apple80211MaxLinkSpeed(void)
{
    if (verbose) puts("STUB: Apple80211MaxLinkSpeed called");
    return NULL;
}

void* Apple80211MonitorEventsWithBlock(void)
{
    if (verbose) puts("STUB: Apple80211MonitorEventsWithBlock called");
    return NULL;
}

void* Apple80211Open(void)
{
    if (verbose) puts("STUB: Apple80211Open called");
    return NULL;
}

void* Apple80211Parse80211dIE(void)
{
    if (verbose) puts("STUB: Apple80211Parse80211dIE called");
    return NULL;
}

void* Apple80211ParseAppleDeviceIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseAppleDeviceIE called");
    return NULL;
}

void* Apple80211ParseAppleIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseAppleIE called");
    return NULL;
}

void* Apple80211ParseAppleP2PIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseAppleP2PIE called");
    return NULL;
}

void* Apple80211ParseAppleSWAPIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseAppleSWAPIE called");
    return NULL;
}

void* Apple80211ParseAppleiOSIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseAppleiOSIE called");
    return NULL;
}

void* Apple80211ParseDSIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseDSIE called");
    return NULL;
}

void* Apple80211ParseExtCapsIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseExtCapsIE called");
    return NULL;
}

void* Apple80211ParseHS20IE(void)
{
    if (verbose) puts("STUB: Apple80211ParseHS20IE called");
    return NULL;
}

void* Apple80211ParseHT_Caps(void)
{
    if (verbose) puts("STUB: Apple80211ParseHT_Caps called");
    return NULL;
}

void* Apple80211ParseHT_IE(void)
{
    if (verbose) puts("STUB: Apple80211ParseHT_IE called");
    return NULL;
}

void* Apple80211ParseInterworkingIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseInterworkingIE called");
    return NULL;
}

void* Apple80211ParseMobilityDomainIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseMobilityDomainIE called");
    return NULL;
}

void* Apple80211ParseP2PIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseP2PIE called");
    return NULL;
}

void* Apple80211ParseQBSSLoadIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseQBSSLoadIE called");
    return NULL;
}

void* Apple80211ParseRSNIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseRSNIE called");
    return NULL;
}

void* Apple80211ParseSES_IE(void)
{
    if (verbose) puts("STUB: Apple80211ParseSES_IE called");
    return NULL;
}

void* Apple80211ParseSSIDIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseSSIDIE called");
    return NULL;
}

void* Apple80211ParseVHT_Caps(void)
{
    if (verbose) puts("STUB: Apple80211ParseVHT_Caps called");
    return NULL;
}

void* Apple80211ParseVhtCapsIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseVhtCapsIE called");
    return NULL;
}

void* Apple80211ParseVhtOpIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseVhtOpIE called");
    return NULL;
}

void* Apple80211ParseWPAIE(void)
{
    if (verbose) puts("STUB: Apple80211ParseWPAIE called");
    return NULL;
}

void* Apple80211ParseWPAPassword(void)
{
    if (verbose) puts("STUB: Apple80211ParseWPAPassword called");
    return NULL;
}

void* Apple80211ParseWPS_IE(void)
{
    if (verbose) puts("STUB: Apple80211ParseWPS_IE called");
    return NULL;
}

void* Apple80211RangeAsync(void)
{
    if (verbose) puts("STUB: Apple80211RangeAsync called");
    return NULL;
}

void* Apple80211Scan(void)
{
    if (verbose) puts("STUB: Apple80211Scan called");
    return NULL;
}

void* Apple80211ScanAsync(void)
{
    if (verbose) puts("STUB: Apple80211ScanAsync called");
    return NULL;
}

void* Apple80211ScanDynamic(void)
{
    if (verbose) puts("STUB: Apple80211ScanDynamic called");
    return NULL;
}

void* Apple80211Set(void)
{
    if (verbose) puts("STUB: Apple80211Set called");
    return NULL;
}

void* Apple80211SetPower(void)
{
    if (verbose) puts("STUB: Apple80211SetPower called");
    return NULL;
}

void* Apple80211StartMonitoringEvent(void)
{
    if (verbose) puts("STUB: Apple80211StartMonitoringEvent called");
    return NULL;
}

void* Apple80211StopMonitoringEvent(void)
{
    if (verbose) puts("STUB: Apple80211StopMonitoringEvent called");
    return NULL;
}

void* AppleAllocInitialReqFrm(void)
{
    if (verbose) puts("STUB: AppleAllocInitialReqFrm called");
    return NULL;
}

void* AppleParseGASInitialRequestFrame(void)
{
    if (verbose) puts("STUB: AppleParseGASInitialRequestFrame called");
    return NULL;
}

void* AsyncConnection_Connect(void)
{
    if (verbose) puts("STUB: AsyncConnection_Connect called");
    return NULL;
}

void* AsyncConnection_ConnectEx(void)
{
    if (verbose) puts("STUB: AsyncConnection_ConnectEx called");
    return NULL;
}

void* AsyncConnection_ConnectSync(void)
{
    if (verbose) puts("STUB: AsyncConnection_ConnectSync called");
    return NULL;
}

void* AsyncConnection_ConnectSyncEx(void)
{
    if (verbose) puts("STUB: AsyncConnection_ConnectSyncEx called");
    return NULL;
}

void* AsyncConnection_Release(void)
{
    if (verbose) puts("STUB: AsyncConnection_Release called");
    return NULL;
}

void* AsyncOperationQueueComplete(void)
{
    if (verbose) puts("STUB: AsyncOperationQueueComplete called");
    return NULL;
}

void* AsyncOperationQueueCreate(void)
{
    if (verbose) puts("STUB: AsyncOperationQueueCreate called");
    return NULL;
}

void* AsyncOperationQueueSchedule_b(void)
{
    if (verbose) puts("STUB: AsyncOperationQueueSchedule_b called");
    return NULL;
}

void* AsyncOperationQueueSetDispatchQueue(void)
{
    if (verbose) puts("STUB: AsyncOperationQueueSetDispatchQueue called");
    return NULL;
}

void* AudioStreamCreate(void)
{
    if (verbose) puts("STUB: AudioStreamCreate called");
    return NULL;
}

void* AudioStreamGetTypeID(void)
{
    if (verbose) puts("STUB: AudioStreamGetTypeID called");
    return NULL;
}

void* AudioStreamPrepare(void)
{
    if (verbose) puts("STUB: AudioStreamPrepare called");
    return NULL;
}

void* AudioStreamRampVolume(void)
{
    if (verbose) puts("STUB: AudioStreamRampVolume called");
    return NULL;
}

void* AudioStreamSetInputCallback(void)
{
    if (verbose) puts("STUB: AudioStreamSetInputCallback called");
    return NULL;
}

void* AudioStreamSetOutputCallback(void)
{
    if (verbose) puts("STUB: AudioStreamSetOutputCallback called");
    return NULL;
}

void* AudioStreamStart(void)
{
    if (verbose) puts("STUB: AudioStreamStart called");
    return NULL;
}

void* AudioStreamStop(void)
{
    if (verbose) puts("STUB: AudioStreamStop called");
    return NULL;
}

void* AudioStreamTest(void)
{
    if (verbose) puts("STUB: AudioStreamTest called");
    return NULL;
}

void* Base64Decode(void)
{
    if (verbose) puts("STUB: Base64Decode called");
    return NULL;
}

void* Base64DecodeCopy(void)
{
    if (verbose) puts("STUB: Base64DecodeCopy called");
    return NULL;
}

void* Base64EncodeCopyEx(void)
{
    if (verbose) puts("STUB: Base64EncodeCopyEx called");
    return NULL;
}

void* Base64EncodeEx(void)
{
    if (verbose) puts("STUB: Base64EncodeEx called");
    return NULL;
}

void* Base64EncodeLinesCopyEx(void)
{
    if (verbose) puts("STUB: Base64EncodeLinesCopyEx called");
    return NULL;
}

void* BitListString_Make(void)
{
    if (verbose) puts("STUB: BitListString_Make called");
    return NULL;
}

void* BitListString_Parse(void)
{
    if (verbose) puts("STUB: BitListString_Parse called");
    return NULL;
}

void* BonjourAdvertiserCreate(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserCreate called");
    return NULL;
}

void* BonjourAdvertiserSetDispatchQueue(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetDispatchQueue called");
    return NULL;
}

void* BonjourAdvertiserSetDomain(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetDomain called");
    return NULL;
}

void* BonjourAdvertiserSetFlags(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetFlags called");
    return NULL;
}

void* BonjourAdvertiserSetInterfaceIndex(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetInterfaceIndex called");
    return NULL;
}

void* BonjourAdvertiserSetInterfaceName(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetInterfaceName called");
    return NULL;
}

void* BonjourAdvertiserSetLabel(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetLabel called");
    return NULL;
}

void* BonjourAdvertiserSetName(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetName called");
    return NULL;
}

void* BonjourAdvertiserSetP2P(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetP2P called");
    return NULL;
}

void* BonjourAdvertiserSetPort(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetPort called");
    return NULL;
}

void* BonjourAdvertiserSetServiceType(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetServiceType called");
    return NULL;
}

void* BonjourAdvertiserSetTXTRecord(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetTXTRecord called");
    return NULL;
}

void* BonjourAdvertiserSetTrafficFlags(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserSetTrafficFlags called");
    return NULL;
}

void* BonjourAdvertiserStart(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserStart called");
    return NULL;
}

void* BonjourAdvertiserStop(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserStop called");
    return NULL;
}

void* BonjourAdvertiserUpdate(void)
{
    if (verbose) puts("STUB: BonjourAdvertiserUpdate called");
    return NULL;
}

void* BonjourBrowser_CopyDevices(void)
{
    if (verbose) puts("STUB: BonjourBrowser_CopyDevices called");
    return NULL;
}

void* BonjourBrowser_Create(void)
{
    if (verbose) puts("STUB: BonjourBrowser_Create called");
    return NULL;
}

void* BonjourBrowser_CreateEx(void)
{
    if (verbose) puts("STUB: BonjourBrowser_CreateEx called");
    return NULL;
}

void* BonjourBrowser_ReconfirmDevice(void)
{
    if (verbose) puts("STUB: BonjourBrowser_ReconfirmDevice called");
    return NULL;
}

void* BonjourBrowser_ReconfirmDeviceEx(void)
{
    if (verbose) puts("STUB: BonjourBrowser_ReconfirmDeviceEx called");
    return NULL;
}

void* BonjourBrowser_SetDispatchQueue(void)
{
    if (verbose) puts("STUB: BonjourBrowser_SetDispatchQueue called");
    return NULL;
}

void* BonjourBrowser_SetEventHandler(void)
{
    if (verbose) puts("STUB: BonjourBrowser_SetEventHandler called");
    return NULL;
}

void* BonjourBrowser_SetEventHandlerBlock(void)
{
    if (verbose) puts("STUB: BonjourBrowser_SetEventHandlerBlock called");
    return NULL;
}

void* BonjourBrowser_Start(void)
{
    if (verbose) puts("STUB: BonjourBrowser_Start called");
    return NULL;
}

void* BonjourBrowser_Stop(void)
{
    if (verbose) puts("STUB: BonjourBrowser_Stop called");
    return NULL;
}

void* BonjourDevice_CopyCFString(void)
{
    if (verbose) puts("STUB: BonjourDevice_CopyCFString called");
    return NULL;
}

void* BonjourDevice_CopyConnectionInfo(void)
{
    if (verbose) puts("STUB: BonjourDevice_CopyConnectionInfo called");
    return NULL;
}

void* BonjourDevice_CopyDNSNames(void)
{
    if (verbose) puts("STUB: BonjourDevice_CopyDNSNames called");
    return NULL;
}

void* BonjourDevice_GetBitListValue(void)
{
    if (verbose) puts("STUB: BonjourDevice_GetBitListValue called");
    return NULL;
}

void* BonjourDevice_GetDNSName(void)
{
    if (verbose) puts("STUB: BonjourDevice_GetDNSName called");
    return NULL;
}

void* BonjourDevice_GetDeviceID(void)
{
    if (verbose) puts("STUB: BonjourDevice_GetDeviceID called");
    return NULL;
}

void* BonjourDevice_GetInt64(void)
{
    if (verbose) puts("STUB: BonjourDevice_GetInt64 called");
    return NULL;
}

void* BonjourDevice_MergeInfo(void)
{
    if (verbose) puts("STUB: BonjourDevice_MergeInfo called");
    return NULL;
}

void* BonjourDevice_Reconfirm(void)
{
    if (verbose) puts("STUB: BonjourDevice_Reconfirm called");
    return NULL;
}

void* BonjourDevice_RemoveInterfaceInfo(void)
{
    if (verbose) puts("STUB: BonjourDevice_RemoveInterfaceInfo called");
    return NULL;
}

void* CADiffStats(void)
{
    if (verbose) puts("STUB: CADiffStats called");
    return NULL;
}

void* CAGetStats(void)
{
    if (verbose) puts("STUB: CAGetStats called");
    return NULL;
}

void* CFAppendPrintF(void)
{
    if (verbose) puts("STUB: CFAppendPrintF called");
    return NULL;
}

void* CFAppendPrintV(void)
{
    if (verbose) puts("STUB: CFAppendPrintV called");
    return NULL;
}

void* CFArrayAppendBytes(void)
{
    if (verbose) puts("STUB: CFArrayAppendBytes called");
    return NULL;
}

void* CFArrayAppendInt64(void)
{
    if (verbose) puts("STUB: CFArrayAppendInt64 called");
    return NULL;
}

void* CFArrayApplyBlock(void)
{
    if (verbose) puts("STUB: CFArrayApplyBlock called");
    return NULL;
}

void* CFArrayEnsureCreatedAndAppend(void)
{
    if (verbose) puts("STUB: CFArrayEnsureCreatedAndAppend called");
    return NULL;
}

void* CFArrayEnsureCreatedAndAppendCString(void)
{
    if (verbose) puts("STUB: CFArrayEnsureCreatedAndAppendCString called");
    return NULL;
}

void* CFArrayGetTypedValueAtIndex(void)
{
    if (verbose) puts("STUB: CFArrayGetTypedValueAtIndex called");
    return NULL;
}

void* CFBinaryPlistStreamedCreateData(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedCreateData called");
    return NULL;
}

void* CFBinaryPlistStreamedCreateDataEx(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedCreateDataEx called");
    return NULL;
}

void* CFBinaryPlistStreamedCreateWithBytes(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedCreateWithBytes called");
    return NULL;
}

void* CFBinaryPlistStreamedCreateWithBytesEx(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedCreateWithBytesEx called");
    return NULL;
}

void* CFBinaryPlistStreamedCreateWithBytesEx2(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedCreateWithBytesEx2 called");
    return NULL;
}

void* CFBinaryPlistStreamedCreateWithData(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedCreateWithData called");
    return NULL;
}

void* CFBinaryPlistStreamedWriteObject(void)
{
    if (verbose) puts("STUB: CFBinaryPlistStreamedWriteObject called");
    return NULL;
}

void* CFCopyCString(void)
{
    if (verbose) puts("STUB: CFCopyCString called");
    return NULL;
}

void* CFCreateF(void)
{
    if (verbose) puts("STUB: CFCreateF called");
    return NULL;
}

void* CFCreateJSONData(void)
{
    if (verbose) puts("STUB: CFCreateJSONData called");
    return NULL;
}

void* CFCreateObjectFromString(void)
{
    if (verbose) puts("STUB: CFCreateObjectFromString called");
    return NULL;
}

void* CFCreateUUIDString(void)
{
    if (verbose) puts("STUB: CFCreateUUIDString called");
    return NULL;
}

void* CFCreateV(void)
{
    if (verbose) puts("STUB: CFCreateV called");
    return NULL;
}

void* CFCreateWithJSONBytes(void)
{
    if (verbose) puts("STUB: CFCreateWithJSONBytes called");
    return NULL;
}

void* CFCreateWithPlistBytes(void)
{
    if (verbose) puts("STUB: CFCreateWithPlistBytes called");
    return NULL;
}

void* CFDataCreateSubdataWithRangeNoCopy(void)
{
    if (verbose) puts("STUB: CFDataCreateSubdataWithRangeNoCopy called");
    return NULL;
}

void* CFDataCreateWithANSIFile(void)
{
    if (verbose) puts("STUB: CFDataCreateWithANSIFile called");
    return NULL;
}

void* CFDataCreateWithFilePath(void)
{
    if (verbose) puts("STUB: CFDataCreateWithFilePath called");
    return NULL;
}

void* CFDataGetEmpty(void)
{
    if (verbose) puts("STUB: CFDataGetEmpty called");
    return NULL;
}

void* CFDateCreateWithComponents(void)
{
    if (verbose) puts("STUB: CFDateCreateWithComponents called");
    return NULL;
}

void* CFDateGetComponents(void)
{
    if (verbose) puts("STUB: CFDateGetComponents called");
    return NULL;
}

void* CFDictionaryApplyBlock(void)
{
    if (verbose) puts("STUB: CFDictionaryApplyBlock called");
    return NULL;
}

void* CFDictionaryCopyCString(void)
{
    if (verbose) puts("STUB: CFDictionaryCopyCString called");
    return NULL;
}

void* CFDictionaryCopyKeys(void)
{
    if (verbose) puts("STUB: CFDictionaryCopyKeys called");
    return NULL;
}

void* CFDictionaryCreateFromNameTypeValueArgList(void)
{
    if (verbose) puts("STUB: CFDictionaryCreateFromNameTypeValueArgList called");
    return NULL;
}

void* CFDictionaryCreateWithINIBytes(void)
{
    if (verbose) puts("STUB: CFDictionaryCreateWithINIBytes called");
    return NULL;
}

void* CFDictionaryCreateWithTXTRecordBytes(void)
{
    if (verbose) puts("STUB: CFDictionaryCreateWithTXTRecordBytes called");
    return NULL;
}

void* CFDictionaryGetCFDataOfLength(void)
{
    if (verbose) puts("STUB: CFDictionaryGetCFDataOfLength called");
    return NULL;
}

void* CFDictionaryGetCGRect(void)
{
    if (verbose) puts("STUB: CFDictionaryGetCGRect called");
    return NULL;
}

void* CFDictionaryGetCGSize(void)
{
    if (verbose) puts("STUB: CFDictionaryGetCGSize called");
    return NULL;
}

void* CFDictionaryGetCString(void)
{
    if (verbose) puts("STUB: CFDictionaryGetCString called");
    return NULL;
}

void* CFDictionaryGetData(void)
{
    if (verbose) puts("STUB: CFDictionaryGetData called");
    return NULL;
}

void* CFDictionaryGetDouble(void)
{
    if (verbose) puts("STUB: CFDictionaryGetDouble called");
    return NULL;
}

void* CFDictionaryGetEmpty(void)
{
    if (verbose) puts("STUB: CFDictionaryGetEmpty called");
    return NULL;
}

void* CFDictionaryGetHardwareAddress(void)
{
    if (verbose) puts("STUB: CFDictionaryGetHardwareAddress called");
    return NULL;
}

void* CFDictionaryGetInt64(void)
{
    if (verbose) puts("STUB: CFDictionaryGetInt64 called");
    return NULL;
}

void* CFDictionaryGetInt64Ranged(void)
{
    if (verbose) puts("STUB: CFDictionaryGetInt64Ranged called");
    return NULL;
}

void* CFDictionaryGetTypedValue(void)
{
    if (verbose) puts("STUB: CFDictionaryGetTypedValue called");
    return NULL;
}

void* CFDictionaryGetUUIDEx(void)
{
    if (verbose) puts("STUB: CFDictionaryGetUUIDEx called");
    return NULL;
}

void* CFDictionaryMergeDictionary(void)
{
    if (verbose) puts("STUB: CFDictionaryMergeDictionary called");
    return NULL;
}

void* CFDictionarySetCGRect(void)
{
    if (verbose) puts("STUB: CFDictionarySetCGRect called");
    return NULL;
}

void* CFDictionarySetCGSize(void)
{
    if (verbose) puts("STUB: CFDictionarySetCGSize called");
    return NULL;
}

void* CFDictionarySetCString(void)
{
    if (verbose) puts("STUB: CFDictionarySetCString called");
    return NULL;
}

void* CFDictionarySetData(void)
{
    if (verbose) puts("STUB: CFDictionarySetData called");
    return NULL;
}

void* CFDictionarySetDouble(void)
{
    if (verbose) puts("STUB: CFDictionarySetDouble called");
    return NULL;
}

void* CFDictionarySetHardwareAddress(void)
{
    if (verbose) puts("STUB: CFDictionarySetHardwareAddress called");
    return NULL;
}

void* CFDictionarySetInt64(void)
{
    if (verbose) puts("STUB: CFDictionarySetInt64 called");
    return NULL;
}

void* CFDictionarySetNumber(void)
{
    if (verbose) puts("STUB: CFDictionarySetNumber called");
    return NULL;
}

void* CFDictionarySetUUIDString(void)
{
    if (verbose) puts("STUB: CFDictionarySetUUIDString called");
    return NULL;
}

void* CFGetCString(void)
{
    if (verbose) puts("STUB: CFGetCString called");
    return NULL;
}

void* CFGetData(void)
{
    if (verbose) puts("STUB: CFGetData called");
    return NULL;
}

void* CFGetDouble(void)
{
    if (verbose) puts("STUB: CFGetDouble called");
    return NULL;
}

void* CFGetHardwareAddress(void)
{
    if (verbose) puts("STUB: CFGetHardwareAddress called");
    return NULL;
}

void* CFGetInt64(void)
{
    if (verbose) puts("STUB: CFGetInt64 called");
    return NULL;
}

void* CFGetInt64Ranged(void)
{
    if (verbose) puts("STUB: CFGetInt64Ranged called");
    return NULL;
}

void* CFGetTypedObject(void)
{
    if (verbose) puts("STUB: CFGetTypedObject called");
    return NULL;
}

void* CFNumberCreateInt64(void)
{
    if (verbose) puts("STUB: CFNumberCreateInt64 called");
    return NULL;
}

void* CFNumberGetObject(void)
{
    if (verbose) puts("STUB: CFNumberGetObject called");
    return NULL;
}

void* CFObjectControlAsync(void)
{
    if (verbose) puts("STUB: CFObjectControlAsync called");
    return NULL;
}

void* CFObjectControlAsyncF(void)
{
    if (verbose) puts("STUB: CFObjectControlAsyncF called");
    return NULL;
}

void* CFObjectControlSync(void)
{
    if (verbose) puts("STUB: CFObjectControlSync called");
    return NULL;
}

void* CFObjectControlSyncF(void)
{
    if (verbose) puts("STUB: CFObjectControlSyncF called");
    return NULL;
}

void* CFObjectCopyBytes(void)
{
    if (verbose) puts("STUB: CFObjectCopyBytes called");
    return NULL;
}

void* CFObjectCopyCString(void)
{
    if (verbose) puts("STUB: CFObjectCopyCString called");
    return NULL;
}

void* CFObjectCopyProperty(void)
{
    if (verbose) puts("STUB: CFObjectCopyProperty called");
    return NULL;
}

void* CFObjectCopyTypedValue(void)
{
    if (verbose) puts("STUB: CFObjectCopyTypedValue called");
    return NULL;
}

void* CFObjectGetBytes(void)
{
    if (verbose) puts("STUB: CFObjectGetBytes called");
    return NULL;
}

void* CFObjectGetCString(void)
{
    if (verbose) puts("STUB: CFObjectGetCString called");
    return NULL;
}

void* CFObjectGetDouble(void)
{
    if (verbose) puts("STUB: CFObjectGetDouble called");
    return NULL;
}

void* CFObjectGetHardwareAddress(void)
{
    if (verbose) puts("STUB: CFObjectGetHardwareAddress called");
    return NULL;
}

void* CFObjectGetInt64(void)
{
    if (verbose) puts("STUB: CFObjectGetInt64 called");
    return NULL;
}

void* CFObjectGetInt64Ranged(void)
{
    if (verbose) puts("STUB: CFObjectGetInt64Ranged called");
    return NULL;
}

void* CFObjectGetPropertyCStringSync(void)
{
    if (verbose) puts("STUB: CFObjectGetPropertyCStringSync called");
    return NULL;
}

void* CFObjectGetPropertyDoubleSync(void)
{
    if (verbose) puts("STUB: CFObjectGetPropertyDoubleSync called");
    return NULL;
}

void* CFObjectGetPropertyInt64Sync(void)
{
    if (verbose) puts("STUB: CFObjectGetPropertyInt64Sync called");
    return NULL;
}

void* CFObjectGetUUID(void)
{
    if (verbose) puts("STUB: CFObjectGetUUID called");
    return NULL;
}

void* CFObjectSetBytes(void)
{
    if (verbose) puts("STUB: CFObjectSetBytes called");
    return NULL;
}

void* CFObjectSetCString(void)
{
    if (verbose) puts("STUB: CFObjectSetCString called");
    return NULL;
}

void* CFObjectSetDouble(void)
{
    if (verbose) puts("STUB: CFObjectSetDouble called");
    return NULL;
}

void* CFObjectSetHardwareAddress(void)
{
    if (verbose) puts("STUB: CFObjectSetHardwareAddress called");
    return NULL;
}

void* CFObjectSetInt64(void)
{
    if (verbose) puts("STUB: CFObjectSetInt64 called");
    return NULL;
}

void* CFObjectSetProperty(void)
{
    if (verbose) puts("STUB: CFObjectSetProperty called");
    return NULL;
}

void* CFObjectSetPropertyCString(void)
{
    if (verbose) puts("STUB: CFObjectSetPropertyCString called");
    return NULL;
}

void* CFObjectSetPropertyData(void)
{
    if (verbose) puts("STUB: CFObjectSetPropertyData called");
    return NULL;
}

void* CFObjectSetPropertyDouble(void)
{
    if (verbose) puts("STUB: CFObjectSetPropertyDouble called");
    return NULL;
}

void* CFObjectSetPropertyF(void)
{
    if (verbose) puts("STUB: CFObjectSetPropertyF called");
    return NULL;
}

void* CFObjectSetPropertyInt64(void)
{
    if (verbose) puts("STUB: CFObjectSetPropertyInt64 called");
    return NULL;
}

void* CFObjectSetUUID(void)
{
    if (verbose) puts("STUB: CFObjectSetUUID called");
    return NULL;
}

void* CFObjectSetUUIDString(void)
{
    if (verbose) puts("STUB: CFObjectSetUUIDString called");
    return NULL;
}

void* CFObjectSetValue(void)
{
    if (verbose) puts("STUB: CFObjectSetValue called");
    return NULL;
}

void* CFPrefs_CopyKeys(void)
{
    if (verbose) puts("STUB: CFPrefs_CopyKeys called");
    return NULL;
}

void* CFPrefs_CopyTypedValue(void)
{
    if (verbose) puts("STUB: CFPrefs_CopyTypedValue called");
    return NULL;
}

void* CFPrefs_GetCString(void)
{
    if (verbose) puts("STUB: CFPrefs_GetCString called");
    return NULL;
}

void* CFPrefs_GetData(void)
{
    if (verbose) puts("STUB: CFPrefs_GetData called");
    return NULL;
}

void* CFPrefs_GetDouble(void)
{
    if (verbose) puts("STUB: CFPrefs_GetDouble called");
    return NULL;
}

void* CFPrefs_GetInt64(void)
{
    if (verbose) puts("STUB: CFPrefs_GetInt64 called");
    return NULL;
}

void* CFPrefs_RemoveValue(void)
{
    if (verbose) puts("STUB: CFPrefs_RemoveValue called");
    return NULL;
}

void* CFPrefs_SetCString(void)
{
    if (verbose) puts("STUB: CFPrefs_SetCString called");
    return NULL;
}

void* CFPrefs_SetDouble(void)
{
    if (verbose) puts("STUB: CFPrefs_SetDouble called");
    return NULL;
}

void* CFPrefs_SetInt64(void)
{
    if (verbose) puts("STUB: CFPrefs_SetInt64 called");
    return NULL;
}

void* CFPrefs_SetValue(void)
{
    if (verbose) puts("STUB: CFPrefs_SetValue called");
    return NULL;
}

void* CFPropertyListAppendFormatted(void)
{
    if (verbose) puts("STUB: CFPropertyListAppendFormatted called");
    return NULL;
}

void* CFPropertyListCreateBytes(void)
{
    if (verbose) puts("STUB: CFPropertyListCreateBytes called");
    return NULL;
}

void* CFPropertyListCreateFormatted(void)
{
    if (verbose) puts("STUB: CFPropertyListCreateFormatted called");
    return NULL;
}

void* CFPropertyListCreateFormattedVAList(void)
{
    if (verbose) puts("STUB: CFPropertyListCreateFormattedVAList called");
    return NULL;
}

void* CFPropertyListCreateFromFilePath(void)
{
    if (verbose) puts("STUB: CFPropertyListCreateFromFilePath called");
    return NULL;
}

void* CFPropertyListExtractFormatted(void)
{
    if (verbose) puts("STUB: CFPropertyListExtractFormatted called");
    return NULL;
}

void* CFPropertyListWriteToFilePath(void)
{
    if (verbose) puts("STUB: CFPropertyListWriteToFilePath called");
    return NULL;
}

void* CFRunLoopRunEx(void)
{
    if (verbose) puts("STUB: CFRunLoopRunEx called");
    return NULL;
}

void* CFRunLoopRunFor(void)
{
    if (verbose) puts("STUB: CFRunLoopRunFor called");
    return NULL;
}

void* CFRuntimeCreateObject(void)
{
    if (verbose) puts("STUB: CFRuntimeCreateObject called");
    return NULL;
}

void* CFSetApplyBlock(void)
{
    if (verbose) puts("STUB: CFSetApplyBlock called");
    return NULL;
}

void* CFSetObjectAtPath(void)
{
    if (verbose) puts("STUB: CFSetObjectAtPath called");
    return NULL;
}

void* CFSortCompareKeyPath(void)
{
    if (verbose) puts("STUB: CFSortCompareKeyPath called");
    return NULL;
}

void* CFSortLocalizedStandardCompare(void)
{
    if (verbose) puts("STUB: CFSortLocalizedStandardCompare called");
    return NULL;
}

void* CFStringAppendF(void)
{
    if (verbose) puts("STUB: CFStringAppendF called");
    return NULL;
}

void* CFStringAppendV(void)
{
    if (verbose) puts("STUB: CFStringAppendV called");
    return NULL;
}

void* CFStringCopyUTF8CString(void)
{
    if (verbose) puts("STUB: CFStringCopyUTF8CString called");
    return NULL;
}

void* CFStringCreateF(void)
{
    if (verbose) puts("STUB: CFStringCreateF called");
    return NULL;
}

void* CFStringCreateV(void)
{
    if (verbose) puts("STUB: CFStringCreateV called");
    return NULL;
}

void* CFStringGetOrCopyCStringUTF8(void)
{
    if (verbose) puts("STUB: CFStringGetOrCopyCStringUTF8 called");
    return NULL;
}

void* CGColorCreateWithRGB(void)
{
    if (verbose) puts("STUB: CGColorCreateWithRGB called");
    return NULL;
}

void* CGColorGetBlack(void)
{
    if (verbose) puts("STUB: CGColorGetBlack called");
    return NULL;
}

void* CGColorSpaceGetGray(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetGray called");
    return NULL;
}

void* CGColorSpaceGetRGB(void)
{
    if (verbose) puts("STUB: CGColorSpaceGetRGB called");
    return NULL;
}

void* CGImageCreateData(void)
{
    if (verbose) puts("STUB: CGImageCreateData called");
    return NULL;
}

void* CGImageCreateWithData(void)
{
    if (verbose) puts("STUB: CGImageCreateWithData called");
    return NULL;
}

void* CLIArgNameValueParse(void)
{
    if (verbose) puts("STUB: CLIArgNameValueParse called");
    return NULL;
}

void* CLIArgToValue(void)
{
    if (verbose) puts("STUB: CLIArgToValue called");
    return NULL;
}

void* CLIFlagsToValue(void)
{
    if (verbose) puts("STUB: CLIFlagsToValue called");
    return NULL;
}

void* CLIFlagsToValueEx(void)
{
    if (verbose) puts("STUB: CLIFlagsToValueEx called");
    return NULL;
}

void* CLIHelpCommand(void)
{
    if (verbose) puts("STUB: CLIHelpCommand called");
    return NULL;
}

void* CLIInit(void)
{
    if (verbose) puts("STUB: CLIInit called");
    return NULL;
}

void* CLIInteractiveReadLine(void)
{
    if (verbose) puts("STUB: CLIInteractiveReadLine called");
    return NULL;
}

void* CLIInteractiveReadLine_b(void)
{
    if (verbose) puts("STUB: CLIInteractiveReadLine_b called");
    return NULL;
}

void* CLIOutputBegin(void)
{
    if (verbose) puts("STUB: CLIOutputBegin called");
    return NULL;
}

void* CLIOutputEnd(void)
{
    if (verbose) puts("STUB: CLIOutputEnd called");
    return NULL;
}

void* CLIParse(void)
{
    if (verbose) puts("STUB: CLIParse called");
    return NULL;
}

void* CLIProgressClear(void)
{
    if (verbose) puts("STUB: CLIProgressClear called");
    return NULL;
}

void* CLIProgressUpdate(void)
{
    if (verbose) puts("STUB: CLIProgressUpdate called");
    return NULL;
}

void* CLIPromptForPIN(void)
{
    if (verbose) puts("STUB: CLIPromptForPIN called");
    return NULL;
}

void* CPrintF(void)
{
    if (verbose) puts("STUB: CPrintF called");
    return NULL;
}

void* CRC16_CCITT(void)
{
    if (verbose) puts("STUB: CRC16_CCITT called");
    return NULL;
}

void* CUAddSuffixForCurrentDeviceClass(void)
{
    if (verbose) puts("STUB: CUAddSuffixForCurrentDeviceClass called");
    return NULL;
}

void* CUAddSuffixForDeviceClass(void)
{
    if (verbose) puts("STUB: CUAddSuffixForDeviceClass called");
    return NULL;
}

void* CUAltDSIDPrimary(void)
{
    if (verbose) puts("STUB: CUAltDSIDPrimary called");
    return NULL;
}

void* CUCLIPrefs(void)
{
    if (verbose) puts("STUB: CUCLIPrefs called");
    return NULL;
}

void* CUCopyMACAddresses(void)
{
    if (verbose) puts("STUB: CUCopyMACAddresses called");
    return NULL;
}

void* CUDashboardLog(void)
{
    if (verbose) puts("STUB: CUDashboardLog called");
    return NULL;
}

void* CUDescriptionWithLevel(void)
{
    if (verbose) puts("STUB: CUDescriptionWithLevel called");
    return NULL;
}

void* CUDispatchSocketAllocate(void)
{
    if (verbose) puts("STUB: CUDispatchSocketAllocate called");
    return NULL;
}

void* CUDispatchSocketCancelAll(void)
{
    if (verbose) puts("STUB: CUDispatchSocketCancelAll called");
    return NULL;
}

void* CUDispatchSocketReadCanceled(void)
{
    if (verbose) puts("STUB: CUDispatchSocketReadCanceled called");
    return NULL;
}

void* CUDispatchSocketWriteCanceled(void)
{
    if (verbose) puts("STUB: CUDispatchSocketWriteCanceled called");
    return NULL;
}

void* CUDispatchTimerSet(void)
{
    if (verbose) puts("STUB: CUDispatchTimerSet called");
    return NULL;
}

void* CUErrorCFErrorToString(void)
{
    if (verbose) puts("STUB: CUErrorCFErrorToString called");
    return NULL;
}

void* CUErrorDomainCodeToString(void)
{
    if (verbose) puts("STUB: CUErrorDomainCodeToString called");
    return NULL;
}

void* CUGetInterfaceAddresses(void)
{
    if (verbose) puts("STUB: CUGetInterfaceAddresses called");
    return NULL;
}

void* CUIsGreenTeaDevice(void)
{
    if (verbose) puts("STUB: CUIsGreenTeaDevice called");
    return NULL;
}

void* CULocalizedStringEx(void)
{
    if (verbose) puts("STUB: CULocalizedStringEx called");
    return NULL;
}

void* CUMainQueue(void)
{
    if (verbose) puts("STUB: CUMainQueue called");
    return NULL;
}

void* CUMetricsLog(void)
{
    if (verbose) puts("STUB: CUMetricsLog called");
    return NULL;
}

void* CUMetricsLogEx(void)
{
    if (verbose) puts("STUB: CUMetricsLogEx called");
    return NULL;
}

void* CUNormalizeEmailAddress(void)
{
    if (verbose) puts("STUB: CUNormalizeEmailAddress called");
    return NULL;
}

void* CUNormalizePhoneNumber(void)
{
    if (verbose) puts("STUB: CUNormalizePhoneNumber called");
    return NULL;
}

void* CUSetSystemName(void)
{
    if (verbose) puts("STUB: CUSetSystemName called");
    return NULL;
}

void* CUStateDumpWithObject(void)
{
    if (verbose) puts("STUB: CUStateDumpWithObject called");
    return NULL;
}

void* CUTXTDataCreateWithDictionary(void)
{
    if (verbose) puts("STUB: CUTXTDataCreateWithDictionary called");
    return NULL;
}

void* CUTXTDictionaryCreateWithTXTData(void)
{
    if (verbose) puts("STUB: CUTXTDictionaryCreateWithTXTData called");
    return NULL;
}

void* CUVectorClockCompare(void)
{
    if (verbose) puts("STUB: CUVectorClockCompare called");
    return NULL;
}

void* CUVectorClockCreate(void)
{
    if (verbose) puts("STUB: CUVectorClockCreate called");
    return NULL;
}

void* CUVectorClockCreateDictionary(void)
{
    if (verbose) puts("STUB: CUVectorClockCreateDictionary called");
    return NULL;
}

void* CUVectorClockLocalEvent(void)
{
    if (verbose) puts("STUB: CUVectorClockLocalEvent called");
    return NULL;
}

void* CUVectorClockReceivedMessage(void)
{
    if (verbose) puts("STUB: CUVectorClockReceivedMessage called");
    return NULL;
}

void* CUVectorClockSendMessageCopy(void)
{
    if (verbose) puts("STUB: CUVectorClockSendMessageCopy called");
    return NULL;
}

void* CUXPCDecodeBool(void)
{
    if (verbose) puts("STUB: CUXPCDecodeBool called");
    return NULL;
}

void* CUXPCDecodeDouble(void)
{
    if (verbose) puts("STUB: CUXPCDecodeDouble called");
    return NULL;
}

void* CUXPCDecodeNSArrayOfInteger(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSArrayOfInteger called");
    return NULL;
}

void* CUXPCDecodeNSData(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSData called");
    return NULL;
}

void* CUXPCDecodeNSDataOfLength(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSDataOfLength called");
    return NULL;
}

void* CUXPCDecodeNSError(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSError called");
    return NULL;
}

void* CUXPCDecodeNSErrorIfNeeded(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSErrorIfNeeded called");
    return NULL;
}

void* CUXPCDecodeNSString(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSString called");
    return NULL;
}

void* CUXPCDecodeNSUUID(void)
{
    if (verbose) puts("STUB: CUXPCDecodeNSUUID called");
    return NULL;
}

void* CUXPCDecodeSInt64Ranged(void)
{
    if (verbose) puts("STUB: CUXPCDecodeSInt64Ranged called");
    return NULL;
}

void* CUXPCDecodeSInt64RangedEx(void)
{
    if (verbose) puts("STUB: CUXPCDecodeSInt64RangedEx called");
    return NULL;
}

void* CUXPCDecodeUInt64Ranged(void)
{
    if (verbose) puts("STUB: CUXPCDecodeUInt64Ranged called");
    return NULL;
}

void* CUXPCDecodeUInt64RangedEx(void)
{
    if (verbose) puts("STUB: CUXPCDecodeUInt64RangedEx called");
    return NULL;
}

void* CUXPCEncodeNSArrayOfNSNumber(void)
{
    if (verbose) puts("STUB: CUXPCEncodeNSArrayOfNSNumber called");
    return NULL;
}

void* CUXPCEncodeNSError(void)
{
    if (verbose) puts("STUB: CUXPCEncodeNSError called");
    return NULL;
}

void* CUXPCGetNextClientID(void)
{
    if (verbose) puts("STUB: CUXPCGetNextClientID called");
    return NULL;
}

void* CocoaTextViewAppendUTF8String(void)
{
    if (verbose) puts("STUB: CocoaTextViewAppendUTF8String called");
    return NULL;
}

void* CopyEDIDbyUUID(void)
{
    if (verbose) puts("STUB: CopyEDIDbyUUID called");
    return NULL;
}

void* CopyFileDataByFile(void)
{
    if (verbose) puts("STUB: CopyFileDataByFile called");
    return NULL;
}

void* CopyFileDataByPath(void)
{
    if (verbose) puts("STUB: CopyFileDataByPath called");
    return NULL;
}

void* CoreAudioCopyDataSourceName(void)
{
    if (verbose) puts("STUB: CoreAudioCopyDataSourceName called");
    return NULL;
}

void* CoreAudioCopyPropertyData(void)
{
    if (verbose) puts("STUB: CoreAudioCopyPropertyData called");
    return NULL;
}

void* CoreAudioFindDeviceByUID(void)
{
    if (verbose) puts("STUB: CoreAudioFindDeviceByUID called");
    return NULL;
}

void* CoreAudioGetDefaultAudioDevice(void)
{
    if (verbose) puts("STUB: CoreAudioGetDefaultAudioDevice called");
    return NULL;
}

void* CoreAudioGetPropertyData(void)
{
    if (verbose) puts("STUB: CoreAudioGetPropertyData called");
    return NULL;
}

void* CoreAudioPropertyListener_Create(void)
{
    if (verbose) puts("STUB: CoreAudioPropertyListener_Create called");
    return NULL;
}

void* CoreAudioPropertyListener_Delete(void)
{
    if (verbose) puts("STUB: CoreAudioPropertyListener_Delete called");
    return NULL;
}

void* CoreAudioRampParameterStart(void)
{
    if (verbose) puts("STUB: CoreAudioRampParameterStart called");
    return NULL;
}

void* CoreAudioSetDefaultAudioDevice(void)
{
    if (verbose) puts("STUB: CoreAudioSetDefaultAudioDevice called");
    return NULL;
}

void* CoreAudioSetPropertyData(void)
{
    if (verbose) puts("STUB: CoreAudioSetPropertyData called");
    return NULL;
}

void* CreateCAContextForRemoteHosting(void)
{
    if (verbose) puts("STUB: CreateCAContextForRemoteHosting called");
    return NULL;
}

void* CreateCALayerForRemoteDrawing(void)
{
    if (verbose) puts("STUB: CreateCALayerForRemoteDrawing called");
    return NULL;
}

void* CreateScaledImageDataWithData(void)
{
    if (verbose) puts("STUB: CreateScaledImageDataWithData called");
    return NULL;
}

void* CreateTXTRecordWithCString(void)
{
    if (verbose) puts("STUB: CreateTXTRecordWithCString called");
    return NULL;
}

void* CreateUsableInterfaceList(void)
{
    if (verbose) puts("STUB: CreateUsableInterfaceList called");
    return NULL;
}

void* CryptoAEADCreate(void)
{
    if (verbose) puts("STUB: CryptoAEADCreate called");
    return NULL;
}

void* CryptoAEADDecryptMessage(void)
{
    if (verbose) puts("STUB: CryptoAEADDecryptMessage called");
    return NULL;
}

void* CryptoAEADDecryptMessageOneShot(void)
{
    if (verbose) puts("STUB: CryptoAEADDecryptMessageOneShot called");
    return NULL;
}

void* CryptoAEADEncryptMessage(void)
{
    if (verbose) puts("STUB: CryptoAEADEncryptMessage called");
    return NULL;
}

void* CryptoAEADEncryptMessageOneShot(void)
{
    if (verbose) puts("STUB: CryptoAEADEncryptMessageOneShot called");
    return NULL;
}

void* CryptoHKDF(void)
{
    if (verbose) puts("STUB: CryptoHKDF called");
    return NULL;
}

void* CryptoHMACFinal(void)
{
    if (verbose) puts("STUB: CryptoHMACFinal called");
    return NULL;
}

void* CryptoHMACInit(void)
{
    if (verbose) puts("STUB: CryptoHMACInit called");
    return NULL;
}

void* CryptoHMACOneShot(void)
{
    if (verbose) puts("STUB: CryptoHMACOneShot called");
    return NULL;
}

void* CryptoHMACUpdate(void)
{
    if (verbose) puts("STUB: CryptoHMACUpdate called");
    return NULL;
}

void* CryptoHashFinal(void)
{
    if (verbose) puts("STUB: CryptoHashFinal called");
    return NULL;
}

void* CryptoHashInit(void)
{
    if (verbose) puts("STUB: CryptoHashInit called");
    return NULL;
}

void* CryptoHashOneShot(void)
{
    if (verbose) puts("STUB: CryptoHashOneShot called");
    return NULL;
}

void* CryptoHashUpdate(void)
{
    if (verbose) puts("STUB: CryptoHashUpdate called");
    return NULL;
}

void* CryptoRandomKey(void)
{
    if (verbose) puts("STUB: CryptoRandomKey called");
    return NULL;
}

void* Curve25519MakeKeyPair(void)
{
    if (verbose) puts("STUB: Curve25519MakeKeyPair called");
    return NULL;
}

void* DACPServer_AddRemote(void)
{
    if (verbose) puts("STUB: DACPServer_AddRemote called");
    return NULL;
}

void* DACPServer_Create(void)
{
    if (verbose) puts("STUB: DACPServer_Create called");
    return NULL;
}

void* DACPServer_GetRemoteCount(void)
{
    if (verbose) puts("STUB: DACPServer_GetRemoteCount called");
    return NULL;
}

void* DACPServer_RemoveRemote(void)
{
    if (verbose) puts("STUB: DACPServer_RemoveRemote called");
    return NULL;
}

void* DMAPContentBlock_AddCFObjectByKey(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_AddCFObjectByKey called");
    return NULL;
}

void* DMAPContentBlock_AddInt32(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_AddInt32 called");
    return NULL;
}

void* DMAPContentBlock_AddInt8(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_AddInt8 called");
    return NULL;
}

void* DMAPContentBlock_CloseContainer(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_CloseContainer called");
    return NULL;
}

void* DMAPContentBlock_Commit(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_Commit called");
    return NULL;
}

void* DMAPContentBlock_Free(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_Free called");
    return NULL;
}

void* DMAPContentBlock_GetNextChunk(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_GetNextChunk called");
    return NULL;
}

void* DMAPContentBlock_Init(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_Init called");
    return NULL;
}

void* DMAPContentBlock_OpenContainer(void)
{
    if (verbose) puts("STUB: DMAPContentBlock_OpenContainer called");
    return NULL;
}

void* DMAPFindEntryByContentCode(void)
{
    if (verbose) puts("STUB: DMAPFindEntryByContentCode called");
    return NULL;
}

void* DataBuffer_Append(void)
{
    if (verbose) puts("STUB: DataBuffer_Append called");
    return NULL;
}

void* DataBuffer_AppendF(void)
{
    if (verbose) puts("STUB: DataBuffer_AppendF called");
    return NULL;
}

void* DataBuffer_AppendFile(void)
{
    if (verbose) puts("STUB: DataBuffer_AppendFile called");
    return NULL;
}

void* DataBuffer_AppendIE(void)
{
    if (verbose) puts("STUB: DataBuffer_AppendIE called");
    return NULL;
}

void* DataBuffer_Commit(void)
{
    if (verbose) puts("STUB: DataBuffer_Commit called");
    return NULL;
}

void* DataBuffer_Detach(void)
{
    if (verbose) puts("STUB: DataBuffer_Detach called");
    return NULL;
}

void* DataBuffer_Free(void)
{
    if (verbose) puts("STUB: DataBuffer_Free called");
    return NULL;
}

void* DataBuffer_Init(void)
{
    if (verbose) puts("STUB: DataBuffer_Init called");
    return NULL;
}

void* DataBuffer_RunProcessAndAppendOutput(void)
{
    if (verbose) puts("STUB: DataBuffer_RunProcessAndAppendOutput called");
    return NULL;
}

void* DataToHexCStringEx(void)
{
    if (verbose) puts("STUB: DataToHexCStringEx called");
    return NULL;
}

void* DebugGetErrorString(void)
{
    if (verbose) puts("STUB: DebugGetErrorString called");
    return NULL;
}

void* DebugGetErrorStringEx(void)
{
    if (verbose) puts("STUB: DebugGetErrorStringEx called");
    return NULL;
}

void* DebugGetNextError(void)
{
    if (verbose) puts("STUB: DebugGetNextError called");
    return NULL;
}

void* DebugIPC_EnsureFinalized(void)
{
    if (verbose) puts("STUB: DebugIPC_EnsureFinalized called");
    return NULL;
}

void* DebugIPC_EnsureInitialized(void)
{
    if (verbose) puts("STUB: DebugIPC_EnsureInitialized called");
    return NULL;
}

void* DebugIPC_LogControl(void)
{
    if (verbose) puts("STUB: DebugIPC_LogControl called");
    return NULL;
}

void* DebugIPC_PerformF(void)
{
    if (verbose) puts("STUB: DebugIPC_PerformF called");
    return NULL;
}

void* DebugIsCurrentDispatchQueue(void)
{
    if (verbose) puts("STUB: DebugIsCurrentDispatchQueue called");
    return NULL;
}

void* DebugPrintAssert(void)
{
    if (verbose) puts("STUB: DebugPrintAssert called");
    return NULL;
}

void* DeviceOSVersionAtOrLater(void)
{
    if (verbose) puts("STUB: DeviceOSVersionAtOrLater called");
    return NULL;
}

void* DrainUDPSocket(void)
{
    if (verbose) puts("STUB: DrainUDPSocket called");
    return NULL;
}

void* ErrQuit(void)
{
    if (verbose) puts("STUB: ErrQuit called");
    return NULL;
}

void* FDWriteAll(void)
{
    if (verbose) puts("STUB: FDWriteAll called");
    return NULL;
}

void* FPrintF(void)
{
    if (verbose) puts("STUB: FPrintF called");
    return NULL;
}

void* FadeInLayerThruBlack(void)
{
    if (verbose) puts("STUB: FadeInLayerThruBlack called");
    return NULL;
}

void* FadeOutLayerThruBlack(void)
{
    if (verbose) puts("STUB: FadeOutLayerThruBlack called");
    return NULL;
}

void* FatalErrorF(void)
{
    if (verbose) puts("STUB: FatalErrorF called");
    return NULL;
}

void* FatalErrorV(void)
{
    if (verbose) puts("STUB: FatalErrorV called");
    return NULL;
}

void* FourCharCodeToCString(void)
{
    if (verbose) puts("STUB: FourCharCodeToCString called");
    return NULL;
}

void* FreeCommandLineArgV(void)
{
    if (verbose) puts("STUB: FreeCommandLineArgV called");
    return NULL;
}

void* GestaltCopyAnswer(void)
{
    if (verbose) puts("STUB: GestaltCopyAnswer called");
    return NULL;
}

void* GestaltGetBoolean(void)
{
    if (verbose) puts("STUB: GestaltGetBoolean called");
    return NULL;
}

void* GestaltGetCString(void)
{
    if (verbose) puts("STUB: GestaltGetCString called");
    return NULL;
}

void* GestaltGetData(void)
{
    if (verbose) puts("STUB: GestaltGetData called");
    return NULL;
}

void* GestaltGetDeviceClass(void)
{
    if (verbose) puts("STUB: GestaltGetDeviceClass called");
    return NULL;
}

void* GestaltProductTypeStringToDeviceClass(void)
{
    if (verbose) puts("STUB: GestaltProductTypeStringToDeviceClass called");
    return NULL;
}

void* GestaltSetDomain(void)
{
    if (verbose) puts("STUB: GestaltSetDomain called");
    return NULL;
}

void* GestaltSetHook(void)
{
    if (verbose) puts("STUB: GestaltSetHook called");
    return NULL;
}

void* GetDeviceInternalModelString(void)
{
    if (verbose) puts("STUB: GetDeviceInternalModelString called");
    return NULL;
}

void* GetDeviceModelString(void)
{
    if (verbose) puts("STUB: GetDeviceModelString called");
    return NULL;
}

void* GetDeviceName(void)
{
    if (verbose) puts("STUB: GetDeviceName called");
    return NULL;
}

void* GetDeviceUniqueID(void)
{
    if (verbose) puts("STUB: GetDeviceUniqueID called");
    return NULL;
}

void* GetFairPlayHWInfo(void)
{
    if (verbose) puts("STUB: GetFairPlayHWInfo called");
    return NULL;
}

void* GetFairPlayHWInfoEx(void)
{
    if (verbose) puts("STUB: GetFairPlayHWInfoEx called");
    return NULL;
}

void* GetFileExtensionFromString(void)
{
    if (verbose) puts("STUB: GetFileExtensionFromString called");
    return NULL;
}

void* GetInterfaceMACAddress(void)
{
    if (verbose) puts("STUB: GetInterfaceMACAddress called");
    return NULL;
}

void* GetLastFilePathSegment(void)
{
    if (verbose) puts("STUB: GetLastFilePathSegment called");
    return NULL;
}

void* GetLocalHostName(void)
{
    if (verbose) puts("STUB: GetLocalHostName called");
    return NULL;
}

void* GetMachThreadPriority(void)
{
    if (verbose) puts("STUB: GetMachThreadPriority called");
    return NULL;
}

void* GetPeerMACAddress(void)
{
    if (verbose) puts("STUB: GetPeerMACAddress called");
    return NULL;
}

void* GetPrimaryIPAddress(void)
{
    if (verbose) puts("STUB: GetPrimaryIPAddress called");
    return NULL;
}

void* GetPrimaryMACAddress(void)
{
    if (verbose) puts("STUB: GetPrimaryMACAddress called");
    return NULL;
}

void* GetProcessNameByPID(void)
{
    if (verbose) puts("STUB: GetProcessNameByPID called");
    return NULL;
}

void* GetSystemBuildVersionString(void)
{
    if (verbose) puts("STUB: GetSystemBuildVersionString called");
    return NULL;
}

void* H264ConvertAVCCtoAnnexBHeader(void)
{
    if (verbose) puts("STUB: H264ConvertAVCCtoAnnexBHeader called");
    return NULL;
}

void* H264EscapeEmulationPrevention(void)
{
    if (verbose) puts("STUB: H264EscapeEmulationPrevention called");
    return NULL;
}

void* H264GetNextNALUnit(void)
{
    if (verbose) puts("STUB: H264GetNextNALUnit called");
    return NULL;
}

void* HIDBrowserCopyProperty(void)
{
    if (verbose) puts("STUB: HIDBrowserCopyProperty called");
    return NULL;
}

void* HIDBrowserCreate(void)
{
    if (verbose) puts("STUB: HIDBrowserCreate called");
    return NULL;
}

void* HIDBrowserSetDispatchQueue(void)
{
    if (verbose) puts("STUB: HIDBrowserSetDispatchQueue called");
    return NULL;
}

void* HIDBrowserSetEventHandler(void)
{
    if (verbose) puts("STUB: HIDBrowserSetEventHandler called");
    return NULL;
}

void* HIDBrowserSetProperty(void)
{
    if (verbose) puts("STUB: HIDBrowserSetProperty called");
    return NULL;
}

void* HIDBrowserStart(void)
{
    if (verbose) puts("STUB: HIDBrowserStart called");
    return NULL;
}

void* HIDBrowserStop(void)
{
    if (verbose) puts("STUB: HIDBrowserStop called");
    return NULL;
}

void* HIDBrowserStopDevices(void)
{
    if (verbose) puts("STUB: HIDBrowserStopDevices called");
    return NULL;
}

void* HIDButtonsCreateDescriptor(void)
{
    if (verbose) puts("STUB: HIDButtonsCreateDescriptor called");
    return NULL;
}

void* HIDCopyOverrideDescriptor(void)
{
    if (verbose) puts("STUB: HIDCopyOverrideDescriptor called");
    return NULL;
}

void* HIDDPadCreateDescriptor(void)
{
    if (verbose) puts("STUB: HIDDPadCreateDescriptor called");
    return NULL;
}

void* HIDDPadFillReport(void)
{
    if (verbose) puts("STUB: HIDDPadFillReport called");
    return NULL;
}

void* HIDDeregisterDevice(void)
{
    if (verbose) puts("STUB: HIDDeregisterDevice called");
    return NULL;
}

void* HIDDeregisterOverrideDescriptor(void)
{
    if (verbose) puts("STUB: HIDDeregisterOverrideDescriptor called");
    return NULL;
}

void* HIDDeviceCopyProperty(void)
{
    if (verbose) puts("STUB: HIDDeviceCopyProperty called");
    return NULL;
}

void* HIDDeviceCreateVirtual(void)
{
    if (verbose) puts("STUB: HIDDeviceCreateVirtual called");
    return NULL;
}

void* HIDDevicePostReport(void)
{
    if (verbose) puts("STUB: HIDDevicePostReport called");
    return NULL;
}

void* HIDDeviceSetDispatchQueue(void)
{
    if (verbose) puts("STUB: HIDDeviceSetDispatchQueue called");
    return NULL;
}

void* HIDDeviceSetEventHandler(void)
{
    if (verbose) puts("STUB: HIDDeviceSetEventHandler called");
    return NULL;
}

void* HIDDeviceSetProperty(void)
{
    if (verbose) puts("STUB: HIDDeviceSetProperty called");
    return NULL;
}

void* HIDDeviceStart(void)
{
    if (verbose) puts("STUB: HIDDeviceStart called");
    return NULL;
}

void* HIDDeviceStop(void)
{
    if (verbose) puts("STUB: HIDDeviceStop called");
    return NULL;
}

void* HIDKnobCreateDescriptor(void)
{
    if (verbose) puts("STUB: HIDKnobCreateDescriptor called");
    return NULL;
}

void* HIDKnobFillReport(void)
{
    if (verbose) puts("STUB: HIDKnobFillReport called");
    return NULL;
}

void* HIDPointingCreateDescriptor(void)
{
    if (verbose) puts("STUB: HIDPointingCreateDescriptor called");
    return NULL;
}

void* HIDPointingFillReport(void)
{
    if (verbose) puts("STUB: HIDPointingFillReport called");
    return NULL;
}

void* HIDPostReport(void)
{
    if (verbose) puts("STUB: HIDPostReport called");
    return NULL;
}

void* HIDRegisterDevice(void)
{
    if (verbose) puts("STUB: HIDRegisterDevice called");
    return NULL;
}

void* HIDRegisterOverrideDescriptor(void)
{
    if (verbose) puts("STUB: HIDRegisterOverrideDescriptor called");
    return NULL;
}

void* HIDSimpleFillReport(void)
{
    if (verbose) puts("STUB: HIDSimpleFillReport called");
    return NULL;
}

void* HIDTelephonyCreateDescriptor(void)
{
    if (verbose) puts("STUB: HIDTelephonyCreateDescriptor called");
    return NULL;
}

void* HIDTrackpadCreateDescriptor(void)
{
    if (verbose) puts("STUB: HIDTrackpadCreateDescriptor called");
    return NULL;
}

void* HIDTrackpadFillReport(void)
{
    if (verbose) puts("STUB: HIDTrackpadFillReport called");
    return NULL;
}

void* HTTPClientAuthorization_Apply(void)
{
    if (verbose) puts("STUB: HTTPClientAuthorization_Apply called");
    return NULL;
}

void* HTTPClientAuthorization_Free(void)
{
    if (verbose) puts("STUB: HTTPClientAuthorization_Free called");
    return NULL;
}

void* HTTPClientAuthorization_Init(void)
{
    if (verbose) puts("STUB: HTTPClientAuthorization_Init called");
    return NULL;
}

void* HTTPClientCreate(void)
{
    if (verbose) puts("STUB: HTTPClientCreate called");
    return NULL;
}

void* HTTPClientCreateWithSocket(void)
{
    if (verbose) puts("STUB: HTTPClientCreateWithSocket called");
    return NULL;
}

void* HTTPClientDetach(void)
{
    if (verbose) puts("STUB: HTTPClientDetach called");
    return NULL;
}

void* HTTPClientGetDelegateContext(void)
{
    if (verbose) puts("STUB: HTTPClientGetDelegateContext called");
    return NULL;
}

void* HTTPClientGetPeerAddress(void)
{
    if (verbose) puts("STUB: HTTPClientGetPeerAddress called");
    return NULL;
}

void* HTTPClientGetSelfAddress(void)
{
    if (verbose) puts("STUB: HTTPClientGetSelfAddress called");
    return NULL;
}

void* HTTPClientInvalidate(void)
{
    if (verbose) puts("STUB: HTTPClientInvalidate called");
    return NULL;
}

void* HTTPClientSendBinaryBytes(void)
{
    if (verbose) puts("STUB: HTTPClientSendBinaryBytes called");
    return NULL;
}

void* HTTPClientSendMessage(void)
{
    if (verbose) puts("STUB: HTTPClientSendMessage called");
    return NULL;
}

void* HTTPClientSendMessageSync(void)
{
    if (verbose) puts("STUB: HTTPClientSendMessageSync called");
    return NULL;
}

void* HTTPClientSetConnectionLogging(void)
{
    if (verbose) puts("STUB: HTTPClientSetConnectionLogging called");
    return NULL;
}

void* HTTPClientSetDebugDelegate(void)
{
    if (verbose) puts("STUB: HTTPClientSetDebugDelegate called");
    return NULL;
}

void* HTTPClientSetDelegate(void)
{
    if (verbose) puts("STUB: HTTPClientSetDelegate called");
    return NULL;
}

void* HTTPClientSetDestination(void)
{
    if (verbose) puts("STUB: HTTPClientSetDestination called");
    return NULL;
}

void* HTTPClientSetDispatchQueue(void)
{
    if (verbose) puts("STUB: HTTPClientSetDispatchQueue called");
    return NULL;
}

void* HTTPClientSetFlags(void)
{
    if (verbose) puts("STUB: HTTPClientSetFlags called");
    return NULL;
}

void* HTTPClientSetKeepAlive(void)
{
    if (verbose) puts("STUB: HTTPClientSetKeepAlive called");
    return NULL;
}

void* HTTPClientSetKeepAliveEx(void)
{
    if (verbose) puts("STUB: HTTPClientSetKeepAliveEx called");
    return NULL;
}

void* HTTPClientSetLogging(void)
{
    if (verbose) puts("STUB: HTTPClientSetLogging called");
    return NULL;
}

void* HTTPClientSetTimeout(void)
{
    if (verbose) puts("STUB: HTTPClientSetTimeout called");
    return NULL;
}

void* HTTPClientSetTransportDelegate(void)
{
    if (verbose) puts("STUB: HTTPClientSetTransportDelegate called");
    return NULL;
}

void* HTTPConnectionCreate(void)
{
    if (verbose) puts("STUB: HTTPConnectionCreate called");
    return NULL;
}

void* HTTPConnectionDetach(void)
{
    if (verbose) puts("STUB: HTTPConnectionDetach called");
    return NULL;
}

void* HTTPConnectionGetNextURLSegmentEx(void)
{
    if (verbose) puts("STUB: HTTPConnectionGetNextURLSegmentEx called");
    return NULL;
}

void* HTTPConnectionInitResponse(void)
{
    if (verbose) puts("STUB: HTTPConnectionInitResponse called");
    return NULL;
}

void* HTTPConnectionSendBinaryBytes(void)
{
    if (verbose) puts("STUB: HTTPConnectionSendBinaryBytes called");
    return NULL;
}

void* HTTPConnectionSendEvent(void)
{
    if (verbose) puts("STUB: HTTPConnectionSendEvent called");
    return NULL;
}

void* HTTPConnectionSendResponse(void)
{
    if (verbose) puts("STUB: HTTPConnectionSendResponse called");
    return NULL;
}

void* HTTPConnectionSendSimpleResponseEx(void)
{
    if (verbose) puts("STUB: HTTPConnectionSendSimpleResponseEx called");
    return NULL;
}

void* HTTPConnectionSetDelegate(void)
{
    if (verbose) puts("STUB: HTTPConnectionSetDelegate called");
    return NULL;
}

void* HTTPConnectionSetDispatchQueue(void)
{
    if (verbose) puts("STUB: HTTPConnectionSetDispatchQueue called");
    return NULL;
}

void* HTTPConnectionSetTimeout(void)
{
    if (verbose) puts("STUB: HTTPConnectionSetTimeout called");
    return NULL;
}

void* HTTPConnectionSetTransportDelegate(void)
{
    if (verbose) puts("STUB: HTTPConnectionSetTransportDelegate called");
    return NULL;
}

void* HTTPConnectionStart(void)
{
    if (verbose) puts("STUB: HTTPConnectionStart called");
    return NULL;
}

void* HTTPConnectionStop(void)
{
    if (verbose) puts("STUB: HTTPConnectionStop called");
    return NULL;
}

void* HTTPConnectionStopAsync(void)
{
    if (verbose) puts("STUB: HTTPConnectionStopAsync called");
    return NULL;
}

void* HTTPGetHeaderField(void)
{
    if (verbose) puts("STUB: HTTPGetHeaderField called");
    return NULL;
}

void* HTTPGetReasonPhrase(void)
{
    if (verbose) puts("STUB: HTTPGetReasonPhrase called");
    return NULL;
}

void* HTTPHeader_Commit(void)
{
    if (verbose) puts("STUB: HTTPHeader_Commit called");
    return NULL;
}

void* HTTPHeader_InitRequest(void)
{
    if (verbose) puts("STUB: HTTPHeader_InitRequest called");
    return NULL;
}

void* HTTPHeader_InitRequestF(void)
{
    if (verbose) puts("STUB: HTTPHeader_InitRequestF called");
    return NULL;
}

void* HTTPHeader_InitResponse(void)
{
    if (verbose) puts("STUB: HTTPHeader_InitResponse called");
    return NULL;
}

void* HTTPHeader_InitResponseEx(void)
{
    if (verbose) puts("STUB: HTTPHeader_InitResponseEx called");
    return NULL;
}

void* HTTPHeader_Parse(void)
{
    if (verbose) puts("STUB: HTTPHeader_Parse called");
    return NULL;
}

void* HTTPHeader_SetField(void)
{
    if (verbose) puts("STUB: HTTPHeader_SetField called");
    return NULL;
}

void* HTTPMakeDateString(void)
{
    if (verbose) puts("STUB: HTTPMakeDateString called");
    return NULL;
}

void* HTTPMakeTimedNonce(void)
{
    if (verbose) puts("STUB: HTTPMakeTimedNonce called");
    return NULL;
}

void* HTTPMessageCopy(void)
{
    if (verbose) puts("STUB: HTTPMessageCopy called");
    return NULL;
}

void* HTTPMessageCreate(void)
{
    if (verbose) puts("STUB: HTTPMessageCreate called");
    return NULL;
}

void* HTTPMessageCreateDictionary(void)
{
    if (verbose) puts("STUB: HTTPMessageCreateDictionary called");
    return NULL;
}

void* HTTPMessageGetHeaderField(void)
{
    if (verbose) puts("STUB: HTTPMessageGetHeaderField called");
    return NULL;
}

void* HTTPMessageGetOrCopyFormVariable(void)
{
    if (verbose) puts("STUB: HTTPMessageGetOrCopyFormVariable called");
    return NULL;
}

void* HTTPMessageInitRequest(void)
{
    if (verbose) puts("STUB: HTTPMessageInitRequest called");
    return NULL;
}

void* HTTPMessageInitResponse(void)
{
    if (verbose) puts("STUB: HTTPMessageInitResponse called");
    return NULL;
}

void* HTTPMessageSetBody(void)
{
    if (verbose) puts("STUB: HTTPMessageSetBody called");
    return NULL;
}

void* HTTPMessageSetBodyFileDescriptor(void)
{
    if (verbose) puts("STUB: HTTPMessageSetBodyFileDescriptor called");
    return NULL;
}

void* HTTPMessageSetBodyFilePath(void)
{
    if (verbose) puts("STUB: HTTPMessageSetBodyFilePath called");
    return NULL;
}

void* HTTPMessageSetBodyLength(void)
{
    if (verbose) puts("STUB: HTTPMessageSetBodyLength called");
    return NULL;
}

void* HTTPMessageSetCompletionBlock(void)
{
    if (verbose) puts("STUB: HTTPMessageSetCompletionBlock called");
    return NULL;
}

void* HTTPMessageSetHeaderField(void)
{
    if (verbose) puts("STUB: HTTPMessageSetHeaderField called");
    return NULL;
}

void* HTTPMessageSetReceiveFilePath(void)
{
    if (verbose) puts("STUB: HTTPMessageSetReceiveFilePath called");
    return NULL;
}

void* HTTPParseByteRangeRequest(void)
{
    if (verbose) puts("STUB: HTTPParseByteRangeRequest called");
    return NULL;
}

void* HTTPParseCacheControlHeader(void)
{
    if (verbose) puts("STUB: HTTPParseCacheControlHeader called");
    return NULL;
}

void* HTTPParseCacheControlValue(void)
{
    if (verbose) puts("STUB: HTTPParseCacheControlValue called");
    return NULL;
}

void* HTTPParseParameter(void)
{
    if (verbose) puts("STUB: HTTPParseParameter called");
    return NULL;
}

void* HTTPParseRTPInfo(void)
{
    if (verbose) puts("STUB: HTTPParseRTPInfo called");
    return NULL;
}

void* HTTPReadHeader(void)
{
    if (verbose) puts("STUB: HTTPReadHeader called");
    return NULL;
}

void* HTTPScanFHeaderValue(void)
{
    if (verbose) puts("STUB: HTTPScanFHeaderValue called");
    return NULL;
}

void* HTTPServerCreate(void)
{
    if (verbose) puts("STUB: HTTPServerCreate called");
    return NULL;
}

void* HTTPServerSetDispatchQueue(void)
{
    if (verbose) puts("STUB: HTTPServerSetDispatchQueue called");
    return NULL;
}

void* HTTPServerSetLogging(void)
{
    if (verbose) puts("STUB: HTTPServerSetLogging called");
    return NULL;
}

void* HTTPServer_Create(void)
{
    if (verbose) puts("STUB: HTTPServer_Create called");
    return NULL;
}

void* HTTPServer_Delete(void)
{
    if (verbose) puts("STUB: HTTPServer_Delete called");
    return NULL;
}

void* HTTPServer_GetNextURLSegment(void)
{
    if (verbose) puts("STUB: HTTPServer_GetNextURLSegment called");
    return NULL;
}

void* HTTPServer_GetOrCopyFormVariable(void)
{
    if (verbose) puts("STUB: HTTPServer_GetOrCopyFormVariable called");
    return NULL;
}

void* HTTPServer_InitResponse(void)
{
    if (verbose) puts("STUB: HTTPServer_InitResponse called");
    return NULL;
}

void* HTTPServer_SendSimpleResponse(void)
{
    if (verbose) puts("STUB: HTTPServer_SendSimpleResponse called");
    return NULL;
}

void* HTTPServer_SendStatusResponse(void)
{
    if (verbose) puts("STUB: HTTPServer_SendStatusResponse called");
    return NULL;
}

void* HTTPServer_SendUnauthorizedResponse(void)
{
    if (verbose) puts("STUB: HTTPServer_SendUnauthorizedResponse called");
    return NULL;
}

void* HTTPServer_Start(void)
{
    if (verbose) puts("STUB: HTTPServer_Start called");
    return NULL;
}

void* HTTPServer_Stop(void)
{
    if (verbose) puts("STUB: HTTPServer_Stop called");
    return NULL;
}

void* HTTPVerifyAuthorization(void)
{
    if (verbose) puts("STUB: HTTPVerifyAuthorization called");
    return NULL;
}

void* HTTPVerifyTimedNonce(void)
{
    if (verbose) puts("STUB: HTTPVerifyTimedNonce called");
    return NULL;
}

void* HardwareAddressToCString(void)
{
    if (verbose) puts("STUB: HardwareAddressToCString called");
    return NULL;
}

void* HexToData(void)
{
    if (verbose) puts("STUB: HexToData called");
    return NULL;
}

void* HexToDataCopy(void)
{
    if (verbose) puts("STUB: HexToDataCopy called");
    return NULL;
}

void* IEBufferAppendIE(void)
{
    if (verbose) puts("STUB: IEBufferAppendIE called");
    return NULL;
}

void* IEBufferEndVendorIE(void)
{
    if (verbose) puts("STUB: IEBufferEndVendorIE called");
    return NULL;
}

void* IEBufferStartVendorIE(void)
{
    if (verbose) puts("STUB: IEBufferStartVendorIE called");
    return NULL;
}

void* IEEE80211iPasswordHash(void)
{
    if (verbose) puts("STUB: IEEE80211iPasswordHash called");
    return NULL;
}

void* IEGet(void)
{
    if (verbose) puts("STUB: IEGet called");
    return NULL;
}

void* IEGetAppleGeneral(void)
{
    if (verbose) puts("STUB: IEGetAppleGeneral called");
    return NULL;
}

void* IEGetNext(void)
{
    if (verbose) puts("STUB: IEGetNext called");
    return NULL;
}

void* IEGetVendorSpecific(void)
{
    if (verbose) puts("STUB: IEGetVendorSpecific called");
    return NULL;
}

void* IO8Log(void)
{
    if (verbose) puts("STUB: IO8Log called");
    return NULL;
}

void* IO8Log2(void)
{
    if (verbose) puts("STUB: IO8Log2 called");
    return NULL;
}

void* IO8Logv(void)
{
    if (verbose) puts("STUB: IO8Logv called");
    return NULL;
}

void* IO8Logv2(void)
{
    if (verbose) puts("STUB: IO8Logv2 called");
    return NULL;
}

void* IOKitCopyParentOfClass(void)
{
    if (verbose) puts("STUB: IOKitCopyParentOfClass called");
    return NULL;
}

void* IPv4AddressToCString(void)
{
    if (verbose) puts("STUB: IPv4AddressToCString called");
    return NULL;
}

void* IPv6AddressToCString(void)
{
    if (verbose) puts("STUB: IPv6AddressToCString called");
    return NULL;
}

void* IncrementNumericSuffix(void)
{
    if (verbose) puts("STUB: IncrementNumericSuffix called");
    return NULL;
}

void* IsAppleInternalBuild(void)
{
    if (verbose) puts("STUB: IsAppleInternalBuild called");
    return NULL;
}

void* IsAppleTV(void)
{
    if (verbose) puts("STUB: IsAppleTV called");
    return NULL;
}

void* IsGlobalIPv4Address(void)
{
    if (verbose) puts("STUB: IsGlobalIPv4Address called");
    return NULL;
}

void* IsIPv6MartianAddress(void)
{
    if (verbose) puts("STUB: IsIPv6MartianAddress called");
    return NULL;
}

void* IsIPv6MartianAddressEx(void)
{
    if (verbose) puts("STUB: IsIPv6MartianAddressEx called");
    return NULL;
}

void* IsWiFiNetworkInterface(void)
{
    if (verbose) puts("STUB: IsWiFiNetworkInterface called");
    return NULL;
}

void* KeychainAddFormatted(void)
{
    if (verbose) puts("STUB: KeychainAddFormatted called");
    return NULL;
}

void* KeychainCopyMatchingFormatted(void)
{
    if (verbose) puts("STUB: KeychainCopyMatchingFormatted called");
    return NULL;
}

void* KeychainDeleteFormatted(void)
{
    if (verbose) puts("STUB: KeychainDeleteFormatted called");
    return NULL;
}

void* KeychainDeleteItemByPersistentRef(void)
{
    if (verbose) puts("STUB: KeychainDeleteItemByPersistentRef called");
    return NULL;
}

void* KeychainUpdateFormatted(void)
{
    if (verbose) puts("STUB: KeychainUpdateFormatted called");
    return NULL;
}

void* LevenshteinDistance(void)
{
    if (verbose) puts("STUB: LevenshteinDistance called");
    return NULL;
}

void* LogCategoryCopyOSLogHandle(void)
{
    if (verbose) puts("STUB: LogCategoryCopyOSLogHandle called");
    return NULL;
}

void* LogCategoryCreate(void)
{
    if (verbose) puts("STUB: LogCategoryCreate called");
    return NULL;
}

void* LogCategoryCreateEx(void)
{
    if (verbose) puts("STUB: LogCategoryCreateEx called");
    return NULL;
}

void* LogCategoryReplaceF(void)
{
    if (verbose) puts("STUB: LogCategoryReplaceF called");
    return NULL;
}

void* LogCategoryReplaceV(void)
{
    if (verbose) puts("STUB: LogCategoryReplaceV called");
    return NULL;
}

void* LogCategory_Remove(void)
{
    if (verbose) puts("STUB: LogCategory_Remove called");
    return NULL;
}

void* LogControl(void)
{
    if (verbose) puts("STUB: LogControl called");
    return NULL;
}

void* LogControlCF(void)
{
    if (verbose) puts("STUB: LogControlCF called");
    return NULL;
}

void* LogHTTP(void)
{
    if (verbose) puts("STUB: LogHTTP called");
    return NULL;
}

void* LogPrintF(void)
{
    if (verbose) puts("STUB: LogPrintF called");
    return NULL;
}

void* LogPrintV(void)
{
    if (verbose) puts("STUB: LogPrintV called");
    return NULL;
}

void* LogSetAppID(void)
{
    if (verbose) puts("STUB: LogSetAppID called");
    return NULL;
}

void* LogSetOutputCallback(void)
{
    if (verbose) puts("STUB: LogSetOutputCallback called");
    return NULL;
}

void* LogShow(void)
{
    if (verbose) puts("STUB: LogShow called");
    return NULL;
}

void* MFiPlatform_CopyCertificate(void)
{
    if (verbose) puts("STUB: MFiPlatform_CopyCertificate called");
    return NULL;
}

void* MFiPlatform_CreateSignature(void)
{
    if (verbose) puts("STUB: MFiPlatform_CreateSignature called");
    return NULL;
}

void* MFiPlatform_CreateSignatureEx(void)
{
    if (verbose) puts("STUB: MFiPlatform_CreateSignatureEx called");
    return NULL;
}

void* MFiPlatform_Finalize(void)
{
    if (verbose) puts("STUB: MFiPlatform_Finalize called");
    return NULL;
}

void* MFiPlatform_Initialize(void)
{
    if (verbose) puts("STUB: MFiPlatform_Initialize called");
    return NULL;
}

void* MFiPlatform_VerifySignature(void)
{
    if (verbose) puts("STUB: MFiPlatform_VerifySignature called");
    return NULL;
}

void* MFiSAP_CopyCertificate(void)
{
    if (verbose) puts("STUB: MFiSAP_CopyCertificate called");
    return NULL;
}

void* MFiSAP_CopyCertificateSerialNumber(void)
{
    if (verbose) puts("STUB: MFiSAP_CopyCertificateSerialNumber called");
    return NULL;
}

void* MFiSAP_Create(void)
{
    if (verbose) puts("STUB: MFiSAP_Create called");
    return NULL;
}

void* MFiSAP_Decrypt(void)
{
    if (verbose) puts("STUB: MFiSAP_Decrypt called");
    return NULL;
}

void* MFiSAP_Delete(void)
{
    if (verbose) puts("STUB: MFiSAP_Delete called");
    return NULL;
}

void* MFiSAP_Encrypt(void)
{
    if (verbose) puts("STUB: MFiSAP_Encrypt called");
    return NULL;
}

void* MFiSAP_Exchange(void)
{
    if (verbose) puts("STUB: MFiSAP_Exchange called");
    return NULL;
}

void* MIMETypeToExtension(void)
{
    if (verbose) puts("STUB: MIMETypeToExtension called");
    return NULL;
}

void* MakeFractionalDateString(void)
{
    if (verbose) puts("STUB: MakeFractionalDateString called");
    return NULL;
}

void* MapCFStringToValue(void)
{
    if (verbose) puts("STUB: MapCFStringToValue called");
    return NULL;
}

void* MapStringToValue(void)
{
    if (verbose) puts("STUB: MapStringToValue called");
    return NULL;
}

void* MemPrintF(void)
{
    if (verbose) puts("STUB: MemPrintF called");
    return NULL;
}

void* MicrosecondsToUpTicks(void)
{
    if (verbose) puts("STUB: MicrosecondsToUpTicks called");
    return NULL;
}

void* MillisecondsToUpTicks(void)
{
    if (verbose) puts("STUB: MillisecondsToUpTicks called");
    return NULL;
}

void* MirroredRingBufferFree(void)
{
    if (verbose) puts("STUB: MirroredRingBufferFree called");
    return NULL;
}

void* MirroredRingBufferInit(void)
{
    if (verbose) puts("STUB: MirroredRingBufferInit called");
    return NULL;
}

void* MorseCode(void)
{
    if (verbose) puts("STUB: MorseCode called");
    return NULL;
}

void* NADyFF4UGjtVGBmEW6h(void)
{
    if (verbose) puts("STUB: NADyFF4UGjtVGBmEW6h called");
    return NULL;
}

void* NMEAFree(void)
{
    if (verbose) puts("STUB: NMEAFree called");
    return NULL;
}

void* NMEAParse(void)
{
    if (verbose) puts("STUB: NMEAParse called");
    return NULL;
}

void* NSAppendPrintF(void)
{
    if (verbose) puts("STUB: NSAppendPrintF called");
    return NULL;
}

void* NSAppendPrintV(void)
{
    if (verbose) puts("STUB: NSAppendPrintV called");
    return NULL;
}

void* NSArrayGetNSUUIDAtIndex(void)
{
    if (verbose) puts("STUB: NSArrayGetNSUUIDAtIndex called");
    return NULL;
}

void* NSArrayGetValueAtIndexOfClass(void)
{
    if (verbose) puts("STUB: NSArrayGetValueAtIndexOfClass called");
    return NULL;
}

void* NSArrayValidateClasses(void)
{
    if (verbose) puts("STUB: NSArrayValidateClasses called");
    return NULL;
}

void* NSDataCompress(void)
{
    if (verbose) puts("STUB: NSDataCompress called");
    return NULL;
}

void* NSDataDecompress(void)
{
    if (verbose) puts("STUB: NSDataDecompress called");
    return NULL;
}

void* NSDataWithHex(void)
{
    if (verbose) puts("STUB: NSDataWithHex called");
    return NULL;
}

void* NSDecodeNSArrayOfClassIfPresent(void)
{
    if (verbose) puts("STUB: NSDecodeNSArrayOfClassIfPresent called");
    return NULL;
}

void* NSDecodeNSDictionaryOfClassesIfPresent(void)
{
    if (verbose) puts("STUB: NSDecodeNSDictionaryOfClassesIfPresent called");
    return NULL;
}

void* NSDecodeNSSetOfClassIfPresent(void)
{
    if (verbose) puts("STUB: NSDecodeNSSetOfClassIfPresent called");
    return NULL;
}

void* NSDecodeObjectIfPresent(void)
{
    if (verbose) puts("STUB: NSDecodeObjectIfPresent called");
    return NULL;
}

void* NSDecodeSInt64RangedIfPresent(void)
{
    if (verbose) puts("STUB: NSDecodeSInt64RangedIfPresent called");
    return NULL;
}

void* NSDecodeStandardContainerIfPresent(void)
{
    if (verbose) puts("STUB: NSDecodeStandardContainerIfPresent called");
    return NULL;
}

void* NSDictionaryGetNSArrayOfClass(void)
{
    if (verbose) puts("STUB: NSDictionaryGetNSArrayOfClass called");
    return NULL;
}

void* NSDictionaryGetNSUUID(void)
{
    if (verbose) puts("STUB: NSDictionaryGetNSUUID called");
    return NULL;
}

void* NSDictionaryGetValueOfClass(void)
{
    if (verbose) puts("STUB: NSDictionaryGetValueOfClass called");
    return NULL;
}

void* NSDictionaryValidateClasses(void)
{
    if (verbose) puts("STUB: NSDictionaryValidateClasses called");
    return NULL;
}

void* NSErrorF(void)
{
    if (verbose) puts("STUB: NSErrorF called");
    return NULL;
}

void* NSErrorNestedF(void)
{
    if (verbose) puts("STUB: NSErrorNestedF called");
    return NULL;
}

void* NSErrorNestedV(void)
{
    if (verbose) puts("STUB: NSErrorNestedV called");
    return NULL;
}

void* NSErrorToOSStatus(void)
{
    if (verbose) puts("STUB: NSErrorToOSStatus called");
    return NULL;
}

void* NSErrorV(void)
{
    if (verbose) puts("STUB: NSErrorV called");
    return NULL;
}

void* NSErrorWithOSStatusF(void)
{
    if (verbose) puts("STUB: NSErrorWithOSStatusF called");
    return NULL;
}

void* NSPredicateMatches(void)
{
    if (verbose) puts("STUB: NSPredicateMatches called");
    return NULL;
}

void* NSPrintF(void)
{
    if (verbose) puts("STUB: NSPrintF called");
    return NULL;
}

void* NSPrintTypedObject(void)
{
    if (verbose) puts("STUB: NSPrintTypedObject called");
    return NULL;
}

void* NSPrintV(void)
{
    if (verbose) puts("STUB: NSPrintV called");
    return NULL;
}

void* NSRandomData(void)
{
    if (verbose) puts("STUB: NSRandomData called");
    return NULL;
}

void* NSStringWithMACAddress(void)
{
    if (verbose) puts("STUB: NSStringWithMACAddress called");
    return NULL;
}

void* NTPClockCreate(void)
{
    if (verbose) puts("STUB: NTPClockCreate called");
    return NULL;
}

void* NTPClockGetListenPort(void)
{
    if (verbose) puts("STUB: NTPClockGetListenPort called");
    return NULL;
}

void* NTPClockGetSynchronizedNTPTime(void)
{
    if (verbose) puts("STUB: NTPClockGetSynchronizedNTPTime called");
    return NULL;
}

void* NTPClockGetSynchronizedNTPTimeNearUpTicks(void)
{
    if (verbose) puts("STUB: NTPClockGetSynchronizedNTPTimeNearUpTicks called");
    return NULL;
}

void* NTPClockGetUpTicksNearSynchronizedNTPTime(void)
{
    if (verbose) puts("STUB: NTPClockGetUpTicksNearSynchronizedNTPTime called");
    return NULL;
}

void* NTPClockSetEpoch(void)
{
    if (verbose) puts("STUB: NTPClockSetEpoch called");
    return NULL;
}

void* NTPClockSetListenPort(void)
{
    if (verbose) puts("STUB: NTPClockSetListenPort called");
    return NULL;
}

void* NTPClockSetLogging(void)
{
    if (verbose) puts("STUB: NTPClockSetLogging called");
    return NULL;
}

void* NTPClockSetP2P(void)
{
    if (verbose) puts("STUB: NTPClockSetP2P called");
    return NULL;
}

void* NTPClockSetPeer(void)
{
    if (verbose) puts("STUB: NTPClockSetPeer called");
    return NULL;
}

void* NTPClockSetQoSDisabled(void)
{
    if (verbose) puts("STUB: NTPClockSetQoSDisabled called");
    return NULL;
}

void* NTPClockSetRTCP(void)
{
    if (verbose) puts("STUB: NTPClockSetRTCP called");
    return NULL;
}

void* NTPClockSetThreadName(void)
{
    if (verbose) puts("STUB: NTPClockSetThreadName called");
    return NULL;
}

void* NTPClockSetThreadPriority(void)
{
    if (verbose) puts("STUB: NTPClockSetThreadPriority called");
    return NULL;
}

void* NTPClockSetUpTicksConvertHandler(void)
{
    if (verbose) puts("STUB: NTPClockSetUpTicksConvertHandler called");
    return NULL;
}

void* NTPClockSetUpTicksPerSecondHandler(void)
{
    if (verbose) puts("STUB: NTPClockSetUpTicksPerSecondHandler called");
    return NULL;
}

void* NTPClockStartClient(void)
{
    if (verbose) puts("STUB: NTPClockStartClient called");
    return NULL;
}

void* NTPClockStartServer(void)
{
    if (verbose) puts("STUB: NTPClockStartServer called");
    return NULL;
}

void* NTPClockStop(void)
{
    if (verbose) puts("STUB: NTPClockStop called");
    return NULL;
}

void* NTPtoUpTicks(void)
{
    if (verbose) puts("STUB: NTPtoUpTicks called");
    return NULL;
}

void* NanoTimeGetCurrent(void)
{
    if (verbose) puts("STUB: NanoTimeGetCurrent called");
    return NULL;
}

void* NanosecondsToUpTicks(void)
{
    if (verbose) puts("STUB: NanosecondsToUpTicks called");
    return NULL;
}

void* NetPerfCreate(void)
{
    if (verbose) puts("STUB: NetPerfCreate called");
    return NULL;
}

void* NetPerfSetEventHandler(void)
{
    if (verbose) puts("STUB: NetPerfSetEventHandler called");
    return NULL;
}

void* NetSocketChaCha20Poly1305Configure(void)
{
    if (verbose) puts("STUB: NetSocketChaCha20Poly1305Configure called");
    return NULL;
}

void* NetSocketGCM_Configure(void)
{
    if (verbose) puts("STUB: NetSocketGCM_Configure called");
    return NULL;
}

void* NetSocketTLS_Configure(void)
{
    if (verbose) puts("STUB: NetSocketTLS_Configure called");
    return NULL;
}

void* NetSocket_Cancel(void)
{
    if (verbose) puts("STUB: NetSocket_Cancel called");
    return NULL;
}

void* NetSocket_Create(void)
{
    if (verbose) puts("STUB: NetSocket_Create called");
    return NULL;
}

void* NetSocket_CreateWithNative(void)
{
    if (verbose) puts("STUB: NetSocket_CreateWithNative called");
    return NULL;
}

void* NetSocket_Delete(void)
{
    if (verbose) puts("STUB: NetSocket_Delete called");
    return NULL;
}

void* NetSocket_Disconnect(void)
{
    if (verbose) puts("STUB: NetSocket_Disconnect called");
    return NULL;
}

void* NetSocket_GetNative(void)
{
    if (verbose) puts("STUB: NetSocket_GetNative called");
    return NULL;
}

void* NetSocket_HTTPReadHeader(void)
{
    if (verbose) puts("STUB: NetSocket_HTTPReadHeader called");
    return NULL;
}

void* NetSocket_ReadFile(void)
{
    if (verbose) puts("STUB: NetSocket_ReadFile called");
    return NULL;
}

void* NetSocket_Reset(void)
{
    if (verbose) puts("STUB: NetSocket_Reset called");
    return NULL;
}

void* NetSocket_TCPConnect(void)
{
    if (verbose) puts("STUB: NetSocket_TCPConnect called");
    return NULL;
}

void* NetSocket_Wait(void)
{
    if (verbose) puts("STUB: NetSocket_Wait called");
    return NULL;
}

void* NetTransportChaCha20Poly1305Configure(void)
{
    if (verbose) puts("STUB: NetTransportChaCha20Poly1305Configure called");
    return NULL;
}

void* NetTransportGCMConfigure(void)
{
    if (verbose) puts("STUB: NetTransportGCMConfigure called");
    return NULL;
}

void* NetTransportTLSConfigure(void)
{
    if (verbose) puts("STUB: NetTransportTLSConfigure called");
    return NULL;
}

void* NetTransportTLSConfigureEx(void)
{
    if (verbose) puts("STUB: NetTransportTLSConfigureEx called");
    return NULL;
}

void* NetTransportTLSPSKConfigure(void)
{
    if (verbose) puts("STUB: NetTransportTLSPSKConfigure called");
    return NULL;
}

void* NetworkChangeListenerCreate(void)
{
    if (verbose) puts("STUB: NetworkChangeListenerCreate called");
    return NULL;
}

void* NetworkChangeListenerSetDispatchQueue(void)
{
    if (verbose) puts("STUB: NetworkChangeListenerSetDispatchQueue called");
    return NULL;
}

void* NetworkChangeListenerSetHandler(void)
{
    if (verbose) puts("STUB: NetworkChangeListenerSetHandler called");
    return NULL;
}

void* NetworkChangeListenerStart(void)
{
    if (verbose) puts("STUB: NetworkChangeListenerStart called");
    return NULL;
}

void* NetworkChangeListenerStop(void)
{
    if (verbose) puts("STUB: NetworkChangeListenerStop called");
    return NULL;
}

void* NormalizePath(void)
{
    if (verbose) puts("STUB: NormalizePath called");
    return NULL;
}

void* NormalizeUUIDString(void)
{
    if (verbose) puts("STUB: NormalizeUUIDString called");
    return NULL;
}

void* OPACKDecodeBytes(void)
{
    if (verbose) puts("STUB: OPACKDecodeBytes called");
    return NULL;
}

void* OPACKDecodeData(void)
{
    if (verbose) puts("STUB: OPACKDecodeData called");
    return NULL;
}

void* OPACKEncodeObject(void)
{
    if (verbose) puts("STUB: OPACKEncodeObject called");
    return NULL;
}

void* OPACKEncoderCreateData(void)
{
    if (verbose) puts("STUB: OPACKEncoderCreateData called");
    return NULL;
}

void* OPACKEncoderCreateDataMutable(void)
{
    if (verbose) puts("STUB: OPACKEncoderCreateDataMutable called");
    return NULL;
}

void* OPACKEstimateEncodedSize(void)
{
    if (verbose) puts("STUB: OPACKEstimateEncodedSize called");
    return NULL;
}

void* OSX_Apple80211ANQPAppendAppleActionFrameHeader(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPAppendAppleActionFrameHeader called");
    return NULL;
}

void* OSX_Apple80211ANQPAppendGASComebackRequest(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPAppendGASComebackRequest called");
    return NULL;
}

void* OSX_Apple80211ANQPAppendGASInitialRequestHeader(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPAppendGASInitialRequestHeader called");
    return NULL;
}

void* OSX_Apple80211ANQPAppendHS20QueryList(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPAppendHS20QueryList called");
    return NULL;
}

void* OSX_Apple80211ANQPAppendIEEEQueryList(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPAppendIEEEQueryList called");
    return NULL;
}

void* OSX_Apple80211ANQPParse3GPPCellularNetwork(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParse3GPPCellularNetwork called");
    return NULL;
}

void* OSX_Apple80211ANQPParseANQPData(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseANQPData called");
    return NULL;
}

void* OSX_Apple80211ANQPParseActionFrame(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseActionFrame called");
    return NULL;
}

void* OSX_Apple80211ANQPParseCapabilityList(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseCapabilityList called");
    return NULL;
}

void* OSX_Apple80211ANQPParseDomainName(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseDomainName called");
    return NULL;
}

void* OSX_Apple80211ANQPParseGASComebackResponse(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseGASComebackResponse called");
    return NULL;
}

void* OSX_Apple80211ANQPParseGASInitialResponse(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseGASInitialResponse called");
    return NULL;
}

void* OSX_Apple80211ANQPParseHS20CapabilityList(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseHS20CapabilityList called");
    return NULL;
}

void* OSX_Apple80211ANQPParseHS20ConnectionCapability(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseHS20ConnectionCapability called");
    return NULL;
}

void* OSX_Apple80211ANQPParseHS20Element(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseHS20Element called");
    return NULL;
}

void* OSX_Apple80211ANQPParseHS20OperationClassIndication(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseHS20OperationClassIndication called");
    return NULL;
}

void* OSX_Apple80211ANQPParseHS20OperatorFriendlyName(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseHS20OperatorFriendlyName called");
    return NULL;
}

void* OSX_Apple80211ANQPParseHS20WANMetrics(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseHS20WANMetrics called");
    return NULL;
}

void* OSX_Apple80211ANQPParseIPAddressTypeAvailability(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseIPAddressTypeAvailability called");
    return NULL;
}

void* OSX_Apple80211ANQPParseNAIRealm(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseNAIRealm called");
    return NULL;
}

void* OSX_Apple80211ANQPParseNetworkAuthenticationType(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseNetworkAuthenticationType called");
    return NULL;
}

void* OSX_Apple80211ANQPParseRoamingConsortium(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseRoamingConsortium called");
    return NULL;
}

void* OSX_Apple80211ANQPParseVenueName(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseVenueName called");
    return NULL;
}

void* OSX_Apple80211ANQPParseWFAElement(void)
{
    if (verbose) puts("STUB: OSX_Apple80211ANQPParseWFAElement called");
    return NULL;
}

void* OpenSelfConnectedLoopbackSocket(void)
{
    if (verbose) puts("STUB: OpenSelfConnectedLoopbackSocket called");
    return NULL;
}

void* P2PAllocAppleIE(void)
{
    if (verbose) puts("STUB: P2PAllocAppleIE called");
    return NULL;
}

void* P2PAllocComebackResp(void)
{
    if (verbose) puts("STUB: P2PAllocComebackResp called");
    return NULL;
}

void* P2PAllocComebackRespList(void)
{
    if (verbose) puts("STUB: P2PAllocComebackRespList called");
    return NULL;
}

void* P2PAllocGONegReqFrm(void)
{
    if (verbose) puts("STUB: P2PAllocGONegReqFrm called");
    return NULL;
}

void* P2PAllocInitialReqFrm(void)
{
    if (verbose) puts("STUB: P2PAllocInitialReqFrm called");
    return NULL;
}

void* P2PAllocInitialRespFrm(void)
{
    if (verbose) puts("STUB: P2PAllocInitialRespFrm called");
    return NULL;
}

void* P2PAllocInvitationRequest(void)
{
    if (verbose) puts("STUB: P2PAllocInvitationRequest called");
    return NULL;
}

void* P2PAllocInvitationResponse(void)
{
    if (verbose) puts("STUB: P2PAllocInvitationResponse called");
    return NULL;
}

void* P2PAllocServiceDiscoveryResp(void)
{
    if (verbose) puts("STUB: P2PAllocServiceDiscoveryResp called");
    return NULL;
}

void* P2PAllocWPSAssocReqIE(void)
{
    if (verbose) puts("STUB: P2PAllocWPSAssocReqIE called");
    return NULL;
}

void* P2PAllocWPSAssocRespIE(void)
{
    if (verbose) puts("STUB: P2PAllocWPSAssocRespIE called");
    return NULL;
}

void* P2PAllocWPSBeaconIE(void)
{
    if (verbose) puts("STUB: P2PAllocWPSBeaconIE called");
    return NULL;
}

void* P2PAllocWPSProbeReqIE(void)
{
    if (verbose) puts("STUB: P2PAllocWPSProbeReqIE called");
    return NULL;
}

void* P2PAllocWPSProbeResponseIE(void)
{
    if (verbose) puts("STUB: P2PAllocWPSProbeResponseIE called");
    return NULL;
}

void* P2PIEAllocAssocReqIE(void)
{
    if (verbose) puts("STUB: P2PIEAllocAssocReqIE called");
    return NULL;
}

void* P2PIEAllocBeaconIE(void)
{
    if (verbose) puts("STUB: P2PIEAllocBeaconIE called");
    return NULL;
}

void* P2PIEAllocProbeRequestIE(void)
{
    if (verbose) puts("STUB: P2PIEAllocProbeRequestIE called");
    return NULL;
}

void* P2PIEAllocProbeRespIE(void)
{
    if (verbose) puts("STUB: P2PIEAllocProbeRespIE called");
    return NULL;
}

void* P2PIEInitAssocRespIE(void)
{
    if (verbose) puts("STUB: P2PIEInitAssocRespIE called");
    return NULL;
}

void* P2PInitComebackReq(void)
{
    if (verbose) puts("STUB: P2PInitComebackReq called");
    return NULL;
}

void* P2PParseAppleIESubtypeList(void)
{
    if (verbose) puts("STUB: P2PParseAppleIESubtypeList called");
    return NULL;
}

void* P2PParseComebackRequest(void)
{
    if (verbose) puts("STUB: P2PParseComebackRequest called");
    return NULL;
}

void* P2PParseComebackResponse(void)
{
    if (verbose) puts("STUB: P2PParseComebackResponse called");
    return NULL;
}

void* P2PParseComebackResponseFragmentList(void)
{
    if (verbose) puts("STUB: P2PParseComebackResponseFragmentList called");
    return NULL;
}

void* P2PParseGASInitialRequestFrame(void)
{
    if (verbose) puts("STUB: P2PParseGASInitialRequestFrame called");
    return NULL;
}

void* P2PParseGASInitialResponseFrame(void)
{
    if (verbose) puts("STUB: P2PParseGASInitialResponseFrame called");
    return NULL;
}

void* P2PParseInvitationRequest(void)
{
    if (verbose) puts("STUB: P2PParseInvitationRequest called");
    return NULL;
}

void* P2PParseInvitationResponse(void)
{
    if (verbose) puts("STUB: P2PParseInvitationResponse called");
    return NULL;
}

void* P2PParseSubelements(void)
{
    if (verbose) puts("STUB: P2PParseSubelements called");
    return NULL;
}

void* P2PStatusCodeToString(void)
{
    if (verbose) puts("STUB: P2PStatusCodeToString called");
    return NULL;
}

void* PIDInit(void)
{
    if (verbose) puts("STUB: PIDInit called");
    return NULL;
}

void* PIDUpdate(void)
{
    if (verbose) puts("STUB: PIDUpdate called");
    return NULL;
}

void* PairingSessionCopyExtraData(void)
{
    if (verbose) puts("STUB: PairingSessionCopyExtraData called");
    return NULL;
}

void* PairingSessionCopyIdentity(void)
{
    if (verbose) puts("STUB: PairingSessionCopyIdentity called");
    return NULL;
}

void* PairingSessionCopyPairedPeer(void)
{
    if (verbose) puts("STUB: PairingSessionCopyPairedPeer called");
    return NULL;
}

void* PairingSessionCopyPeer(void)
{
    if (verbose) puts("STUB: PairingSessionCopyPeer called");
    return NULL;
}

void* PairingSessionCopyPeerACL(void)
{
    if (verbose) puts("STUB: PairingSessionCopyPeerACL called");
    return NULL;
}

void* PairingSessionCopyPeerIdentifier(void)
{
    if (verbose) puts("STUB: PairingSessionCopyPeerIdentifier called");
    return NULL;
}

void* PairingSessionCopyPeerInfo(void)
{
    if (verbose) puts("STUB: PairingSessionCopyPeerInfo called");
    return NULL;
}

void* PairingSessionCopyPeers(void)
{
    if (verbose) puts("STUB: PairingSessionCopyPeers called");
    return NULL;
}

void* PairingSessionCopyProperty(void)
{
    if (verbose) puts("STUB: PairingSessionCopyProperty called");
    return NULL;
}

void* PairingSessionCreate(void)
{
    if (verbose) puts("STUB: PairingSessionCreate called");
    return NULL;
}

void* PairingSessionDeleteIdentity(void)
{
    if (verbose) puts("STUB: PairingSessionDeleteIdentity called");
    return NULL;
}

void* PairingSessionDeletePeer(void)
{
    if (verbose) puts("STUB: PairingSessionDeletePeer called");
    return NULL;
}

void* PairingSessionDeriveKey(void)
{
    if (verbose) puts("STUB: PairingSessionDeriveKey called");
    return NULL;
}

void* PairingSessionExchange(void)
{
    if (verbose) puts("STUB: PairingSessionExchange called");
    return NULL;
}

void* PairingSessionFindPeer(void)
{
    if (verbose) puts("STUB: PairingSessionFindPeer called");
    return NULL;
}

void* PairingSessionGetPairedPeer(void)
{
    if (verbose) puts("STUB: PairingSessionGetPairedPeer called");
    return NULL;
}

void* PairingSessionGetPeerAppFlags(void)
{
    if (verbose) puts("STUB: PairingSessionGetPeerAppFlags called");
    return NULL;
}

void* PairingSessionGetPeerFlags(void)
{
    if (verbose) puts("STUB: PairingSessionGetPeerFlags called");
    return NULL;
}

void* PairingSessionGetResumeInfo(void)
{
    if (verbose) puts("STUB: PairingSessionGetResumeInfo called");
    return NULL;
}

void* PairingSessionInvalidate(void)
{
    if (verbose) puts("STUB: PairingSessionInvalidate called");
    return NULL;
}

void* PairingSessionSavePeer(void)
{
    if (verbose) puts("STUB: PairingSessionSavePeer called");
    return NULL;
}

void* PairingSessionSetACL(void)
{
    if (verbose) puts("STUB: PairingSessionSetACL called");
    return NULL;
}

void* PairingSessionSetAddPairingHandler(void)
{
    if (verbose) puts("STUB: PairingSessionSetAddPairingHandler called");
    return NULL;
}

void* PairingSessionSetAdditionalPeerInfo(void)
{
    if (verbose) puts("STUB: PairingSessionSetAdditionalPeerInfo called");
    return NULL;
}

void* PairingSessionSetAdditionalSelfInfo(void)
{
    if (verbose) puts("STUB: PairingSessionSetAdditionalSelfInfo called");
    return NULL;
}

void* PairingSessionSetEventHandler(void)
{
    if (verbose) puts("STUB: PairingSessionSetEventHandler called");
    return NULL;
}

void* PairingSessionSetExtraData(void)
{
    if (verbose) puts("STUB: PairingSessionSetExtraData called");
    return NULL;
}

void* PairingSessionSetFlags(void)
{
    if (verbose) puts("STUB: PairingSessionSetFlags called");
    return NULL;
}

void* PairingSessionSetIdentifier(void)
{
    if (verbose) puts("STUB: PairingSessionSetIdentifier called");
    return NULL;
}

void* PairingSessionSetKeychainInfo(void)
{
    if (verbose) puts("STUB: PairingSessionSetKeychainInfo called");
    return NULL;
}

void* PairingSessionSetListPairingsHandler(void)
{
    if (verbose) puts("STUB: PairingSessionSetListPairingsHandler called");
    return NULL;
}

void* PairingSessionSetLogging(void)
{
    if (verbose) puts("STUB: PairingSessionSetLogging called");
    return NULL;
}

void* PairingSessionSetMTU(void)
{
    if (verbose) puts("STUB: PairingSessionSetMTU called");
    return NULL;
}

void* PairingSessionSetMaxResumeSessions(void)
{
    if (verbose) puts("STUB: PairingSessionSetMaxResumeSessions called");
    return NULL;
}

void* PairingSessionSetMaxTries(void)
{
    if (verbose) puts("STUB: PairingSessionSetMaxTries called");
    return NULL;
}

void* PairingSessionSetMyAppleID(void)
{
    if (verbose) puts("STUB: PairingSessionSetMyAppleID called");
    return NULL;
}

void* PairingSessionSetMyAppleIDInfoClient(void)
{
    if (verbose) puts("STUB: PairingSessionSetMyAppleIDInfoClient called");
    return NULL;
}

void* PairingSessionSetPeerAppleID(void)
{
    if (verbose) puts("STUB: PairingSessionSetPeerAppleID called");
    return NULL;
}

void* PairingSessionSetPeerAppleIDs(void)
{
    if (verbose) puts("STUB: PairingSessionSetPeerAppleIDs called");
    return NULL;
}

void* PairingSessionSetProperty(void)
{
    if (verbose) puts("STUB: PairingSessionSetProperty called");
    return NULL;
}

void* PairingSessionSetRemovePairingHandler(void)
{
    if (verbose) puts("STUB: PairingSessionSetRemovePairingHandler called");
    return NULL;
}

void* PairingSessionSetResumeInfo(void)
{
    if (verbose) puts("STUB: PairingSessionSetResumeInfo called");
    return NULL;
}

void* PairingSessionSetResumeTTL(void)
{
    if (verbose) puts("STUB: PairingSessionSetResumeTTL called");
    return NULL;
}

void* PairingSessionSetSelfAppFlags(void)
{
    if (verbose) puts("STUB: PairingSessionSetSelfAppFlags called");
    return NULL;
}

void* PairingSessionSetSetupCode(void)
{
    if (verbose) puts("STUB: PairingSessionSetSetupCode called");
    return NULL;
}

void* PairingSessionSetSignBytesHandler(void)
{
    if (verbose) puts("STUB: PairingSessionSetSignBytesHandler called");
    return NULL;
}

void* PairingSessionSetSignBytesHandler_b(void)
{
    if (verbose) puts("STUB: PairingSessionSetSignBytesHandler_b called");
    return NULL;
}

void* PairingSessionSetVerifySignatureHandler_b(void)
{
    if (verbose) puts("STUB: PairingSessionSetVerifySignatureHandler_b called");
    return NULL;
}

void* PairingSessionUpdatePeerInfo(void)
{
    if (verbose) puts("STUB: PairingSessionUpdatePeerInfo called");
    return NULL;
}

void* Parity32(void)
{
    if (verbose) puts("STUB: Parity32 called");
    return NULL;
}

void* ParseCommandLineIntoArgV(void)
{
    if (verbose) puts("STUB: ParseCommandLineIntoArgV called");
    return NULL;
}

void* ParseFractionalDateString(void)
{
    if (verbose) puts("STUB: ParseFractionalDateString called");
    return NULL;
}

void* ParseLine(void)
{
    if (verbose) puts("STUB: ParseLine called");
    return NULL;
}

void* PrintFCore(void)
{
    if (verbose) puts("STUB: PrintFCore called");
    return NULL;
}

void* PrintFCoreVAList(void)
{
    if (verbose) puts("STUB: PrintFCoreVAList called");
    return NULL;
}

void* PrintFDeregisterExtension(void)
{
    if (verbose) puts("STUB: PrintFDeregisterExtension called");
    return NULL;
}

void* PrintFRegisterExtension(void)
{
    if (verbose) puts("STUB: PrintFRegisterExtension called");
    return NULL;
}

void* RandomBytes(void)
{
    if (verbose) puts("STUB: RandomBytes called");
    return NULL;
}

void* RandomString(void)
{
    if (verbose) puts("STUB: RandomString called");
    return NULL;
}

void* ReadANSIFile(void)
{
    if (verbose) puts("STUB: ReadANSIFile called");
    return NULL;
}

void* RegexMatch(void)
{
    if (verbose) puts("STUB: RegexMatch called");
    return NULL;
}

void* ReleaseUsableInterfaceList(void)
{
    if (verbose) puts("STUB: ReleaseUsableInterfaceList called");
    return NULL;
}

void* RemovePath(void)
{
    if (verbose) puts("STUB: RemovePath called");
    return NULL;
}

void* ReplaceDifferentString(void)
{
    if (verbose) puts("STUB: ReplaceDifferentString called");
    return NULL;
}

void* ReplaceString(void)
{
    if (verbose) puts("STUB: ReplaceString called");
    return NULL;
}

void* RollLogFiles(void)
{
    if (verbose) puts("STUB: RollLogFiles called");
    return NULL;
}

void* RunProcessAndCaptureOutput(void)
{
    if (verbose) puts("STUB: RunProcessAndCaptureOutput called");
    return NULL;
}

void* RunProcessAndCaptureOutputEx(void)
{
    if (verbose) puts("STUB: RunProcessAndCaptureOutputEx called");
    return NULL;
}

void* SDPFindAttribute(void)
{
    if (verbose) puts("STUB: SDPFindAttribute called");
    return NULL;
}

void* SDPFindMediaSection(void)
{
    if (verbose) puts("STUB: SDPFindMediaSection called");
    return NULL;
}

void* SDPFindParameter(void)
{
    if (verbose) puts("STUB: SDPFindParameter called");
    return NULL;
}

void* SDPFindSessionSection(void)
{
    if (verbose) puts("STUB: SDPFindSessionSection called");
    return NULL;
}

void* SDPFindType(void)
{
    if (verbose) puts("STUB: SDPFindType called");
    return NULL;
}

void* SDPGetNext(void)
{
    if (verbose) puts("STUB: SDPGetNext called");
    return NULL;
}

void* SDPGetNextAttribute(void)
{
    if (verbose) puts("STUB: SDPGetNextAttribute called");
    return NULL;
}

void* SNPrintF(void)
{
    if (verbose) puts("STUB: SNPrintF called");
    return NULL;
}

void* SNPrintF_Add(void)
{
    if (verbose) puts("STUB: SNPrintF_Add called");
    return NULL;
}

void* SNScanF(void)
{
    if (verbose) puts("STUB: SNScanF called");
    return NULL;
}

void* SRPClientStart_corecrypto(void)
{
    if (verbose) puts("STUB: SRPClientStart_corecrypto called");
    return NULL;
}

void* SRPClientStart_libsrp(void)
{
    if (verbose) puts("STUB: SRPClientStart_libsrp called");
    return NULL;
}

void* SRPClientVerify_corecrypto(void)
{
    if (verbose) puts("STUB: SRPClientVerify_corecrypto called");
    return NULL;
}

void* SRPClientVerify_libsrp(void)
{
    if (verbose) puts("STUB: SRPClientVerify_libsrp called");
    return NULL;
}

void* SRPCreate_corecrypto(void)
{
    if (verbose) puts("STUB: SRPCreate_corecrypto called");
    return NULL;
}

void* SRPCreate_libsrp(void)
{
    if (verbose) puts("STUB: SRPCreate_libsrp called");
    return NULL;
}

void* SRPDelete_corecrypto(void)
{
    if (verbose) puts("STUB: SRPDelete_corecrypto called");
    return NULL;
}

void* SRPDelete_libsrp(void)
{
    if (verbose) puts("STUB: SRPDelete_libsrp called");
    return NULL;
}

void* SRPServerStart_corecrypto(void)
{
    if (verbose) puts("STUB: SRPServerStart_corecrypto called");
    return NULL;
}

void* SRPServerStart_libsrp(void)
{
    if (verbose) puts("STUB: SRPServerStart_libsrp called");
    return NULL;
}

void* SRPServerVerify_corecrypto(void)
{
    if (verbose) puts("STUB: SRPServerVerify_corecrypto called");
    return NULL;
}

void* SRPServerVerify_libsrp(void)
{
    if (verbose) puts("STUB: SRPServerVerify_libsrp called");
    return NULL;
}

void* SaveScreenshot(void)
{
    if (verbose) puts("STUB: SaveScreenshot called");
    return NULL;
}

void* ScreenCopyAllScreens(void)
{
    if (verbose) puts("STUB: ScreenCopyAllScreens called");
    return NULL;
}

void* ScreenCopyMain(void)
{
    if (verbose) puts("STUB: ScreenCopyMain called");
    return NULL;
}

void* ScreenCreate(void)
{
    if (verbose) puts("STUB: ScreenCreate called");
    return NULL;
}

void* ScreenDeregister(void)
{
    if (verbose) puts("STUB: ScreenDeregister called");
    return NULL;
}

void* ScreenGetTypeID(void)
{
    if (verbose) puts("STUB: ScreenGetTypeID called");
    return NULL;
}

void* ScreenRegister(void)
{
    if (verbose) puts("STUB: ScreenRegister called");
    return NULL;
}

void* ScreenStreamCreate(void)
{
    if (verbose) puts("STUB: ScreenStreamCreate called");
    return NULL;
}

void* ScreenStreamProcessData(void)
{
    if (verbose) puts("STUB: ScreenStreamProcessData called");
    return NULL;
}

void* ScreenStreamSetFrameHook(void)
{
    if (verbose) puts("STUB: ScreenStreamSetFrameHook called");
    return NULL;
}

void* ScreenStreamStart(void)
{
    if (verbose) puts("STUB: ScreenStreamStart called");
    return NULL;
}

void* ScreenStreamStop(void)
{
    if (verbose) puts("STUB: ScreenStreamStop called");
    return NULL;
}

void* SecondsToUpTicks(void)
{
    if (verbose) puts("STUB: SecondsToUpTicks called");
    return NULL;
}

void* SecondsToUpTicksF(void)
{
    if (verbose) puts("STUB: SecondsToUpTicksF called");
    return NULL;
}

void* SecondsToYMD_HMS(void)
{
    if (verbose) puts("STUB: SecondsToYMD_HMS called");
    return NULL;
}

void* SendSelfConnectedLoopbackMessage(void)
{
    if (verbose) puts("STUB: SendSelfConnectedLoopbackMessage called");
    return NULL;
}

void* SerialStreamCreate(void)
{
    if (verbose) puts("STUB: SerialStreamCreate called");
    return NULL;
}

void* SerialStreamGetTypeID(void)
{
    if (verbose) puts("STUB: SerialStreamGetTypeID called");
    return NULL;
}

void* SerialStreamInvalidate(void)
{
    if (verbose) puts("STUB: SerialStreamInvalidate called");
    return NULL;
}

void* SerialStreamRead(void)
{
    if (verbose) puts("STUB: SerialStreamRead called");
    return NULL;
}

void* SerialStreamReadLine(void)
{
    if (verbose) puts("STUB: SerialStreamReadLine called");
    return NULL;
}

void* SerialStreamReadLineSync(void)
{
    if (verbose) puts("STUB: SerialStreamReadLineSync called");
    return NULL;
}

void* SerialStreamReadSync(void)
{
    if (verbose) puts("STUB: SerialStreamReadSync called");
    return NULL;
}

void* SerialStreamSetConfig(void)
{
    if (verbose) puts("STUB: SerialStreamSetConfig called");
    return NULL;
}

void* SerialStreamSetDispatchQueue(void)
{
    if (verbose) puts("STUB: SerialStreamSetDispatchQueue called");
    return NULL;
}

void* SerialStreamWrite(void)
{
    if (verbose) puts("STUB: SerialStreamWrite called");
    return NULL;
}

void* SerialStreamWriteSync(void)
{
    if (verbose) puts("STUB: SerialStreamWriteSync called");
    return NULL;
}

void* ServerSocketOpen(void)
{
    if (verbose) puts("STUB: ServerSocketOpen called");
    return NULL;
}

void* ServerSocketOpenEx(void)
{
    if (verbose) puts("STUB: ServerSocketOpenEx called");
    return NULL;
}

void* ServerSocketOpenEx2(void)
{
    if (verbose) puts("STUB: ServerSocketOpenEx2 called");
    return NULL;
}

void* ServerSocketPairOpen(void)
{
    if (verbose) puts("STUB: ServerSocketPairOpen called");
    return NULL;
}

void* SetCurrentThreadPriority(void)
{
    if (verbose) puts("STUB: SetCurrentThreadPriority called");
    return NULL;
}

void* SimpleArrayAppendItem(void)
{
    if (verbose) puts("STUB: SimpleArrayAppendItem called");
    return NULL;
}

void* SimpleArrayRemoveItemAtIndex(void)
{
    if (verbose) puts("STUB: SimpleArrayRemoveItemAtIndex called");
    return NULL;
}

void* SineTable_Create(void)
{
    if (verbose) puts("STUB: SineTable_Create called");
    return NULL;
}

void* SineTable_Delete(void)
{
    if (verbose) puts("STUB: SineTable_Delete called");
    return NULL;
}

void* SineTable_GetSamples(void)
{
    if (verbose) puts("STUB: SineTable_GetSamples called");
    return NULL;
}

void* SipHash(void)
{
    if (verbose) puts("STUB: SipHash called");
    return NULL;
}

void* SleepForUpTicks(void)
{
    if (verbose) puts("STUB: SleepForUpTicks called");
    return NULL;
}

void* SleepUntilUpTicks(void)
{
    if (verbose) puts("STUB: SleepUntilUpTicks called");
    return NULL;
}

void* SockAddrCompareAddr(void)
{
    if (verbose) puts("STUB: SockAddrCompareAddr called");
    return NULL;
}

void* SockAddrConvertToIPv6(void)
{
    if (verbose) puts("STUB: SockAddrConvertToIPv6 called");
    return NULL;
}

void* SockAddrCopy(void)
{
    if (verbose) puts("STUB: SockAddrCopy called");
    return NULL;
}

void* SockAddrGetPort(void)
{
    if (verbose) puts("STUB: SockAddrGetPort called");
    return NULL;
}

void* SockAddrGetSize(void)
{
    if (verbose) puts("STUB: SockAddrGetSize called");
    return NULL;
}

void* SockAddrSetPort(void)
{
    if (verbose) puts("STUB: SockAddrSetPort called");
    return NULL;
}

void* SockAddrSimplify(void)
{
    if (verbose) puts("STUB: SockAddrSimplify called");
    return NULL;
}

void* SockAddrToDeviceID(void)
{
    if (verbose) puts("STUB: SockAddrToDeviceID called");
    return NULL;
}

void* SockAddrToString(void)
{
    if (verbose) puts("STUB: SockAddrToString called");
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

void* SocketGetInterfaceInfo(void)
{
    if (verbose) puts("STUB: SocketGetInterfaceInfo called");
    return NULL;
}

void* SocketJoinMulticast(void)
{
    if (verbose) puts("STUB: SocketJoinMulticast called");
    return NULL;
}

void* SocketLeaveMulticast(void)
{
    if (verbose) puts("STUB: SocketLeaveMulticast called");
    return NULL;
}

void* SocketReadData(void)
{
    if (verbose) puts("STUB: SocketReadData called");
    return NULL;
}

void* SocketReadDataEx(void)
{
    if (verbose) puts("STUB: SocketReadDataEx called");
    return NULL;
}

void* SocketRecvFrom(void)
{
    if (verbose) puts("STUB: SocketRecvFrom called");
    return NULL;
}

void* SocketSetBoundInterface(void)
{
    if (verbose) puts("STUB: SocketSetBoundInterface called");
    return NULL;
}

void* SocketSetBufferSize(void)
{
    if (verbose) puts("STUB: SocketSetBufferSize called");
    return NULL;
}

void* SocketSetKeepAlive(void)
{
    if (verbose) puts("STUB: SocketSetKeepAlive called");
    return NULL;
}

void* SocketSetKeepAliveEx(void)
{
    if (verbose) puts("STUB: SocketSetKeepAliveEx called");
    return NULL;
}

void* SocketSetMulticastInterface(void)
{
    if (verbose) puts("STUB: SocketSetMulticastInterface called");
    return NULL;
}

void* SocketSetNonBlocking(void)
{
    if (verbose) puts("STUB: SocketSetNonBlocking called");
    return NULL;
}

void* SocketSetNotSentLowWatermark(void)
{
    if (verbose) puts("STUB: SocketSetNotSentLowWatermark called");
    return NULL;
}

void* SocketSetP2P(void)
{
    if (verbose) puts("STUB: SocketSetP2P called");
    return NULL;
}

void* SocketSetPacketTimestamps(void)
{
    if (verbose) puts("STUB: SocketSetPacketTimestamps called");
    return NULL;
}

void* SocketSetQoS(void)
{
    if (verbose) puts("STUB: SocketSetQoS called");
    return NULL;
}

void* SocketSetTCPTimeout(void)
{
    if (verbose) puts("STUB: SocketSetTCPTimeout called");
    return NULL;
}

void* SocketTransportRead(void)
{
    if (verbose) puts("STUB: SocketTransportRead called");
    return NULL;
}

void* SocketWriteData(void)
{
    if (verbose) puts("STUB: SocketWriteData called");
    return NULL;
}

void* SourceVersionToCString(void)
{
    if (verbose) puts("STUB: SourceVersionToCString called");
    return NULL;
}

void* StringArray_Free(void)
{
    if (verbose) puts("STUB: StringArray_Free called");
    return NULL;
}

void* StringToSockAddr(void)
{
    if (verbose) puts("STUB: StringToSockAddr called");
    return NULL;
}

void* StringToUUID(void)
{
    if (verbose) puts("STUB: StringToUUID called");
    return NULL;
}

void* StringToUUIDEx(void)
{
    if (verbose) puts("STUB: StringToUUIDEx called");
    return NULL;
}

void* Swap16Mem(void)
{
    if (verbose) puts("STUB: Swap16Mem called");
    return NULL;
}

void* SwitchToMobileUser(void)
{
    if (verbose) puts("STUB: SwitchToMobileUser called");
    return NULL;
}

void* TDSBloomFilterAddHash(void)
{
    if (verbose) puts("STUB: TDSBloomFilterAddHash called");
    return NULL;
}

void* TDSBloomFilterAddString(void)
{
    if (verbose) puts("STUB: TDSBloomFilterAddString called");
    return NULL;
}

void* TDSBloomFilterContainsHash(void)
{
    if (verbose) puts("STUB: TDSBloomFilterContainsHash called");
    return NULL;
}

void* TDSBloomFilterContainsString(void)
{
    if (verbose) puts("STUB: TDSBloomFilterContainsString called");
    return NULL;
}

void* TLV8BufferAppend(void)
{
    if (verbose) puts("STUB: TLV8BufferAppend called");
    return NULL;
}

void* TLV8BufferAppendSInt64(void)
{
    if (verbose) puts("STUB: TLV8BufferAppendSInt64 called");
    return NULL;
}

void* TLV8BufferAppendUInt64(void)
{
    if (verbose) puts("STUB: TLV8BufferAppendUInt64 called");
    return NULL;
}

void* TLV8BufferDetach(void)
{
    if (verbose) puts("STUB: TLV8BufferDetach called");
    return NULL;
}

void* TLV8BufferFree(void)
{
    if (verbose) puts("STUB: TLV8BufferFree called");
    return NULL;
}

void* TLV8BufferInit(void)
{
    if (verbose) puts("STUB: TLV8BufferInit called");
    return NULL;
}

void* TLV8Get(void)
{
    if (verbose) puts("STUB: TLV8Get called");
    return NULL;
}

void* TLV8GetBytes(void)
{
    if (verbose) puts("STUB: TLV8GetBytes called");
    return NULL;
}

void* TLV8GetNext(void)
{
    if (verbose) puts("STUB: TLV8GetNext called");
    return NULL;
}

void* TLV8GetOrCopyCoalesced(void)
{
    if (verbose) puts("STUB: TLV8GetOrCopyCoalesced called");
    return NULL;
}

void* TLV8GetUInt64(void)
{
    if (verbose) puts("STUB: TLV8GetUInt64 called");
    return NULL;
}

void* TLVParseSInt64(void)
{
    if (verbose) puts("STUB: TLVParseSInt64 called");
    return NULL;
}

void* TLVParseUInt64(void)
{
    if (verbose) puts("STUB: TLVParseUInt64 called");
    return NULL;
}

void* TN6sBSnEP2(void)
{
    if (verbose) puts("STUB: TN6sBSnEP2 called");
    return NULL;
}

void* TUConvertToJUnit(void)
{
    if (verbose) puts("STUB: TUConvertToJUnit called");
    return NULL;
}

void* TUFaultDisable(void)
{
    if (verbose) puts("STUB: TUFaultDisable called");
    return NULL;
}

void* TUFaultEnable(void)
{
    if (verbose) puts("STUB: TUFaultEnable called");
    return NULL;
}

void* TUFaultInject(void)
{
    if (verbose) puts("STUB: TUFaultInject called");
    return NULL;
}

void* TUFinalize(void)
{
    if (verbose) puts("STUB: TUFinalize called");
    return NULL;
}

void* TUInitialize(void)
{
    if (verbose) puts("STUB: TUInitialize called");
    return NULL;
}

void* TULogF(void)
{
    if (verbose) puts("STUB: TULogF called");
    return NULL;
}

void* TULogLevelEnabled(void)
{
    if (verbose) puts("STUB: TULogLevelEnabled called");
    return NULL;
}

void* TULogV(void)
{
    if (verbose) puts("STUB: TULogV called");
    return NULL;
}

void* TUSetExpectedTestCount(void)
{
    if (verbose) puts("STUB: TUSetExpectedTestCount called");
    return NULL;
}

void* TUTestRequire(void)
{
    if (verbose) puts("STUB: TUTestRequire called");
    return NULL;
}

void* TUTestRequireNoErr(void)
{
    if (verbose) puts("STUB: TUTestRequireNoErr called");
    return NULL;
}

void* TUTestRequireNoNSError(void)
{
    if (verbose) puts("STUB: TUTestRequireNoNSError called");
    return NULL;
}

void* TUTestStateCreate(void)
{
    if (verbose) puts("STUB: TUTestStateCreate called");
    return NULL;
}

void* TUTestStateGetObject(void)
{
    if (verbose) puts("STUB: TUTestStateGetObject called");
    return NULL;
}

void* TUTestStateGetSInt32(void)
{
    if (verbose) puts("STUB: TUTestStateGetSInt32 called");
    return NULL;
}

void* TUTestStateSetObject(void)
{
    if (verbose) puts("STUB: TUTestStateSetObject called");
    return NULL;
}

void* TUTestStateSetSInt32(void)
{
    if (verbose) puts("STUB: TUTestStateSetSInt32 called");
    return NULL;
}

void* TUTestStateSignal(void)
{
    if (verbose) puts("STUB: TUTestStateSignal called");
    return NULL;
}

void* TUTestStateSignalCount(void)
{
    if (verbose) puts("STUB: TUTestStateSignalCount called");
    return NULL;
}

void* TUTestStateUpdateSInt32(void)
{
    if (verbose) puts("STUB: TUTestStateUpdateSInt32 called");
    return NULL;
}

void* TUTestStateWait(void)
{
    if (verbose) puts("STUB: TUTestStateWait called");
    return NULL;
}

void* TXTRecordCreateWithDictionary(void)
{
    if (verbose) puts("STUB: TXTRecordCreateWithDictionary called");
    return NULL;
}

void* TXTRecordGetNextItem(void)
{
    if (verbose) puts("STUB: TXTRecordGetNextItem called");
    return NULL;
}

void* TextFindColumn(void)
{
    if (verbose) puts("STUB: TextFindColumn called");
    return NULL;
}

void* TextSep(void)
{
    if (verbose) puts("STUB: TextSep called");
    return NULL;
}

void* TextToFourCharCode(void)
{
    if (verbose) puts("STUB: TextToFourCharCode called");
    return NULL;
}

void* TextToHardwareAddress(void)
{
    if (verbose) puts("STUB: TextToHardwareAddress called");
    return NULL;
}

void* TextToHardwareAddressScalar(void)
{
    if (verbose) puts("STUB: TextToHardwareAddressScalar called");
    return NULL;
}

void* TextToInt32(void)
{
    if (verbose) puts("STUB: TextToInt32 called");
    return NULL;
}

void* TextToSourceVersion(void)
{
    if (verbose) puts("STUB: TextToSourceVersion called");
    return NULL;
}

void* TranslateValue(void)
{
    if (verbose) puts("STUB: TranslateValue called");
    return NULL;
}

void* TruncateUTF8(void)
{
    if (verbose) puts("STUB: TruncateUTF8 called");
    return NULL;
}

void* UDPClientSocketOpen(void)
{
    if (verbose) puts("STUB: UDPClientSocketOpen called");
    return NULL;
}

void* URLGetNextPathSegment(void)
{
    if (verbose) puts("STUB: URLGetNextPathSegment called");
    return NULL;
}

void* URLGetOrCopyNextVariable(void)
{
    if (verbose) puts("STUB: URLGetOrCopyNextVariable called");
    return NULL;
}

void* URLGetOrCopyVariable(void)
{
    if (verbose) puts("STUB: URLGetOrCopyVariable called");
    return NULL;
}

void* URLGetVariable(void)
{
    if (verbose) puts("STUB: URLGetVariable called");
    return NULL;
}

void* URLParseComponents(void)
{
    if (verbose) puts("STUB: URLParseComponents called");
    return NULL;
}

void* UUIDGet(void)
{
    if (verbose) puts("STUB: UUIDGet called");
    return NULL;
}

void* UUIDGetPerSystemHost(void)
{
    if (verbose) puts("STUB: UUIDGetPerSystemHost called");
    return NULL;
}

void* UUIDGetPerUserHost(void)
{
    if (verbose) puts("STUB: UUIDGetPerUserHost called");
    return NULL;
}

void* UUIDtoCString(void)
{
    if (verbose) puts("STUB: UUIDtoCString called");
    return NULL;
}

void* UUIDtoCStringEx(void)
{
    if (verbose) puts("STUB: UUIDtoCStringEx called");
    return NULL;
}

void* UUIDtoCStringFlags(void)
{
    if (verbose) puts("STUB: UUIDtoCStringFlags called");
    return NULL;
}

void* UpTicksPerSecond(void)
{
    if (verbose) puts("STUB: UpTicksPerSecond called");
    return NULL;
}

void* UpTicksToMicroseconds(void)
{
    if (verbose) puts("STUB: UpTicksToMicroseconds called");
    return NULL;
}

void* UpTicksToMilliseconds(void)
{
    if (verbose) puts("STUB: UpTicksToMilliseconds called");
    return NULL;
}

void* UpTicksToNTP(void)
{
    if (verbose) puts("STUB: UpTicksToNTP called");
    return NULL;
}

void* UpTicksToNanoseconds(void)
{
    if (verbose) puts("STUB: UpTicksToNanoseconds called");
    return NULL;
}

void* UpTicksToSeconds(void)
{
    if (verbose) puts("STUB: UpTicksToSeconds called");
    return NULL;
}

void* UpTicksToSecondsF(void)
{
    if (verbose) puts("STUB: UpTicksToSecondsF called");
    return NULL;
}

void* UpdateIOVec(void)
{
    if (verbose) puts("STUB: UpdateIOVec called");
    return NULL;
}

void* VASPrintF(void)
{
    if (verbose) puts("STUB: VASPrintF called");
    return NULL;
}

void* VCPrintF(void)
{
    if (verbose) puts("STUB: VCPrintF called");
    return NULL;
}

void* VFPrintF(void)
{
    if (verbose) puts("STUB: VFPrintF called");
    return NULL;
}

void* VSNPrintF(void)
{
    if (verbose) puts("STUB: VSNPrintF called");
    return NULL;
}

void* VSNScanF(void)
{
    if (verbose) puts("STUB: VSNScanF called");
    return NULL;
}

void* ValueSmootherFree(void)
{
    if (verbose) puts("STUB: ValueSmootherFree called");
    return NULL;
}

void* ValueSmootherGet(void)
{
    if (verbose) puts("STUB: ValueSmootherGet called");
    return NULL;
}

void* ValueSmootherInit(void)
{
    if (verbose) puts("STUB: ValueSmootherInit called");
    return NULL;
}

void* ValueSmootherUpdate(void)
{
    if (verbose) puts("STUB: ValueSmootherUpdate called");
    return NULL;
}

void* VarIntDecode64(void)
{
    if (verbose) puts("STUB: VarIntDecode64 called");
    return NULL;
}

void* VarIntEncode64(void)
{
    if (verbose) puts("STUB: VarIntEncode64 called");
    return NULL;
}

void* WPAHashPSK(void)
{
    if (verbose) puts("STUB: WPAHashPSK called");
    return NULL;
}

void* WiFiCopyCurrentNetworkInfo(void)
{
    if (verbose) puts("STUB: WiFiCopyCurrentNetworkInfo called");
    return NULL;
}

void* WiFiCopyCurrentNetworkInfoEx(void)
{
    if (verbose) puts("STUB: WiFiCopyCurrentNetworkInfoEx called");
    return NULL;
}

void* WiFiCopyNetworkInfo(void)
{
    if (verbose) puts("STUB: WiFiCopyNetworkInfo called");
    return NULL;
}

void* WiFiJoinNetwork(void)
{
    if (verbose) puts("STUB: WiFiJoinNetwork called");
    return NULL;
}

void* WiFiJoinNetwork_b(void)
{
    if (verbose) puts("STUB: WiFiJoinNetwork_b called");
    return NULL;
}

void* WiFiManagerCreate(void)
{
    if (verbose) puts("STUB: WiFiManagerCreate called");
    return NULL;
}

void* WiFiManagerDoApple80211(void)
{
    if (verbose) puts("STUB: WiFiManagerDoApple80211 called");
    return NULL;
}

void* WiFiManagerGetInt64(void)
{
    if (verbose) puts("STUB: WiFiManagerGetInt64 called");
    return NULL;
}

void* WiFiManagerSetProperty(void)
{
    if (verbose) puts("STUB: WiFiManagerSetProperty called");
    return NULL;
}

void* WiFiRestoreNetwork(void)
{
    if (verbose) puts("STUB: WiFiRestoreNetwork called");
    return NULL;
}

void* WiFiSWAPStart(void)
{
    if (verbose) puts("STUB: WiFiSWAPStart called");
    return NULL;
}

void* WiFiSWAPStop(void)
{
    if (verbose) puts("STUB: WiFiSWAPStop called");
    return NULL;
}

void* WiFiScan(void)
{
    if (verbose) puts("STUB: WiFiScan called");
    return NULL;
}

void* WiFiScan_b(void)
{
    if (verbose) puts("STUB: WiFiScan_b called");
    return NULL;
}

void* WriteANSIFile(void)
{
    if (verbose) puts("STUB: WriteANSIFile called");
    return NULL;
}

void* XMLDocumentCreateData(void)
{
    if (verbose) puts("STUB: XMLDocumentCreateData called");
    return NULL;
}

void* XMLDocumentCreateWithBytes(void)
{
    if (verbose) puts("STUB: XMLDocumentCreateWithBytes called");
    return NULL;
}

void* XMLNodeFindNamedAttribute(void)
{
    if (verbose) puts("STUB: XMLNodeFindNamedAttribute called");
    return NULL;
}

void* XMLNodeGetChildren(void)
{
    if (verbose) puts("STUB: XMLNodeGetChildren called");
    return NULL;
}

void* XMLNodeGetDataBytePtr(void)
{
    if (verbose) puts("STUB: XMLNodeGetDataBytePtr called");
    return NULL;
}

void* XMLNodeGetNamePtr(void)
{
    if (verbose) puts("STUB: XMLNodeGetNamePtr called");
    return NULL;
}

void* XMLNodeGetNextNode(void)
{
    if (verbose) puts("STUB: XMLNodeGetNextNode called");
    return NULL;
}

void* XMLNodeSetDataBytes(void)
{
    if (verbose) puts("STUB: XMLNodeSetDataBytes called");
    return NULL;
}

void* _AudioStreamCopyProperty(void)
{
    if (verbose) puts("STUB: _AudioStreamCopyProperty called");
    return NULL;
}

void* _AudioStreamSetProperty(void)
{
    if (verbose) puts("STUB: _AudioStreamSetProperty called");
    return NULL;
}

void* _CLIHelpCommand(void)
{
    if (verbose) puts("STUB: _CLIHelpCommand called");
    return NULL;
}

void* _CLIHelpOption(void)
{
    if (verbose) puts("STUB: _CLIHelpOption called");
    return NULL;
}

void* _CLIInteractiveOption(void)
{
    if (verbose) puts("STUB: _CLIInteractiveOption called");
    return NULL;
}

void* _CLIVersionCommand(void)
{
    if (verbose) puts("STUB: _CLIVersionCommand called");
    return NULL;
}

void* _CLIVersionOption(void)
{
    if (verbose) puts("STUB: _CLIVersionOption called");
    return NULL;
}

void* _HTTPClientSetProperty(void)
{
    if (verbose) puts("STUB: _HTTPClientSetProperty called");
    return NULL;
}

void* _HTTPServerControl(void)
{
    if (verbose) puts("STUB: _HTTPServerControl called");
    return NULL;
}

void* _HTTPServerSetProperty(void)
{
    if (verbose) puts("STUB: _HTTPServerSetProperty called");
    return NULL;
}

void* _LogCategory_Initialize(void)
{
    if (verbose) puts("STUB: _LogCategory_Initialize called");
    return NULL;
}

void* _NetPerfControl(void)
{
    if (verbose) puts("STUB: _NetPerfControl called");
    return NULL;
}

void* _ScreenCopyProperty(void)
{
    if (verbose) puts("STUB: _ScreenCopyProperty called");
    return NULL;
}

void* _ScreenSetProperty(void)
{
    if (verbose) puts("STUB: _ScreenSetProperty called");
    return NULL;
}

void* _ScreenStreamSetProperty(void)
{
    if (verbose) puts("STUB: _ScreenStreamSetProperty called");
    return NULL;
}

void* _TUPerformTest(void)
{
    if (verbose) puts("STUB: _TUPerformTest called");
    return NULL;
}

void* _TUXCTestSetup(void)
{
    if (verbose) puts("STUB: _TUXCTestSetup called");
    return NULL;
}

void* _allocGroupInfoDict(void)
{
    if (verbose) puts("STUB: _allocGroupInfoDict called");
    return NULL;
}

void* _allocP2PInterfaceDict(void)
{
    if (verbose) puts("STUB: _allocP2PInterfaceDict called");
    return NULL;
}

void* _dispatchBGScanCachedNetworks(void)
{
    if (verbose) puts("STUB: _dispatchBGScanCachedNetworks called");
    return NULL;
}

void* _dispatchBssSteeringRequest(void)
{
    if (verbose) puts("STUB: _dispatchBssSteeringRequest called");
    return NULL;
}

void* _dispatchGasResults(void)
{
    if (verbose) puts("STUB: _dispatchGasResults called");
    return NULL;
}

void* _dispatchLeakyStatsEvent(void)
{
    if (verbose) puts("STUB: _dispatchLeakyStatsEvent called");
    return NULL;
}

void* _dispatchP2PNearbyLogTrigger(void)
{
    if (verbose) puts("STUB: _dispatchP2PNearbyLogTrigger called");
    return NULL;
}

void* _dispatchRangingResults(void)
{
    if (verbose) puts("STUB: _dispatchRangingResults called");
    return NULL;
}

void* _eventRead(void)
{
    if (verbose) puts("STUB: _eventRead called");
    return NULL;
}

void* _getRangingPeerRequest(void)
{
    if (verbose) puts("STUB: _getRangingPeerRequest called");
    return NULL;
}

void* _getRangingPhyErrorString(void)
{
    if (verbose) puts("STUB: _getRangingPhyErrorString called");
    return NULL;
}

void* _handleNANGetRequest(void)
{
    if (verbose) puts("STUB: _handleNANGetRequest called");
    return NULL;
}

void* _handleNANSetRequest(void)
{
    if (verbose) puts("STUB: _handleNANSetRequest called");
    return NULL;
}

void* _parseHs20OperatorName(void)
{
    if (verbose) puts("STUB: _parseHs20OperatorName called");
    return NULL;
}

void* apIsApple(void)
{
    if (verbose) puts("STUB: apIsApple called");
    return NULL;
}

void* asl_init(void)
{
    if (verbose) puts("STUB: asl_init called");
    return NULL;
}

void* asl_stop(void)
{
    if (verbose) puts("STUB: asl_stop called");
    return NULL;
}

void* cced25519_make_key_pair_compat(void)
{
    if (verbose) puts("STUB: cced25519_make_key_pair_compat called");
    return NULL;
}

void* cced25519_sign_compat(void)
{
    if (verbose) puts("STUB: cced25519_sign_compat called");
    return NULL;
}

void* cced25519_verify_compat(void)
{
    if (verbose) puts("STUB: cced25519_verify_compat called");
    return NULL;
}

void* chacha20_all_64x64(void)
{
    if (verbose) puts("STUB: chacha20_all_64x64 called");
    return NULL;
}

void* chacha20_all_96x32(void)
{
    if (verbose) puts("STUB: chacha20_all_96x32 called");
    return NULL;
}

void* chacha20_final(void)
{
    if (verbose) puts("STUB: chacha20_final called");
    return NULL;
}

void* chacha20_init_64x64(void)
{
    if (verbose) puts("STUB: chacha20_init_64x64 called");
    return NULL;
}

void* chacha20_poly1305_add_aad(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_add_aad called");
    return NULL;
}

void* chacha20_poly1305_decrypt(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_decrypt called");
    return NULL;
}

void* chacha20_poly1305_decrypt_all_64x64(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_decrypt_all_64x64 called");
    return NULL;
}

void* chacha20_poly1305_decrypt_all_64x64_ex(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_decrypt_all_64x64_ex called");
    return NULL;
}

void* chacha20_poly1305_decrypt_all_96x32(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_decrypt_all_96x32 called");
    return NULL;
}

void* chacha20_poly1305_decrypt_all_96x32_ex(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_decrypt_all_96x32_ex called");
    return NULL;
}

void* chacha20_poly1305_encrypt(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_encrypt called");
    return NULL;
}

void* chacha20_poly1305_encrypt_all_64x64(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_encrypt_all_64x64 called");
    return NULL;
}

void* chacha20_poly1305_encrypt_all_96x32(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_encrypt_all_96x32 called");
    return NULL;
}

void* chacha20_poly1305_final(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_final called");
    return NULL;
}

void* chacha20_poly1305_init_64x64(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_init_64x64 called");
    return NULL;
}

void* chacha20_poly1305_init_96x32(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_init_96x32 called");
    return NULL;
}

void* chacha20_poly1305_verify(void)
{
    if (verbose) puts("STUB: chacha20_poly1305_verify called");
    return NULL;
}

void* chacha20_update(void)
{
    if (verbose) puts("STUB: chacha20_update called");
    return NULL;
}

void* copyFamilyDebugFlagsForInterface(void)
{
    if (verbose) puts("STUB: copyFamilyDebugFlagsForInterface called");
    return NULL;
}

void* copyVendorDebugFlagsForInterface(void)
{
    if (verbose) puts("STUB: copyVendorDebugFlagsForInterface called");
    return NULL;
}

void* curve25519_donna(void)
{
    if (verbose) puts("STUB: curve25519_donna called");
    return NULL;
}

void* fcopyline(void)
{
    if (verbose) puts("STUB: fcopyline called");
    return NULL;
}

void* iceil2(void)
{
    if (verbose) puts("STUB: iceil2 called");
    return NULL;
}

void* ifloor2_64(void)
{
    if (verbose) puts("STUB: ifloor2_64 called");
    return NULL;
}

void* ilog2_64(void)
{
    if (verbose) puts("STUB: ilog2_64 called");
    return NULL;
}

void* memrchr(void)
{
    if (verbose) puts("STUB: memrchr called");
    return NULL;
}

void* memrlen(void)
{
    if (verbose) puts("STUB: memrlen called");
    return NULL;
}

void* mkparent(void)
{
    if (verbose) puts("STUB: mkparent called");
    return NULL;
}

void* mkpath(void)
{
    if (verbose) puts("STUB: mkpath called");
    return NULL;
}

void* poly1305(void)
{
    if (verbose) puts("STUB: poly1305 called");
    return NULL;
}

void* poly1305_final(void)
{
    if (verbose) puts("STUB: poly1305_final called");
    return NULL;
}

void* poly1305_init(void)
{
    if (verbose) puts("STUB: poly1305_init called");
    return NULL;
}

void* poly1305_update(void)
{
    if (verbose) puts("STUB: poly1305_update called");
    return NULL;
}

void* rt_dispatch_async(void)
{
    if (verbose) puts("STUB: rt_dispatch_async called");
    return NULL;
}

void* rt_dispatch_async_f(void)
{
    if (verbose) puts("STUB: rt_dispatch_async_f called");
    return NULL;
}

void* rt_dispatch_get_context(void)
{
    if (verbose) puts("STUB: rt_dispatch_get_context called");
    return NULL;
}

void* rt_dispatch_queue_create(void)
{
    if (verbose) puts("STUB: rt_dispatch_queue_create called");
    return NULL;
}

void* rt_dispatch_release(void)
{
    if (verbose) puts("STUB: rt_dispatch_release called");
    return NULL;
}

void* rt_dispatch_resume(void)
{
    if (verbose) puts("STUB: rt_dispatch_resume called");
    return NULL;
}

void* rt_dispatch_retain(void)
{
    if (verbose) puts("STUB: rt_dispatch_retain called");
    return NULL;
}

void* rt_dispatch_set_context(void)
{
    if (verbose) puts("STUB: rt_dispatch_set_context called");
    return NULL;
}

void* rt_dispatch_set_finalizer_f(void)
{
    if (verbose) puts("STUB: rt_dispatch_set_finalizer_f called");
    return NULL;
}

void* rt_dispatch_source_cancel(void)
{
    if (verbose) puts("STUB: rt_dispatch_source_cancel called");
    return NULL;
}

void* rt_dispatch_source_create(void)
{
    if (verbose) puts("STUB: rt_dispatch_source_create called");
    return NULL;
}

void* rt_dispatch_source_set_event_handler(void)
{
    if (verbose) puts("STUB: rt_dispatch_source_set_event_handler called");
    return NULL;
}

void* rt_dispatch_source_set_event_handler_f(void)
{
    if (verbose) puts("STUB: rt_dispatch_source_set_event_handler_f called");
    return NULL;
}

void* rt_dispatch_source_set_timer(void)
{
    if (verbose) puts("STUB: rt_dispatch_source_set_timer called");
    return NULL;
}

void* rt_dispatch_suspend(void)
{
    if (verbose) puts("STUB: rt_dispatch_suspend called");
    return NULL;
}

void* rt_dispatch_sync(void)
{
    if (verbose) puts("STUB: rt_dispatch_sync called");
    return NULL;
}

void* rt_dispatch_time(void)
{
    if (verbose) puts("STUB: rt_dispatch_time called");
    return NULL;
}

void* se1P4X0Kh(void)
{
    if (verbose) puts("STUB: se1P4X0Kh called");
    return NULL;
}

void* setFamilyDebugFlagsForInterface(void)
{
    if (verbose) puts("STUB: setFamilyDebugFlagsForInterface called");
    return NULL;
}

void* setVendorDebugFlagsForInterface(void)
{
    if (verbose) puts("STUB: setVendorDebugFlagsForInterface called");
    return NULL;
}

void* snprintf_add(void)
{
    if (verbose) puts("STUB: snprintf_add called");
    return NULL;
}

void* strcmp_prefix(void)
{
    if (verbose) puts("STUB: strcmp_prefix called");
    return NULL;
}

void* stricmp_prefix(void)
{
    if (verbose) puts("STUB: stricmp_prefix called");
    return NULL;
}

void* stristr(void)
{
    if (verbose) puts("STUB: stristr called");
    return NULL;
}

void* strncasestr(void)
{
    if (verbose) puts("STUB: strncasestr called");
    return NULL;
}

void* strncmp_prefix(void)
{
    if (verbose) puts("STUB: strncmp_prefix called");
    return NULL;
}

void* strncmpx(void)
{
    if (verbose) puts("STUB: strncmpx called");
    return NULL;
}

void* strnicmp_prefix(void)
{
    if (verbose) puts("STUB: strnicmp_prefix called");
    return NULL;
}

void* strnicmp_suffix(void)
{
    if (verbose) puts("STUB: strnicmp_suffix called");
    return NULL;
}

void* strnicmpx(void)
{
    if (verbose) puts("STUB: strnicmpx called");
    return NULL;
}

void* systemf(void)
{
    if (verbose) puts("STUB: systemf called");
    return NULL;
}

void* unified_logger_deinit(void)
{
    if (verbose) puts("STUB: unified_logger_deinit called");
    return NULL;
}

void* unified_logger_init(void)
{
    if (verbose) puts("STUB: unified_logger_init called");
    return NULL;
}

void* unified_temp_logger_deinit(void)
{
    if (verbose) puts("STUB: unified_temp_logger_deinit called");
    return NULL;
}

void* unified_temp_logger_init(void)
{
    if (verbose) puts("STUB: unified_temp_logger_init called");
    return NULL;
}

void* updateDebugFlagPreferences(void)
{
    if (verbose) puts("STUB: updateDebugFlagPreferences called");
    return NULL;
}

void* wInTP56r94EFs9NAAi(void)
{
    if (verbose) puts("STUB: wInTP56r94EFs9NAAi called");
    return NULL;
}

void* wifi_prefs_deinit(void)
{
    if (verbose) puts("STUB: wifi_prefs_deinit called");
    return NULL;
}

void* wifi_prefs_init(void)
{
    if (verbose) puts("STUB: wifi_prefs_init called");
    return NULL;
}

void* xpc_connection_has_entitlement(void)
{
    if (verbose) puts("STUB: xpc_connection_has_entitlement called");
    return NULL;
}

void* xpc_connection_send_message_with_reply_f(void)
{
    if (verbose) puts("STUB: xpc_connection_send_message_with_reply_f called");
    return NULL;
}

void* xpc_connection_set_event_handler_f(void)
{
    if (verbose) puts("STUB: xpc_connection_set_event_handler_f called");
    return NULL;
}

void* xpc_dictionary_copy_cf_object(void)
{
    if (verbose) puts("STUB: xpc_dictionary_copy_cf_object called");
    return NULL;
}

void* xpc_dictionary_set_cf_object(void)
{
    if (verbose) puts("STUB: xpc_dictionary_set_cf_object called");
    return NULL;
}

void* xpc_send_message_sync(void)
{
    if (verbose) puts("STUB: xpc_send_message_sync called");
    return NULL;
}

#if 0
void* strnlen(void)
{
    if (verbose) puts("STUB: strnlen called");
    return NULL;
}

void* strndup(void)
{
    if (verbose) puts("STUB: strndup called");
    return NULL;
}
#endif
