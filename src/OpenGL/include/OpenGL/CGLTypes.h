#ifndef _CGLTYPES_H
#define _CGLTYPES_H

#include <OpenGL/OpenGLAvailability.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __has_feature(assume_nonnull)
#define OPENGL_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define OPENGL_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define OPENGL_ASSUME_NONNULL_BEGIN
#define OPENGL_ASSUME_NONNULL_END
#endif

#if __has_feature(nullability)
#define OPENGL_NULLABLE __nullable
#define OPENGL_NONNULL __nonnull
#else
#define OPENGL_NULLABLE
#define OPENGL_NONNULL
#endif

#if __has_attribute(objc_bridge) && __has_feature(objc_bridge_id) && __has_feature(objc_bridge_id_on_typedefs)
#define OPENGL_BRIDGED_TYPE(T)		__attribute__((objc_bridge(T)))
#else
#define OPENGL_BRIDGED_TYPE(T)
#endif

#if __has_feature(objc_class_property)
#define OPENGL_SWIFT_NAME(name) __attribute__((swift_name(#name)))
#else
#define OPENGL_SWIFT_NAME(name)
#endif

typedef struct _CGLContextObject       *CGLContextObj;
typedef struct _CGLPixelFormatObject   *CGLPixelFormatObj;
typedef struct _CGLRendererInfoObject  *CGLRendererInfoObj;
typedef struct _CGLPBufferObject       *CGLPBufferObj OPENGL_DEPRECATED(10_3, 10_7);

typedef enum _CGLPixelFormatAttribute {
	kCGLPFAAllRenderers                                                   =   1,
	kCGLPFATripleBuffer OPENGL_ENUM_AVAILABLE(10_7)                       =   3,
	kCGLPFADoubleBuffer                                                   =   5,
	kCGLPFAColorSize                                                      =   8,
	kCGLPFAAlphaSize                                                      =  11,
	kCGLPFADepthSize                                                      =  12,
	kCGLPFAStencilSize                                                    =  13,
	kCGLPFAMinimumPolicy                                                  =  51,
	kCGLPFAMaximumPolicy                                                  =  52,
	kCGLPFASampleBuffers OPENGL_ENUM_AVAILABLE(10_2)                      =  55,
	kCGLPFASamples OPENGL_ENUM_AVAILABLE(10_2)                            =  56,
	kCGLPFAColorFloat OPENGL_ENUM_AVAILABLE(10_2)                         =  58,
	kCGLPFAMultisample OPENGL_ENUM_AVAILABLE(10_3)                        =  59,
	kCGLPFASupersample OPENGL_ENUM_AVAILABLE(10_3)                        =  60,
	kCGLPFASampleAlpha OPENGL_ENUM_AVAILABLE(10_3)                        =  61,
	kCGLPFARendererID                                                     =  70,
	kCGLPFANoRecovery                                                     =  72,
	kCGLPFAAccelerated                                                    =  73,
	kCGLPFAClosestPolicy                                                  =  74,
	kCGLPFABackingStore                                                   =  76,
	kCGLPFABackingVolatile OPENGL_ENUM_AVAILABLE(10_7)                    =  77,
	kCGLPFADisplayMask                                                    =  84,
	kCGLPFAAllowOfflineRenderers OPENGL_ENUM_AVAILABLE(10_5)              =  96,
	kCGLPFAAcceleratedCompute OPENGL_ENUM_AVAILABLE(10_6)                 =  97,
	kCGLPFAOpenGLProfile OPENGL_ENUM_AVAILABLE(10_7)                      =  99,
	kCGLPFASupportsAutomaticGraphicsSwitching OPENGL_ENUM_AVAILABLE(10_8) = 101,
	kCGLPFAVirtualScreenCount                                             = 128,
	kCGLPFAAuxBuffers                                  =   7,
	kCGLPFAAccumSize                                   =  14,
	kCGLPFAAuxDepthStencil OPENGL_ENUM_AVAILABLE(10_2) =  57,

	kCGLPFAStereo OPENGL_ENUM_DEPRECATED(10_0, 10_11)        =   6,
	kCGLPFAOffScreen OPENGL_ENUM_DEPRECATED(10_0, 10_7)      =  53,
	kCGLPFAWindow OPENGL_ENUM_DEPRECATED(10_0, 10_9)         =  80,
	kCGLPFACompliant OPENGL_ENUM_DEPRECATED(10_0, 10_9)      =  83,
	kCGLPFAPBuffer OPENGL_ENUM_DEPRECATED(10_3, 10_7)        =  90,
	kCGLPFARemotePBuffer OPENGL_ENUM_DEPRECATED(10_3, 10_9)  =  91,

	kCGLPFASingleRenderer OPENGL_ENUM_DEPRECATED(10_0, 10_9) =  71,
	kCGLPFARobust OPENGL_ENUM_DEPRECATED(10_0, 10_5)         =  75,
	kCGLPFAMPSafe OPENGL_ENUM_DEPRECATED(10_0, 10_5)         =  78,
	kCGLPFAMultiScreen OPENGL_ENUM_DEPRECATED(10_0, 10_5)    =  81,
	kCGLPFAFullScreen OPENGL_ENUM_DEPRECATED(10_0, 10_6)     =  54,
} CGLPixelFormatAttribute;

typedef enum _CGLRendererProperty {
	kCGLRPOffScreen                                          =  53,
	kCGLRPRendererID                                         =  70,
	kCGLRPAccelerated                                        =  73,
	kCGLRPBackingStore                                       =  76,
	kCGLRPWindow                                             =  80,
	kCGLRPCompliant                                          =  83,
	kCGLRPDisplayMask                                        =  84,
	kCGLRPBufferModes                                        = 100, 
	kCGLRPColorModes                                         = 103,
	kCGLRPAccumModes                                         = 104,
	kCGLRPDepthModes                                         = 105,
	kCGLRPStencilModes                                       = 106,
	kCGLRPMaxAuxBuffers                                      = 107,
	kCGLRPMaxSampleBuffers OPENGL_ENUM_AVAILABLE(10_2)       = 108,
	kCGLRPMaxSamples OPENGL_ENUM_AVAILABLE(10_2)             = 109,
	kCGLRPSampleModes OPENGL_ENUM_AVAILABLE(10_3)            = 110,
	kCGLRPSampleAlpha OPENGL_ENUM_AVAILABLE(10_3)            = 111,
	kCGLRPGPUVertProcCapable OPENGL_ENUM_AVAILABLE(10_4)     = 122,
	kCGLRPGPUFragProcCapable OPENGL_ENUM_AVAILABLE(10_4)     = 123,
	kCGLRPRendererCount                                      = 128,
	kCGLRPOnline OPENGL_ENUM_AVAILABLE(10_5)                 = 129,
	kCGLRPAcceleratedCompute OPENGL_ENUM_AVAILABLE(10_6)     = 130,
	kCGLRPVideoMemoryMegabytes OPENGL_ENUM_AVAILABLE(10_7)   = 131,
	kCGLRPTextureMemoryMegabytes OPENGL_ENUM_AVAILABLE(10_7) = 132,
	kCGLRPMajorGLVersion OPENGL_ENUM_AVAILABLE(10_9)         = 133,

	kCGLRPRegistryIDLow OPENGL_ENUM_AVAILABLE(10_13)         = 140,
	kCGLRPRegistryIDHigh OPENGL_ENUM_AVAILABLE(10_13)        = 141,
	kCGLRPRemovable OPENGL_ENUM_AVAILABLE(10_13)             = 142,
	
	kCGLRPRobust OPENGL_ENUM_DEPRECATED(10_0, 10_5)      =  75,
	kCGLRPMPSafe OPENGL_ENUM_DEPRECATED(10_0, 10_5)      =  78,
	kCGLRPMultiScreen OPENGL_ENUM_DEPRECATED(10_0, 10_5) =  81,
	kCGLRPFullScreen OPENGL_ENUM_DEPRECATED(10_0, 10_6)  =  54,
	kCGLRPVideoMemory OPENGL_ENUM_DEPRECATED_MSG(10_0, 10_7, "Use kCGLRPVideoMemoryMegabytes") = 120,
	kCGLRPTextureMemory OPENGL_ENUM_DEPRECATED_MSG(10_0, 10_7, "Use kCGLRPTextureMemoryMegabytes") = 121,
} CGLRendererProperty;

typedef enum _CGLContextEnable {
	kCGLCESwapRectangle                                       = 201,
	kCGLCESwapLimit OPENGL_ENUM_AVAILABLE(10_2)               = 203,
	kCGLCERasterization                                       = 221,
	kCGLCEStateValidation                                     = 301,
	kCGLCESurfaceBackingSize OPENGL_ENUM_AVAILABLE(10_3)      = 305,
	kCGLCEDisplayListOptimization OPENGL_ENUM_AVAILABLE(10_3) = 307,
	kCGLCEMPEngine OPENGL_ENUM_AVAILABLE(10_4)                = 313,
	kCGLCECrashOnRemovedFunctions OPENGL_ENUM_AVAILABLE(10_7) = 316
} CGLContextEnable;

typedef enum _CGLGPURestartStatus {
	kCGLCPGPURestartStatusNone        = 0,
	kCGLCPGPURestartStatusCaused      = 1,
	kCGLCPGPURestartStatusBlacklisted = 2,
} CGLGPURestartStatus;

typedef enum _CGLContextParameter {
	kCGLCPSwapRectangle                                      = 200,
	kCGLCPSwapInterval                                       = 222,
	kCGLCPDispatchTableSize OPENGL_ENUM_AVAILABLE(10_3)      = 224,
	kCGLCPClientStorage                                      = 226,
	kCGLCPSurfaceTexture OPENGL_ENUM_DEPRECATED(10_3, 10_9)  = 228,
	kCGLCPSurfaceOrder OPENGL_ENUM_AVAILABLE(10_2)           = 235,
	kCGLCPSurfaceOpacity OPENGL_ENUM_AVAILABLE(10_2)         = 236,
	kCGLCPSurfaceBackingSize OPENGL_ENUM_AVAILABLE(10_3)     = 304,
	kCGLCPSurfaceSurfaceVolatile OPENGL_ENUM_AVAILABLE(10_3) = 306,
	kCGLCPReclaimResources OPENGL_ENUM_AVAILABLE(10_4)       = 308,
	kCGLCPCurrentRendererID OPENGL_ENUM_AVAILABLE(10_4)      = 309,
	kCGLCPGPUVertexProcessing OPENGL_ENUM_AVAILABLE(10_4)    = 310,
	kCGLCPGPUFragmentProcessing OPENGL_ENUM_AVAILABLE(10_4)  = 311,
	kCGLCPHasDrawable OPENGL_ENUM_AVAILABLE(10_5)            = 314,
	kCGLCPMPSwapsInFlight OPENGL_ENUM_AVAILABLE(10_5)        = 315,
	kCGLCPGPURestartStatus OPENGL_ENUM_AVAILABLE(10_10)      = 317,
	kCGLCPAbortOnGPURestartStatusBlacklisted OPENGL_ENUM_AVAILABLE(10_10) = 318,
	kCGLCPSupportGPURestart OPENGL_ENUM_AVAILABLE(10_10)     = 319,
	kCGLCPSupportSeparateAddressSpace OPENGL_ENUM_AVAILABLE(10_10) = 320,
	kCGLCPContextPriorityRequest OPENGL_ENUM_AVAILABLE(10_10) = 608,
} CGLContextParameter;

typedef enum
{
	kCGLCPContextPriorityRequestHigh   = 0,
	kCGLCPContextPriorityRequestNormal = 1,
	kCGLCPContextPriorityRequestLow    = 2
} CGLCPContextPriorityRequest;

typedef enum _CGLGlobalOption {
	kCGLGOFormatCacheSize                           = 501,
	kCGLGOClearFormatCache                          = 502,
	kCGLGORetainRenderers                           = 503,
	kCGLGOUseBuildCache OPENGL_ENUM_AVAILABLE(10_5) = 506,
	
	kCGLGOResetLibrary OPENGL_ENUM_DEPRECATED(10_0, 10_4)     = 504,
	kCGLGOUseErrorHandler OPENGL_ENUM_DEPRECATED(10_0, 10_7)  = 505,
} CGLGlobalOption;

typedef enum _CGLOpenGLProfile {
	kCGLOGLPVersion_Legacy OPENGL_ENUM_AVAILABLE(10_7)   = 0x1000,
	kCGLOGLPVersion_3_2_Core OPENGL_ENUM_AVAILABLE(10_7) = 0x3200,
	kCGLOGLPVersion_GL3_Core OPENGL_ENUM_AVAILABLE(10_7) = 0x3200,
	kCGLOGLPVersion_GL4_Core OPENGL_ENUM_AVAILABLE(10_9) = 0x4100,
} CGLOpenGLProfile OPENGL_AVAILABLE(10_7);

typedef enum _CGLError {
	kCGLNoError            = 0,
	kCGLBadAttribute       = 10000,
	kCGLBadProperty        = 10001,
	kCGLBadPixelFormat     = 10002,	
	kCGLBadRendererInfo    = 10003,
	kCGLBadContext         = 10004,
	kCGLBadDrawable        = 10005,
	kCGLBadDisplay         = 10006,
	kCGLBadState           = 10007,
	kCGLBadValue           = 10008,
	kCGLBadMatch           = 10009,
	kCGLBadEnumeration     = 10010,
	kCGLBadOffScreen       = 10011,
	kCGLBadFullScreen      = 10012,
	kCGLBadWindow          = 10013,
	kCGLBadAddress         = 10014,
	kCGLBadCodeModule      = 10015,
	kCGLBadAlloc           = 10016,
	kCGLBadConnection      = 10017
} CGLError;

#define kCGLMonoscopicBit   0x00000001
#define kCGLStereoscopicBit 0x00000002
#define kCGLSingleBufferBit 0x00000004
#define kCGLDoubleBufferBit 0x00000008
#define kCGLTripleBufferBit 0x00000010

#define kCGL0Bit            0x00000001
#define kCGL1Bit            0x00000002
#define kCGL2Bit            0x00000004
#define kCGL3Bit            0x00000008
#define kCGL4Bit            0x00000010
#define kCGL5Bit            0x00000020
#define kCGL6Bit            0x00000040
#define kCGL8Bit            0x00000080
#define kCGL10Bit           0x00000100
#define kCGL12Bit           0x00000200
#define kCGL16Bit           0x00000400
#define kCGL24Bit           0x00000800
#define kCGL32Bit           0x00001000
#define kCGL48Bit           0x00002000
#define kCGL64Bit           0x00004000
#define kCGL96Bit           0x00008000
#define kCGL128Bit          0x00010000

#define kCGLRGB444Bit       0x00000040 
#define kCGLARGB4444Bit     0x00000080
#define kCGLRGB444A8Bit     0x00000100
#define kCGLRGB555Bit       0x00000200
#define kCGLARGB1555Bit     0x00000400
#define kCGLRGB555A8Bit     0x00000800
#define kCGLRGB565Bit       0x00001000
#define kCGLRGB565A8Bit     0x00002000
#define kCGLRGB888Bit       0x00004000
#define kCGLARGB8888Bit     0x00008000
#define kCGLRGB888A8Bit     0x00010000
#define kCGLRGB101010Bit    0x00020000
#define kCGLARGB2101010Bit  0x00040000
#define kCGLRGB101010_A8Bit 0x00080000
#define kCGLRGB121212Bit    0x00100000
#define kCGLARGB12121212Bit 0x00200000
#define kCGLRGB161616Bit    0x00400000
#define kCGLRGBA16161616Bit 0x00800000 
#define kCGLRGBFloat64Bit   0x01000000
#define kCGLRGBAFloat64Bit  0x02000000
#define kCGLRGBFloat128Bit  0x04000000
#define kCGLRGBAFloat128Bit 0x08000000
#define kCGLRGBFloat256Bit  0x10000000
#define kCGLRGBAFloat256Bit 0x20000000 

#define kCGLSupersampleBit 0x00000001
#define kCGLMultisampleBit 0x00000002

#define kCGLARGB16161616Bit kCGLRGBA16161616Bit


#ifdef __cplusplus
}
#endif

#endif