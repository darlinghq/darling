project(MultipeerConnectivity)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "155.2.0")

add_framework(MultipeerConnectivity
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/MultipeerConnectivity.m
        src/MCBrowserViewController.m
        src/MCAdvertiserAssistant.m
        src/MCSessionPeerState.m
        src/MCSessionStream.m
        src/MCResourceDownloader.m
        src/MCSessionPeerConnectionData.m
        src/MCResourceProgressObserver.m
        src/MCSession.m
        src/MCNearbyServiceUtils.m
        src/MCNearbyDiscoveryPeerConnection.m
        src/AWDMultipeerSessionMaxPeersInfo.m
        src/MCNearbyServiceAdvertiser.m
        src/MCNearbyServiceBrowser.m
        src/MCNearbyDiscoveryPeer.m
        src/AWDMultipeerSessionPeerConnectivityInfo.m
        src/MCPeerIDInternal.m
        src/MCPeerID.m

    DEPENDENCIES
        system
        objc
        Foundation
)
