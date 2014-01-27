#import <Foundation/NSObject.h>
#import <OpenGL/gltypes.h>

@class NSData, NSView, NSScreen;




#define NSOPENGL_CURRENT_VERSION 1


typedef enum {
 NSOpenGLGOFormatCacheSize = 501,
 NSOpenGLGOClearFormatCache = 502,
 NSOpenGLGORetainRenderers = 503,
 NSOpenGLGOUseBuildCache = 506,


 NSOpenGLGOResetLibrary = 504
} NSOpenGLGlobalOption;





extern void NSOpenGLSetOption(NSOpenGLGlobalOption pname, GLint param);
extern void NSOpenGLGetOption(NSOpenGLGlobalOption pname, GLint *param);




extern void NSOpenGLGetVersion(GLint *major, GLint *minor);
enum {
 NSOpenGLPFAAllRenderers = 1,
 NSOpenGLPFATripleBuffer = 3,
 NSOpenGLPFADoubleBuffer = 5,
 NSOpenGLPFAStereo = 6,
 NSOpenGLPFAAuxBuffers = 7,
 NSOpenGLPFAColorSize = 8,
 NSOpenGLPFAAlphaSize = 11,
 NSOpenGLPFADepthSize = 12,
 NSOpenGLPFAStencilSize = 13,
 NSOpenGLPFAAccumSize = 14,
 NSOpenGLPFAMinimumPolicy = 51,
 NSOpenGLPFAMaximumPolicy = 52,
 NSOpenGLPFAOffScreen = 53,
 NSOpenGLPFAFullScreen = 54,
 NSOpenGLPFASampleBuffers = 55,
 NSOpenGLPFASamples = 56,
 NSOpenGLPFAAuxDepthStencil = 57,
 NSOpenGLPFAColorFloat = 58,
 NSOpenGLPFAMultisample = 59,
 NSOpenGLPFASupersample = 60,
 NSOpenGLPFASampleAlpha = 61,
 NSOpenGLPFARendererID = 70,
 NSOpenGLPFASingleRenderer = 71,
 NSOpenGLPFANoRecovery = 72,
 NSOpenGLPFAAccelerated = 73,
 NSOpenGLPFAClosestPolicy = 74,
 NSOpenGLPFABackingStore = 76,
 NSOpenGLPFAWindow = 80,
 NSOpenGLPFACompliant = 83,
 NSOpenGLPFAScreenMask = 84,
 NSOpenGLPFAPixelBuffer = 90,
 NSOpenGLPFARemotePixelBuffer = 91,
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
 NSOpenGLPFAAllowOfflineRenderers = 96,
#endif
 NSOpenGLPFAAcceleratedCompute = 97,
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
 NSOpenGLPFAOpenGLProfile = 99,
#endif
 NSOpenGLPFAVirtualScreenCount = 128,



 NSOpenGLPFARobust = 75,
 NSOpenGLPFAMPSafe = 78,
 NSOpenGLPFAMultiScreen = 81
};
typedef uint32_t NSOpenGLPixelFormatAttribute;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7

enum {
 NSOpenGLProfileVersionLegacy = 0x1000,
 NSOpenGLProfileVersion3_2Core = 0x3200
};
#endif




typedef struct _CGLPixelFormatObject NSOpenGLPixelFormatAuxiliary;

@interface NSOpenGLPixelFormat : NSObject <NSCoding>
{
@private
    NSOpenGLPixelFormatAuxiliary* _pixelFormatAuxiliary;
    NSData* _pixelAttributes;
    NSInteger _reserved1;
    NSInteger _reserved2;
    NSInteger _reserved3;
}

- (id)initWithAttributes:(const NSOpenGLPixelFormatAttribute *)attribs;
- (id)initWithData:(NSData*)attribs;
- (id)initWithCGLPixelFormatObj:(void *)format;

- (NSData*)attributes;
- (void)setAttributes:(NSData*)attribs;

- (void)getValues:(GLint *)vals forAttribute:(NSOpenGLPixelFormatAttribute)attrib forVirtualScreen:(GLint)screen;
- (GLint)numberOfVirtualScreens;

- (void *)CGLPixelFormatObj ;

@end







@interface NSOpenGLPixelBuffer : NSObject
{
@private
    struct _CGLPBufferObject *_pixelBufferAuxiliary;
    void *_reserved1;
    void *_reserved2;
}
- (id)initWithTextureTarget:(GLenum)target textureInternalFormat:(GLenum)format textureMaxMipMapLevel:(GLint)maxLevel pixelsWide:(GLsizei)pixelsWide pixelsHigh:(GLsizei)pixelsHigh;
- (id)initWithCGLPBufferObj:(void *)pbuffer;
- (void *)CGLPBufferObj ;
- (GLsizei)pixelsWide;
- (GLsizei)pixelsHigh;
- (GLenum)textureTarget;
- (GLenum)textureInternalFormat;
- (GLint)textureMaxMipMapLevel;
@end







typedef enum {
    NSOpenGLCPSwapInterval = 222,
    NSOpenGLCPSurfaceOrder = 235,
    NSOpenGLCPSurfaceOpacity = 236,
    NSOpenGLCPSurfaceBackingSize = 304,
    NSOpenGLCPReclaimResources = 308,
    NSOpenGLCPCurrentRendererID = 309,
    NSOpenGLCPGPUVertexProcessing = 310,
    NSOpenGLCPGPUFragmentProcessing = 311,
    NSOpenGLCPHasDrawable = 314,
    NSOpenGLCPMPSwapsInFlight = 315,


    NSOpenGLCPSwapRectangle = 200,
    NSOpenGLCPSwapRectangleEnable = 201,
    NSOpenGLCPRasterizationEnable = 221,
    NSOpenGLCPStateValidation = 301,
    NSOpenGLCPSurfaceSurfaceVolatile = 306
} NSOpenGLContextParameter;





typedef struct _CGLContextObject NSOpenGLContextAuxiliary;

@interface NSOpenGLContext : NSObject
{
@private
 __weak NSView *_view;
 NSOpenGLContextAuxiliary *_contextAuxiliary;
}


- (id)initWithFormat:(NSOpenGLPixelFormat *)format shareContext:(NSOpenGLContext *)share;
- (id)initWithCGLContextObj:(void *)context;


- (void)setView:(NSView *)view;
- (NSView *)view;
- (void)setFullScreen;



- (void)setOffScreen:(void *)baseaddr width:(GLsizei)width height:(GLsizei)height rowbytes:(GLint)rowbytes;

- (void)clearDrawable;
- (void)update;


- (void)flushBuffer;


- (void)makeCurrentContext;
+ (void)clearCurrentContext;
+ (NSOpenGLContext *)currentContext;


- (void)copyAttributesFromContext:(NSOpenGLContext *)context withMask:(GLbitfield)mask;


- (void)setValues:(const GLint *)vals forParameter:(NSOpenGLContextParameter)param;
- (void)getValues:(GLint *)vals forParameter:(NSOpenGLContextParameter)param;



- (void)setCurrentVirtualScreen:(GLint)screen;
- (GLint)currentVirtualScreen;


- (void)createTexture:(GLenum)target fromView:(NSView *)view internalFormat:(GLenum)format;


- (void *)CGLContextObj ;
- (void)setPixelBuffer:(NSOpenGLPixelBuffer *)pixelBuffer cubeMapFace:(GLenum)face mipMapLevel:(GLint)level currentVirtualScreen:(GLint)screen;
- (NSOpenGLPixelBuffer *)pixelBuffer;
- (GLenum)pixelBufferCubeMapFace;
- (GLint)pixelBufferMipMapLevel;
- (void)setTextureImageToPixelBuffer:(NSOpenGLPixelBuffer *)pixelBuffer colorBuffer:(GLenum)source;

@end
