project(ViewBridge)

remove_sdk_framework(ViewBridge
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ViewBridge
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ViewBridge"
	PRIVATE
)

add_framework(ViewBridge
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ViewBridge.m
        src/NSVBTestedFault.m
        src/NSVBNamedFault.m
        src/NSViewBridgeObjectBase.m
        src/NSViewBridgeObject.m
        src/NSViewBridge.m
        src/NSServiceViewControllerWindow.m
        src/NSServiceViewControllerAuxiliary.m
        src/NSServiceViewController.m
        src/NSViewService.m
        src/NSViewServiceListenerDelegate.m
        src/NSViewServiceApplication.m
        src/NSViewService_PKSubsystem.m
        src/NSViewServiceBridge.m
        src/NSViewServiceMarshal.m
        src/NSViewServiceAccessoryView.m
        src/NSVB_QueueingProxy.m
        src/NSVB_ViewServiceXPCMachSendRight.m
        src/NSVB_ProxyObject.m
        src/NSVB_ViewServiceReplyControlProxy.m
        src/NSVB_ViewServiceReplyControlTrampoline.m
        src/NSVB_ViewServiceReplyAwaitingTrampoline.m
        src/NSVB_ViewServiceFencingControlProxy.m
        src/NSVB_ViewServiceFencingController.m
        src/NSVB_ViewServiceBehaviorProxy.m
        src/NSVB_Window.m
        src/NSXPCSharedListenerManager.m
        src/NSXPCSharedListenerManagerDelegate.m
        src/NSObtainingEndpointState.m
        src/NSXPCSharedListener.m
        src/NSDeferredSheet.m
        src/NSVBAccessoryWindow.m
        src/NSVBAccessoryWindowBackstopResponder.m
        src/NSFakeServiceResponder.m
        src/NSViewRemoteBridge.m
        src/NSServiceViewControllerUnifyingProxy.m
        src/NSSelfDestructingRemoteViewDelegate.m
        src/NSRendezvousChildWindowDelegate.m
        src/NSRendezvousPopoverDelegate.m
        src/NSRemoteViewSemaphore.m
        src/CALayerHostForRemoteView.m
        src/NSRemoteView.m
        src/NSChildWindowQueueElement.m
        src/NSRemoteViewMarshal.m
        src/NSLayerCentricRemoteView.m
        src/NSVB_ViewAnimationAttributes.m
        src/NSVB_View.m
        src/NSVB_ViewServiceImplicitAnimationEncodingProxy.m
        src/NSVB_ViewServiceImplicitAnimationDecodingProxy.m
        src/NSCFRunLoopSemaphore.m
        src/NSVBClientCallsAuxiliaryConnection.m
        src/HostAndService.m
        src/HostOrService.m
        src/ViewHost.m
        src/ViewService.m
        src/CalledByClient.m
        src/ListenerDelegate.m
        src/HostListenerDelegate.m
        src/ServiceListenerDelegate.m
        src/ControlListenerDelegate.m
        src/NSRemoteViewControllerAuxiliary.m
        src/NSRemoteViewController.m
        src/NSPanelForServiceViewControllerForTouchBarItem.m
        src/NSServiceViewControllerForTouchBarItem.m
        src/NSSpaceTouchBarItemForRemoteView.m
        src/NSCustomTouchBarItemForRemoteView.m
        src/NSGroupTouchBarItemForRemoteView.m
        src/NSRemoteViewControllerForTouchBarItem.m
        src/NSTouchBarItemSurprise.m
        src/NSTouchBarItemOverlayForRemoteView.m
        src/NSTouchBarForRemoteView.m
        src/NSVBObjectDeallocationCanary.m
        src/NSEventQueue.m
        src/NSCFRunLoopObserver.m
        src/ViewBridgeUtilities.m
        src/NSRendezvousWindowController.m
        src/NSRendezvousAppModalSessionController.m
        src/NSRendezvousSheetController.m
        src/NSRendezvousChildWindowController.m
        src/NSRendezvousChildPanelController.m
        src/NSRendezvousPopoverController.m
        src/NSVB_TargetedProxy.m

    DEPENDENCIES
        system
        objc
        Foundation
)
