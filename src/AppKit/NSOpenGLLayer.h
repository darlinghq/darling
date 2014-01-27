#import <AppKit/NSView.h>
#import <AppKit/NSOpenGL.h>
#import <QuartzCore/CAOpenGLLayer.h>



@interface NSOpenGLLayer : CAOpenGLLayer
{
@private
    NSOpenGLPixelFormat *_openGLPixelFormat;
    NSOpenGLContext *_openGLContext;
    void *_reserved[5];
}



@property(assign) NSView *view;



@property(retain) NSOpenGLPixelFormat *openGLPixelFormat;



@property(retain) NSOpenGLContext *openGLContext;



- (NSOpenGLPixelFormat *)openGLPixelFormatForDisplayMask:(uint32_t)mask;



- (NSOpenGLContext *)openGLContextForPixelFormat:(NSOpenGLPixelFormat *)pixelFormat;



- (BOOL)canDrawInOpenGLContext:(NSOpenGLContext *)context pixelFormat:(NSOpenGLPixelFormat *)pixelFormat forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts;



- (void)drawInOpenGLContext:(NSOpenGLContext *)context pixelFormat:(NSOpenGLPixelFormat *)pixelFormat forLayerTime:(CFTimeInterval)t displayTime:(const CVTimeStamp *)ts;

@end
