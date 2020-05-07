#ifndef CVDISPLAYLINK_H
#define CVDISPLAYLINK_H

#include <ApplicationServices/ApplicationServices.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVBase.h>
#include <OpenGL/OpenGL.h>

typedef struct CV_BRIDGED_TYPE(id) __CVDisplayLink *CVDisplayLinkRef;

#if defined(__cplusplus)
extern "C" {
#endif

typedef CVReturn (*CVDisplayLinkOutputCallback)(
		CVDisplayLinkRef CV_NONNULL displayLink,
		const CVTimeStamp * CV_NONNULL inNow,
		const CVTimeStamp * CV_NONNULL inOutputTime,
		CVOptionFlags flagsIn, 
		CVOptionFlags * CV_NONNULL flagsOut,
		void * CV_NULLABLE displayLinkContext );

#if __BLOCKS__
typedef CVReturn (^CVDisplayLinkOutputHandler)(
		CVDisplayLinkRef CV_NONNULL displayLink,
		const CVTimeStamp * CV_NONNULL inNow,
		const CVTimeStamp * CV_NONNULL inOutputTime,
		CVOptionFlags flagsIn,
		CVOptionFlags * CV_NONNULL flagsOut );
#endif // __BLOCKS__

CV_EXPORT CFTypeID CVDisplayLinkGetTypeID(void) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkCreateWithCGDisplays(
    CGDirectDisplayID * CV_NONNULL displayArray,
    CFIndex count,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkCreateWithOpenGLDisplayMask(
    CGOpenGLDisplayMask mask,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkCreateWithCGDisplay(
    CGDirectDisplayID displayID,
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkCreateWithActiveCGDisplays(
    CV_RETURNS_RETAINED_PARAMETER CVDisplayLinkRef CV_NULLABLE * CV_NONNULL displayLinkOut ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkSetCurrentCGDisplay( CVDisplayLinkRef CV_NONNULL displayLink, CGDirectDisplayID displayID ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(
																  CVDisplayLinkRef CV_NONNULL displayLink,
																  CGLContextObj CV_NONNULL cglContext,
																  CGLPixelFormatObj CV_NONNULL cglPixelFormat) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;


CV_EXPORT CGDirectDisplayID CVDisplayLinkGetCurrentCGDisplay( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkSetOutputCallback( CVDisplayLinkRef CV_NONNULL displayLink, CVDisplayLinkOutputCallback CV_NULLABLE callback, void * CV_NULLABLE userInfo ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkSetOutputHandler( CVDisplayLinkRef CV_NONNULL displayLink, CVDisplayLinkOutputHandler CV_NONNULL handler );

CV_EXPORT CVReturn CVDisplayLinkStart( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkStop( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVTime CVDisplayLinkGetNominalOutputVideoRefreshPeriod( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVTime CVDisplayLinkGetOutputVideoLatency( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT double CVDisplayLinkGetActualOutputVideoRefreshPeriod( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT Boolean CVDisplayLinkIsRunning( CVDisplayLinkRef CV_NONNULL displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkGetCurrentTime( CVDisplayLinkRef CV_NONNULL displayLink, CVTimeStamp * CV_NONNULL outTime ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVReturn CVDisplayLinkTranslateTime( CVDisplayLinkRef CV_NONNULL displayLink, const CVTimeStamp * CV_NONNULL inTime, CVTimeStamp * CV_NONNULL outTime ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT CVDisplayLinkRef CV_NULLABLE CVDisplayLinkRetain( CVDisplayLinkRef CV_NULLABLE displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

CV_EXPORT void CVDisplayLinkRelease( CV_RELEASES_ARGUMENT CVDisplayLinkRef CV_NULLABLE displayLink ) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

#if defined(__cplusplus)
}

#endif

#endif

