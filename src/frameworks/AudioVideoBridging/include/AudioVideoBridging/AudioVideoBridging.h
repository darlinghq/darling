/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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


#ifndef _AudioVideoBridging_H_
#define _AudioVideoBridging_H_

#import <Foundation/Foundation.h>

#import <AudioVideoBridging/ADDDeviceService.h>
#import <AudioVideoBridging/ALServiceProtocol.h>
#import <AudioVideoBridging/ASACoreAudioClient.h>
#import <AudioVideoBridging/AVB17221ACMPClient.h>
#import <AudioVideoBridging/AVB17221AECPClient.h>
#import <AudioVideoBridging/AVB17221EntityDiscoveryDelegate.h>
#import <AudioVideoBridging/AVB17221EntityPublisher.h>
#import <AudioVideoBridging/AVBInterfaceDelegate.h>
#import <AudioVideoBridging/AVBMSRPListenerClient.h>
#import <AudioVideoBridging/AVBMSRPTalkerClient.h>
#import <AudioVideoBridging/TSClockClient.h>
#import <AudioVideoBridging/AVB1722MAAP.h>
#import <AudioVideoBridging/AVB1722ControlInterface.h>
#import <AudioVideoBridging/AVB17221ACMPInterface.h>
#import <AudioVideoBridging/AVB17221ACMPPendingResponse.h>
#import <AudioVideoBridging/AVB17221AECPInterface.h>
#import <AudioVideoBridging/AVB17221AECPPendingResponse.h>
#import <AudioVideoBridging/AVB17221AEMAudioCluster.h>
#import <AudioVideoBridging/AVB17221AEMAudioMap.h>
#import <AudioVideoBridging/AVB17221AEMAudioMapping.h>
#import <AudioVideoBridging/AVB17221AEMStreamPort.h>
#import <AudioVideoBridging/AVBWiFiInterface.h>
#import <AudioVideoBridging/AVB17221AEMAudioUnit.h>
#import <AudioVideoBridging/AVB17221AEMAVBInterface.h>
#import <AudioVideoBridging/AVB17221AEMLMMMapping.h>
#import <AudioVideoBridging/AVB17221AEMLocalModelMaker.h>
#import <AudioVideoBridging/AVB17221AEMLocalRemoteMapping.h>
#import <AudioVideoBridging/AVB17221AEMClockSource.h>
#import <AudioVideoBridging/AVB17221AEMConfiguration.h>
#import <AudioVideoBridging/AVBIIDCStreamConfiguration.h>
#import <AudioVideoBridging/AVB17221AEMControl.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorArrayValue.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorBodePlotValue.h>
#import <AudioVideoBridging/AVB17221AEMBodePlotEntry.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorLinearValue.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorSelectorValue.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorUTF8Value.h>
#import <AudioVideoBridging/AVBInterfaceStreamingManager.h>
#import <AudioVideoBridging/AVB17221AEMAVCaptureDeviceModelMaker.h>
#import <AudioVideoBridging/AVBAVDECCController.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorValue.h>
#import <AudioVideoBridging/AVB17221AEMDescriptorCount.h>
#import <AudioVideoBridging/AVB17221AEMEntity.h>
#import <AudioVideoBridging/AVB17221AEMJack.h>
#import <AudioVideoBridging/AVB17221AEMExternalPort.h>
#import <AudioVideoBridging/AVB17221AEMInternalPort.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorLinearSet.h>
#import <AudioVideoBridging/AVB17221AEMLocale.h>
#import <AudioVideoBridging/AVB17221AEMLocalizedStringReference.h>
#import <AudioVideoBridging/AVB17221AEMMatrix.h>
#import <AudioVideoBridging/AVB17221AEMMixer.h>
#import <AudioVideoBridging/AVB17221AEMModelObject.h>
#import <AudioVideoBridging/AVBAVDECCEntity.h>
#import <AudioVideoBridging/AVBAVDECCEntityInterface.h>
#import <AudioVideoBridging/AVB17221AEMMSRPMapping.h>
#import <AudioVideoBridging/AVB17221AEMObject.h>
#import <AudioVideoBridging/AVB17221AEMSamplingRate.h>
#import <AudioVideoBridging/AVB17221AEMSamplingRateRange.h>
#import <AudioVideoBridging/AVBIIDCUserOutputStream.h>
#import <AudioVideoBridging/AVBEUI64Transformer.h>
#import <AudioVideoBridging/AVBConnectedTransformer.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorVendorValue.h>
#import <AudioVideoBridging/AVB17221AEMSensorUnit.h>
#import <AudioVideoBridging/AVB17221AEMSignalSelector.h>
#import <AudioVideoBridging/AVB17221AEMSignal.h>
#import <AudioVideoBridging/AVB17221AEMStream.h>
#import <AudioVideoBridging/AVB17221AEMStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMStandardStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMIIDCStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM61883StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM618834StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM618836StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM61883632BitStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM618836FloatStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM618836AM824StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM618837StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEM618838StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMMMAStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMAAFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMAAFPCMStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMAAFAES3StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMCVFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMCVFMJPEGStreamForamt.h>
#import <AudioVideoBridging/AVB17221AEMCVFH264StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMCVFJPEG2000StreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMCRFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMTSCFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMSVFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMRVFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMVSFStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMVendorStreamFormat.h>
#import <AudioVideoBridging/AVB17221AEMStrings.h>
#import <AudioVideoBridging/AVB17221AEMVideoUnit.h>
#import <AudioVideoBridging/AVBEthernetInterface.h>
#import <AudioVideoBridging/AVBInterface.h>
#import <AudioVideoBridging/AVBIIDCUserInputStream.h>
#import <AudioVideoBridging/AVB17221Entity.h>
#import <AudioVideoBridging/AVB17221EntityDiscovery.h>
#import <AudioVideoBridging/AVBIIDCIOSurfaceStream.h>
#import <AudioVideoBridging/AVBMRP.h>
#import <AudioVideoBridging/AVBMSRPDomain.h>
#import <AudioVideoBridging/AVBMSRPDomainInfo.h>
#import <AudioVideoBridging/AVBSimpleVirtualAudio.h>
#import <AudioVideoBridging/AVBMACAddressFormatter.h>
#import <AudioVideoBridging/AVBVirtualStream.h>
#import <AudioVideoBridging/AVBVirtualStreamConnection.h>
#import <AudioVideoBridging/AVBMSRPListener.h>
#import <AudioVideoBridging/AVB17221AEMModelMaker.h>
#import <AudioVideoBridging/AVBMSRPTalker.h>
#import <AudioVideoBridging/AVBMSRPTalkerAttribute.h>
#import <AudioVideoBridging/AVBMVRP.h>
#import <AudioVideoBridging/AVB17221AEMCoreAudioDeviceModelMaker.h>
#import <AudioVideoBridging/AVBBuiltInAVDECCEntity.h>
#import <AudioVideoBridging/AVBNub.h>
#import <AudioVideoBridging/AVB17221AEMMemoryObjectData.h>
#import <AudioVideoBridging/AVB17221AEMEntityEnumerator.h>
#import <AudioVideoBridging/AVBMACAddress.h>
#import <AudioVideoBridging/AVB17221ACMPMessage.h>
#import <AudioVideoBridging/AVB17221AECPAEMMessage.h>
#import <AudioVideoBridging/AVB17221AECPAddressAccessMessage.h>
#import <AudioVideoBridging/AVBAudioDeviceSharing.h>
#import <AudioVideoBridging/AVBMutableBool.h>
#import <AudioVideoBridging/AVBVirtualEntity.h>
#import <AudioVideoBridging/AVB17221AECPAVCMessage.h>
#import <AudioVideoBridging/AVB17221AECPVendorMessage.h>
#import <AudioVideoBridging/AVB17221AECPAddressAccessTLV.h>
#import <AudioVideoBridging/AVB17221AEMStreamFormatTransformer.h>
#import <AudioVideoBridging/AVB17221AEMVideoColorSpaceTransformer.h>
#import <AudioVideoBridging/AVB17221AECPMessage.h>
#import <AudioVideoBridging/AVB17221AEMMatrixSignal.h>
#import <AudioVideoBridging/AVB17221AEMMemoryObject.h>
#import <AudioVideoBridging/AVB17221AEMNamedModelObject.h>
#import <AudioVideoBridging/AVB17221AEMClockDomainedModelObject.h>
#import <AudioVideoBridging/AVB17221AEMNamedClockDomainedModelObject.h>
#import <AudioVideoBridging/AVB17221AEMUnit.h>
#import <AudioVideoBridging/AVB17221AEMCluster.h>
#import <AudioVideoBridging/AVBMAAPMACAddress.h>
#import <AudioVideoBridging/AVB17221AEMPort.h>
#import <AudioVideoBridging/AVBCentralManager.h>
#import <AudioVideoBridging/AVB17221AEMSignalPort.h>
#import <AudioVideoBridging/AVB17221AEMSampleRateTransformer.h>
#import <AudioVideoBridging/AVB17221AEMVideoMap.h>
#import <AudioVideoBridging/AVBIIDCIOSurfaceOutputStream.h>
#import <AudioVideoBridging/AVB17221AEMSensorMap.h>
#import <AudioVideoBridging/AVB17221AEMVideoMapping.h>
#import <AudioVideoBridging/AVB17221AEMSensorMapping.h>
#import <AudioVideoBridging/AVB17221AEMSensorCluster.h>
#import <AudioVideoBridging/AVB17221AEMBaseControl.h>
#import <AudioVideoBridging/AVBCustomVirtualAudioEntityAEMValidator.h>
#import <AudioVideoBridging/AVB17221AEMSignalTranscoder.h>
#import <AudioVideoBridging/AVB17221AEMSignalSplitter.h>
#import <AudioVideoBridging/AVB17221AEMSignalSplitterMapping.h>
#import <AudioVideoBridging/AVB17221AEMSignalDemultiplexerMapping.h>
#import <AudioVideoBridging/AVBBuiltInAVDECCController.h>
#import <AudioVideoBridging/AVB17221AEMSignalDemultiplexer.h>
#import <AudioVideoBridging/AVB17221AEMSignalCombinerMapping.h>
#import <AudioVideoBridging/AVB17221AEMSignalMultiplexerMapping.h>
#import <AudioVideoBridging/AVB17221AEMSignalCombiner.h>
#import <AudioVideoBridging/AVB17221AEMSignalMultiplexer.h>
#import <AudioVideoBridging/AVB17221AEMVideoCluster.h>
#import <AudioVideoBridging/AVB17221AEMVideoFormatSpecific.h>
#import <AudioVideoBridging/AVB17221AEMVideoAspectRatio.h>
#import <AudioVideoBridging/AVB17221AEMVideoSize.h>
#import <AudioVideoBridging/AVBgPTPEthernetPort.h>
#import <AudioVideoBridging/AVB17221AEMClockDomain.h>
#import <AudioVideoBridging/AVB17221AEMSensorFormat.h>
#import <AudioVideoBridging/AVB17221AEMControlBlock.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorSelectorStringValue.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorSMPTETimeValue.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorGPTPValue.h>
#import <AudioVideoBridging/AVB17221AEMControlDescriptorSamplingRateValue.h>
#import <AudioVideoBridging/AVBAudioDriverManager.h>
#import <AudioVideoBridging/AVBInputStream.h>
#import <AudioVideoBridging/AVBStream.h>
#import <AudioVideoBridging/AVBOutputStream.h>

void* AVB17221AEMVideoColorSpaceBitsPerPixel(void);
void* AVB17221AEMVideoColorSpaceFromCVPixelFormatType(void);
void* AVB17221AEMVideoColorSpaceFromIIDCVideoFormat(void);
void* AVB17221AEMVideoColorSpaceIsSupportedByCVPixelFormatType(void);
void* AVB17221AEMVideoColorSpaceSupportsCVPixelFormatType(void);
void* AVB17221AEMVideoColorSpaceToCVPixelFormatType(void);
void* _Z11AAA_add_128yy(void);
void* _Z13AAA_uint64mulyy(void);
void* _ZNK8AAA_U128dvERKS_(void);
void* _ZNK8AAA_U128mlERKS_(void);
void* _ZmlRK8AAA_U128y(void);
void* _ZmlyRK8AAA_U128(void);

#endif
