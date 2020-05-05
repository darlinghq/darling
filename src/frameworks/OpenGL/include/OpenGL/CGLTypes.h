#ifndef _CGL_TYPES_H_
#define _CGL_TYPES_H_

#include <stdint.h>

#define CGL_EXPORT extern

typedef enum {
    kCGLNoError = 0,
    kCGLBadAttribute = 10000,
    kCGLBadProperty = 10001,
    kCGLBadPixelFormat = 10002,
    kCGLBadRendererInfo = 10003,
    kCGLBadContext = 10004,
    kCGLBadDrawable = 10005,
    kCGLBadDisplay = 10006,
    kCGLBadState = 10007,
    kCGLBadValue = 10008,
    kCGLBadMatch = 10009,
    kCGLBadEnumeration = 10010,
    kCGLBadOffScreen = 10011,
    kCGLBadFullScreen = 10012,
    kCGLBadWindow = 10013,
    kCGLBadAddress = 10014,
    kCGLBadCodeModule = 10015,
    kCGLBadAlloc = 10016,
    kCGLBadConnection = 10017,
} CGLError;

enum {
    kCGLPFAAllRenderers = 1,
    kCGLPFADoubleBuffer = 5,
    kCGLPFAStereo = 6,
    kCGLPFAAuxBuffers = 7,
    kCGLPFAColorSize = 8,
    kCGLPFAAlphaSize = 11,
    kCGLPFADepthSize = 12,
    kCGLPFAStencilSize = 13,
    kCGLPFAAccumSize = 14,
    kCGLPFAMinimumPolicy = 51,
    kCGLPFAMaximumPolicy = 52,
    kCGLPFAOffScreen = 53,
    kCGLPFAFullScreen = 54,
    kCGLPFASampleBuffers = 55,
    kCGLPFASamples = 56,
    kCGLPFAAuxDepthStencil = 57,
    kCGLPFAColorFloat = 58,
    kCGLPFAMultisample = 59,
    kCGLPFASupersample = 60,
    kCGLPFASampleAlpha = 61,
    kCGLPFARendererID = 70,
    kCGLPFASingleRenderer = 71,
    kCGLPFANoRecovery = 72,
    kCGLPFAAccelerated = 73,
    kCGLPFAClosestPolicy = 74,
    kCGLPFARobust = 75,
    kCGLPFABackingStore = 76,
    kCGLPFAMPSafe = 78,
    kCGLPFAWindow = 80,
    kCGLPFAMultiScreen = 81,
    kCGLPFACompliant = 83,
    kCGLPFADisplayMask = 84,
    kCGLPFAPBuffer = 90,
    kCGLPFARemotePBuffer = 91,
    kCGLPFAAllowOfflineRenderers = 96,
    kCGLPFAAcceleratedCompute = 97,
    kCGLPFAVirtualScreenCount = 128,
};

typedef uint32_t CGLPixelFormatAttribute;

typedef enum {
    kCGLCPSwapInterval = 222,
    kCGLCPSurfaceOpacity = 236,
    kCGLCPSurfaceBackingSize = 304,
} CGLContextParameter;

typedef enum _CGLRendererProperty {
   kCGLRPOffScreen           =  53,
   kCGLRPFullScreen          =  54,
   kCGLRPRendererID          =  70,
   kCGLRPAccelerated         =  73,
   kCGLRPRobust              =  75,
   kCGLRPBackingStore        =  76,
   kCGLRPMPSafe              =  78,
   kCGLRPWindow              =  80,
   kCGLRPMultiScreen         =  81,
   kCGLRPCompliant           =  83,
   kCGLRPDisplayMask         =  84,
   kCGLRPBufferModes         = 100,
   kCGLRPColorModes          = 103,
   kCGLRPAccumModes          = 104,
   kCGLRPDepthModes          = 105,
   kCGLRPStencilModes        = 106,
   kCGLRPMaxAuxBuffers       = 107,
   kCGLRPMaxSampleBuffers    = 108,
   kCGLRPMaxSamples          = 109,
   kCGLRPVideoMemory         = 120,
   kCGLRPTextureMemory       = 121,
   kCGLRPRendererCount       = 128,
} CGLRendererProperty;

typedef struct _CGLContextObj *CGLContextObj;
typedef struct _CGLPixelFormatObj *CGLPixelFormatObj;
typedef struct _CGLPBufferObj *CGLPBufferObj;
typedef struct _CGLRendererInfoObj *CGLRendererInfoObj;

#endif /* _CGL_TYPES_H_ */
