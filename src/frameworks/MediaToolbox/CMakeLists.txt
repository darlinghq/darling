project(MediaToolbox)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(MediaToolbox
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/MediaToolbox.m
        src/FigCaptionLayerPrivate.m
        src/FigCaptionLayer.m
        src/FigCaptionRowLayer.m
        src/FigNeroLayer.m
        src/FigNSURLSession.m
        src/FigNSURLSessionRegistry.m
        src/FigHTTPRequestSessionDataDelegate.m
        src/FPSupport_PowerStateSingleton.m
        src/FPSupport_VideoRangeSingleton.m
        src/FigFCRCALayerOutputNodeContentLayer.m
        src/FigFCRCALayerOutputNodeLayer.m
        src/FigCaptionBackdropLayer.m
        src/FigFCRCALayer.m
        src/FigSubtitleWebVTTCueCALayer.m
        src/FigSubtitleBackdropCALayerContentLayer.m
        src/FigSubtitleBackdropCALayer.m
        src/FigCDSCALayer.m
        src/FigBaseCALayer.m
        src/FigBaseCALayerHost.m
        src/FigDisplaySleepAssertion.m
        src/FigSubtitleCALayer.m
        src/FigVideoContainerLayer.m
        src/FigVideoLayerInternal.m
        src/FigVideoLayer.m
        src/FigCDSCALayerOutputNodeContentLayer.m
        src/FigCDSCALayerOutputNodeLayer.m
        src/FigCPEFPAirPlaySession.m
        src/FigSubtitleWebVTTRegionCALayer.m

    DEPENDENCIES
        system
        objc
        Foundation
)
