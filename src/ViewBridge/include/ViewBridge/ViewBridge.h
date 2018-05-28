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


#ifndef _ViewBridge_H_
#define _ViewBridge_H_

#import <Foundation/Foundation.h>

#import <ViewBridge/AuxiliaryCallsClient.h>
#import <ViewBridge/AuxiliaryCallsService.h>
#import <ViewBridge/ClientCallsAuxiliary.h>
#import <ViewBridge/HasAuxiliaryConnection.h>
#import <ViewBridge/HostCallsAuxiliary.h>
#import <ViewBridge/HostOrServiceCallsAuxiliary.h>
#import <ViewBridge/NSRemoteViewDelegate.h>
#import <ViewBridge/NSRemoteViewMarshal.h>
#import <ViewBridge/NSRendezvousWindowDelegate.h>
#import <ViewBridge/NSServiceViewControllerActionResponder.h>
#import <ViewBridge/NSTouchBarDelegate.h>
#import <ViewBridge/NSTouchBarProvider.h>
#import <ViewBridge/NSVBDeallocOnAppKitThread.h>
#import <ViewBridge/NSVBRedactedProtocol.h>
#import <ViewBridge/NSVBXPCConnectionClient.h>
#import <ViewBridge/NSViewBridgeKVOBuddy.h>
#import <ViewBridge/NSViewServiceMarshal.h>
#import <ViewBridge/NSWindowDelegate.h>
#import <ViewBridge/NSXPCConnectionDelegate.h>
#import <ViewBridge/NSXPCListenerDelegate.h>
#import <ViewBridge/NSXPCSharedListenerDelegate.h>
#import <ViewBridge/NSXPCSharedListenerManager.h>
#import <ViewBridge/PKModularService.h>
#import <ViewBridge/ServiceCallsAuxiliary.h>
#import <ViewBridge/TouchBarsAndItems.h>
#import <ViewBridge/_NSAlertSensitiveView.h>
#import <ViewBridge/_NSTouchBarItemLayoutWrapper.h>
#import <ViewBridge/_UIViewServiceImplicitAnimationDecodingProxy_EncodingProxyInterface.h>
#import <ViewBridge/_UIViewServiceProxy_ReplyAwaiting.h>
#import <ViewBridge/_UIViewServiceProxy_ReplyControl.h>
#import <ViewBridge/_UIViewServiceUIBehaviorInterface.h>
#import <ViewBridge/_UIViewService_FencingControlInterface.h>
#import <ViewBridge/NSVBTestedFault.h>
#import <ViewBridge/NSVBNamedFault.h>
#import <ViewBridge/NSViewBridgeObjectBase.h>
#import <ViewBridge/NSViewBridgeObject.h>
#import <ViewBridge/NSViewBridge.h>
#import <ViewBridge/NSServiceViewControllerWindow.h>
#import <ViewBridge/NSServiceViewControllerAuxiliary.h>
#import <ViewBridge/NSServiceViewController.h>
#import <ViewBridge/NSViewService.h>
#import <ViewBridge/NSViewServiceListenerDelegate.h>
#import <ViewBridge/NSViewServiceApplication.h>
#import <ViewBridge/NSViewService_PKSubsystem.h>
#import <ViewBridge/NSViewServiceBridge.h>
#import <ViewBridge/NSViewServiceMarshal.h>
#import <ViewBridge/NSViewServiceAccessoryView.h>
#import <ViewBridge/NSVB_QueueingProxy.h>
#import <ViewBridge/NSVB_ViewServiceXPCMachSendRight.h>
#import <ViewBridge/NSVB_ProxyObject.h>
#import <ViewBridge/NSVB_ViewServiceReplyControlProxy.h>
#import <ViewBridge/NSVB_ViewServiceReplyControlTrampoline.h>
#import <ViewBridge/NSVB_ViewServiceReplyAwaitingTrampoline.h>
#import <ViewBridge/NSVB_ViewServiceFencingControlProxy.h>
#import <ViewBridge/NSVB_ViewServiceFencingController.h>
#import <ViewBridge/NSVB_ViewServiceBehaviorProxy.h>
#import <ViewBridge/NSVB_Window.h>
#import <ViewBridge/NSXPCSharedListenerManager.h>
#import <ViewBridge/NSXPCSharedListenerManagerDelegate.h>
#import <ViewBridge/NSObtainingEndpointState.h>
#import <ViewBridge/NSXPCSharedListener.h>
#import <ViewBridge/NSDeferredSheet.h>
#import <ViewBridge/NSVBAccessoryWindow.h>
#import <ViewBridge/NSVBAccessoryWindowBackstopResponder.h>
#import <ViewBridge/NSFakeServiceResponder.h>
#import <ViewBridge/NSViewRemoteBridge.h>
#import <ViewBridge/NSServiceViewControllerUnifyingProxy.h>
#import <ViewBridge/NSSelfDestructingRemoteViewDelegate.h>
#import <ViewBridge/NSRendezvousChildWindowDelegate.h>
#import <ViewBridge/NSRendezvousPopoverDelegate.h>
#import <ViewBridge/NSRemoteViewSemaphore.h>
#import <ViewBridge/CALayerHostForRemoteView.h>
#import <ViewBridge/NSRemoteView.h>
#import <ViewBridge/NSChildWindowQueueElement.h>
#import <ViewBridge/NSRemoteViewMarshal.h>
#import <ViewBridge/NSLayerCentricRemoteView.h>
#import <ViewBridge/NSVB_ViewAnimationAttributes.h>
#import <ViewBridge/NSVB_View.h>
#import <ViewBridge/NSVB_ViewServiceImplicitAnimationEncodingProxy.h>
#import <ViewBridge/NSVB_ViewServiceImplicitAnimationDecodingProxy.h>
#import <ViewBridge/NSCFRunLoopSemaphore.h>
#import <ViewBridge/NSVBClientCallsAuxiliaryConnection.h>
#import <ViewBridge/HostAndService.h>
#import <ViewBridge/HostOrService.h>
#import <ViewBridge/ViewHost.h>
#import <ViewBridge/ViewService.h>
#import <ViewBridge/CalledByClient.h>
#import <ViewBridge/ListenerDelegate.h>
#import <ViewBridge/HostListenerDelegate.h>
#import <ViewBridge/ServiceListenerDelegate.h>
#import <ViewBridge/ControlListenerDelegate.h>
#import <ViewBridge/NSRemoteViewControllerAuxiliary.h>
#import <ViewBridge/NSRemoteViewController.h>
#import <ViewBridge/NSPanelForServiceViewControllerForTouchBarItem.h>
#import <ViewBridge/NSServiceViewControllerForTouchBarItem.h>
#import <ViewBridge/NSSpaceTouchBarItemForRemoteView.h>
#import <ViewBridge/NSCustomTouchBarItemForRemoteView.h>
#import <ViewBridge/NSGroupTouchBarItemForRemoteView.h>
#import <ViewBridge/NSRemoteViewControllerForTouchBarItem.h>
#import <ViewBridge/NSTouchBarItemSurprise.h>
#import <ViewBridge/NSTouchBarItemOverlayForRemoteView.h>
#import <ViewBridge/NSTouchBarForRemoteView.h>
#import <ViewBridge/NSVBObjectDeallocationCanary.h>
#import <ViewBridge/NSEventQueue.h>
#import <ViewBridge/NSCFRunLoopObserver.h>
#import <ViewBridge/ViewBridgeUtilities.h>
#import <ViewBridge/NSRendezvousWindowController.h>
#import <ViewBridge/NSRendezvousAppModalSessionController.h>
#import <ViewBridge/NSRendezvousSheetController.h>
#import <ViewBridge/NSRendezvousChildWindowController.h>
#import <ViewBridge/NSRendezvousChildPanelController.h>
#import <ViewBridge/NSRendezvousPopoverController.h>
#import <ViewBridge/NSVB_TargetedProxy.h>

void* NSViewServiceApplicationMain(void);
void* ViewBridgeAuxiliaryMain(void);

#endif
