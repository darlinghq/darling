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


#ifndef _Network_H_
#define _Network_H_

#import <Foundation/Foundation.h>

#import <Network/NSURLSessionDelegate.h>
#import <Network/NWPrettyDescription.h>
#import <Network/NWTCPListenerDelegate.h>
#import <Network/NWAWDLibnetcoreMPTCPStatsReport.h>
#import <Network/NWSystemPathMonitor.h>
#import <Network/NWPBCloseConnection.h>
#import <Network/NWAWDLBEndpointsFetchReport.h>
#import <Network/NWTCPConnection.h>
#import <Network/NWAWDLibnetcoreTCPECNInterfaceStatsReport.h>
#import <Network/NWAWDNWAPIUsage.h>
#import <Network/NWAddressEndpoint.h>
#import <Network/NWPBEndpoint.h>
#import <Network/NWAWDLibnetcoreTCPKernelStats.h>
#import <Network/NWAWDNWConnectionReport.h>
#import <Network/NWHostEndpoint.h>
#import <Network/NWMonitor.h>
#import <Network/NWAWDMPTCPConnectionReport.h>
#import <Network/NWAWDLibnetcoreConnectionStatisticsReport.h>
#import <Network/NWAWDLibnetcoreStatsReport.h>
#import <Network/NWConnection.h>
#import <Network/NWAWDNWActivity.h>
#import <Network/NWAWDLibnetcoreNetworkdStatsReport.h>
#import <Network/NWPBBrowseDescriptor.h>
#import <Network/NWRemotePacketProxy.h>
#import <Network/NWPBServiceBrowse.h>
#import <Network/NWPBAgent.h>
#import <Network/NWPBAgentClass.h>
#import <Network/ManagedNetworkSettings.h>
#import <Network/NWAWDLibnetcoreTCPStatsReport.h>
#import <Network/NWAWDLibnetcoreCellularFallbackReport.h>
#import <Network/NWAWDNWDeviceReport.h>
#import <Network/NWAWDLBConnectionReport.h>
#import <Network/NWAdvertiseDescriptor.h>
#import <Network/NWEndpoint.h>
#import <Network/NWAWDLibnetcoreTCPTFOStatsReport.h>
#import <Network/NWPBParameters.h>
#import <Network/NWAWDNWActivityEpilogue.h>
#import <Network/NWPBUpdateBrowse.h>
#import <Network/NWNetworkAgentRegistration.h>
#import <Network/NWPHContext.h>
#import <Network/NWPHHandler.h>
#import <Network/NWPrivilegedHelper.h>
#import <Network/NWUDPSession.h>
#import <Network/NWRemoteConnectionWriteRequest.h>
#import <Network/NWRemoteConnectionDirector.h>
#import <Network/NWNetworkDescription.h>
#import <Network/NWUDPListener.h>
#import <Network/NWParameters.h>
#import <Network/NWGenericNetworkAgent.h>
#import <Network/NWPBAddressEndpoint.h>
#import <Network/NWBonjourServiceEndpoint.h>
#import <Network/NWPath.h>
#import <Network/NWPathFlow.h>
#import <Network/NWRemoteConnectionWrapper.h>
#import <Network/NWRemoteBrowserWrapper.h>
#import <Network/NWRemoteConnectionActor.h>
#import <Network/NWAWDLibnetcoreMbufStatsReport.h>
#import <Network/NWPBHostEndpoint.h>
#import <Network/NWAWDMPTCPSubflowSwitchingReport.h>
#import <Network/NWAWDNWL2Report.h>
#import <Network/NWPBCommandMessage.h>
#import <Network/NWStreamConnection.h>
#import <Network/NWPBOpenConnection.h>
#import <Network/NWProtocolTransform.h>
#import <Network/NWInterface.h>
#import <Network/NWPBStartBrowse.h>
#import <Network/NWAWDLibnetcoreConnectionDataUsageSnapshot.h>
#import <Network/NWBrowser.h>
#import <Network/NWAWDMPTCPConnectionInterfaceReport.h>
#import <Network/NWBonjourBrowseDescriptor.h>
#import <Network/NWPBPath.h>
#import <Network/NWAWDLibnetcoreRNFActivityNotification.h>
#import <Network/NWMessage.h>
#import <Network/NWOutboundMessage.h>
#import <Network/NWInboundMessage.h>
#import <Network/NWMessageConnection.h>
#import <Network/NWL2Report.h>
#import <Network/NWDeviceReport.h>
#import <Network/NWActivityStatistics.h>
#import <Network/NWActivityEpilogueStatistics.h>
#import <Network/NWActivityEmptyTrigger.h>
#import <Network/NWPBStopBrowse.h>
#import <Network/NWConnectionStatistics.h>
#import <Network/NWPBInterface.h>
#import <Network/NWPBSendData.h>
#import <Network/NWAWDLibnetcoreTCPConnectionReport.h>
#import <Network/NWAWDNWActivityEmptyTrigger.h>
#import <Network/NWPBServiceEndpoint.h>
#import <Network/NWDatagramConnection.h>
#import <Network/NWBrowseDescriptor.h>
#import <Network/NWAWDLBClientConnectionReport.h>
#import <Network/NWPBUpdatePath.h>
#import <Network/NWResolver.h>
#import <Network/NWTCPListener.h>
#import <Network/NWPathEvaluator.h>
#import <Network/NWAWDLibnetcoreTCPECNStatsReport.h>

#import <nw/nw.h>

void* NWCreateDispatchDataFromNSData(void);
void* NWCreateNSDataFromDispatchData(void);
void* NWMonitorStatusToString(void);
void* NWPBAddressEndpointReadFrom(void);
void* NWPBAgentClassReadFrom(void);
void* NWPBAgentReadFrom(void);
void* NWPBBrowseDescriptorReadFrom(void);
void* NWPBCloseConnectionReadFrom(void);
void* NWPBCommandMessageReadFrom(void);
void* NWPBEndpointReadFrom(void);
void* NWPBHostEndpointReadFrom(void);
void* NWPBInterfaceReadFrom(void);
void* NWPBOpenConnectionReadFrom(void);
void* NWPBParametersReadFrom(void);
void* NWPBPathReadFrom(void);
void* NWPBSendDataReadFrom(void);
void* NWPBServiceBrowseReadFrom(void);
void* NWPBServiceEndpointReadFrom(void);
void* NWPBStartBrowseReadFrom(void);
void* NWPBStopBrowseReadFrom(void);
void* NWPBUpdateBrowseReadFrom(void);
void* NWPBUpdatePathReadFrom(void);
void* NWPrivilegedHelperHandleEvent(void);
void* NWPrivilegedHelperInitialize(void);
void* NWPrivilegedHelperStart(void);
void* NWUtilsCreateNSDictionaryFromXPCDictionary(void);
void* NWUtilsCreateNSStringFromSockaddr(void);
void* NWUtilsCreateXPCDictionaryFromNSDictionary(void);
void* NWUtilsStringMatchesPattern(void);
void* updateOSLogPlistFromLevel(void);

#endif
