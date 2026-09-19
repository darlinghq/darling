#ifndef CVPIXELBUFFER_H
#define CVPIXELBUFFER_H

#include <sys/cdefs.h>
#include <CoreVideo/CVImageBuffer.h>
#include <CoreVideo/CVReturn.h>

__BEGIN_DECLS

enum
{
	kCVPixelFormatType_1Monochrome    = 0x00000001,
	kCVPixelFormatType_2Indexed       = 0x00000002,
	kCVPixelFormatType_4Indexed       = 0x00000004,
	kCVPixelFormatType_8Indexed       = 0x00000008,
	kCVPixelFormatType_1IndexedGray_WhiteIsZero = 0x00000021,
	kCVPixelFormatType_2IndexedGray_WhiteIsZero = 0x00000022,
	kCVPixelFormatType_4IndexedGray_WhiteIsZero = 0x00000024,
	kCVPixelFormatType_8IndexedGray_WhiteIsZero = 0x00000028,
	kCVPixelFormatType_16BE555        = 0x00000010,
	kCVPixelFormatType_16LE555        = 'L555',
	kCVPixelFormatType_16LE5551       = '5551',
	kCVPixelFormatType_16BE565        = 'B565',
	kCVPixelFormatType_16LE565        = 'L565',
	kCVPixelFormatType_24RGB          = 0x00000018,
	kCVPixelFormatType_24BGR          = '24BG',
	kCVPixelFormatType_32ARGB         = 0x00000020,
	kCVPixelFormatType_32BGRA         = 'BGRA',
	kCVPixelFormatType_32ABGR         = 'ABGR',
	kCVPixelFormatType_32RGBA         = 'RGBA',
	kCVPixelFormatType_64ARGB         = 'b64a',
	kCVPixelFormatType_48RGB          = 'b48r',
	kCVPixelFormatType_32AlphaGray    = 'b32a',
	kCVPixelFormatType_16Gray         = 'b16g',
	kCVPixelFormatType_30RGB          = 'R10k',
	kCVPixelFormatType_422YpCbCr8     = '2vuy',
	kCVPixelFormatType_4444YpCbCrA8   = 'v408',
	kCVPixelFormatType_4444YpCbCrA8R  = 'r408',
	kCVPixelFormatType_4444AYpCbCr8   = 'y408',
	kCVPixelFormatType_4444AYpCbCr16  = 'y416',
	kCVPixelFormatType_444YpCbCr8     = 'v308',
	kCVPixelFormatType_422YpCbCr16    = 'v216',
	kCVPixelFormatType_422YpCbCr10    = 'v210',
	kCVPixelFormatType_444YpCbCr10    = 'v410',
	kCVPixelFormatType_420YpCbCr8Planar = 'y420',
	kCVPixelFormatType_420YpCbCr8PlanarFullRange    = 'f420',
	kCVPixelFormatType_422YpCbCr_4A_8BiPlanar = 'a2vy',
	kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange = '420v',
	kCVPixelFormatType_420YpCbCr8BiPlanarFullRange  = '420f',
	kCVPixelFormatType_422YpCbCr8_yuvs = 'yuvs',
	kCVPixelFormatType_422YpCbCr8FullRange = 'yuvf',
	kCVPixelFormatType_OneComponent8  = 'L008',
	kCVPixelFormatType_TwoComponent8  = '2C08',
	kCVPixelFormatType_30RGBLEPackedWideGamut = 'w30r',
	kCVPixelFormatType_ARGB2101010LEPacked = 'l10r',
	kCVPixelFormatType_OneComponent16Half  = 'L00h',
	kCVPixelFormatType_OneComponent32Float = 'L00f',
	kCVPixelFormatType_TwoComponent16Half  = '2C0h',
	kCVPixelFormatType_TwoComponent32Float = '2C0f',
	kCVPixelFormatType_64RGBAHalf          = 'RGhA',
	kCVPixelFormatType_128RGBAFloat        = 'RGfA',
	kCVPixelFormatType_14Bayer_GRBG        = 'grb4',
	kCVPixelFormatType_14Bayer_RGGB        = 'rgg4',
	kCVPixelFormatType_14Bayer_BGGR        = 'bgg4',
	kCVPixelFormatType_14Bayer_GBRG        = 'gbr4',
	kCVPixelFormatType_DisparityFloat16    = 'hdis',
	kCVPixelFormatType_DisparityFloat32    = 'fdis',
	kCVPixelFormatType_DepthFloat16            = 'hdep',
	kCVPixelFormatType_DepthFloat32            = 'fdep',
	kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange = 'x420',
	kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange = 'x422',
	kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange = 'x444',
	kCVPixelFormatType_420YpCbCr10BiPlanarFullRange  = 'xf20',
	kCVPixelFormatType_422YpCbCr10BiPlanarFullRange  = 'xf22',
	kCVPixelFormatType_444YpCbCr10BiPlanarFullRange  = 'xf44',
	kCVPixelFormatType_420YpCbCr8VideoRange_8A_TriPlanar   = 'v0a8',
};

extern const CFStringRef kCVPixelBufferIOSurfacePropertiesKey;
extern const CFStringRef kCVPixelBufferOpenGLCompatibilityKey;
extern const CFStringRef kCVPixelBufferPixelFormatTypeKey;
extern const CFStringRef kCVPixelBufferMetalCompatibilityKey;
extern const CFStringRef kCVPixelBufferBytesPerRowAlignmentKey;
extern const CFStringRef kCVPixelBufferHeightKey;
extern const CFStringRef kCVPixelBufferWidthKey;

typedef CVImageBufferRef CVPixelBufferRef;
typedef CVOptionFlags CVPixelBufferLockFlags;
typedef void (*CVPixelBufferReleaseBytesCallback)(void *releaseRefCon, const void *baseAddress);
typedef void (*CVPixelBufferReleasePlanarBytesCallback)(void *releaseRefCon, const void *dataPtr, size_t dataSize,
	size_t numberOfPlanes, const void *planeAddresses[]);

CV_EXPORT CFTypeID CVPixelBufferGetTypeID(void);
CV_EXPORT CVPixelBufferRef CVPixelBufferRetain(CVPixelBufferRef pixelBuffer);
CV_EXPORT void CVPixelBufferRelease(CVPixelBufferRef pixelBuffer);

CV_EXPORT CVReturn CVPixelBufferCreate(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef *pixelBufferOut);

CV_EXPORT CVReturn CVPixelBufferCreateWithBytes(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType,
	void *baseAddress, size_t bytesPerRow, CVPixelBufferReleaseBytesCallback releaseCallback, void *releaseRefCon,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef  _Nullable *pixelBufferOut);

CV_EXPORT CVReturn CVPixelBufferCreateWithPlanarBytes(CFAllocatorRef allocator, size_t width, size_t height,
	OSType pixelFormatType, void *dataPtr, size_t dataSize, size_t numberOfPlanes, void *planeBaseAddress[],
	size_t planeWidth[], size_t planeHeight[], size_t planeBytesPerRow[],
	CVPixelBufferReleasePlanarBytesCallback releaseCallback, void *releaseRefCon,
	CFDictionaryRef pixelBufferAttributes, CVPixelBufferRef *pixelBufferOut);

CV_EXPORT size_t CVPixelBufferGetWidth(CVPixelBufferRef pixelBuffer);
CV_EXPORT size_t CVPixelBufferGetHeight(CVPixelBufferRef pixelBuffer);
CV_EXPORT size_t CVPixelBufferGetBytesPerRow(CVPixelBufferRef pixelBuffer);
CV_EXPORT OSType CVPixelBufferGetPixelFormatType(CVPixelBufferRef pixelBuffer);
CV_EXPORT size_t CVPixelBufferGetDataSize(CVPixelBufferRef pixelBuffer);
CV_EXPORT void* CV_NULLABLE CVPixelBufferGetBaseAddress(CVPixelBufferRef pixelBuffer);
CV_EXPORT size_t CVPixelBufferGetPlaneCount(CVPixelBufferRef pixelBuffer);
CV_EXPORT Boolean CVPixelBufferIsPlanar(CVPixelBufferRef pixelBuffer);
CV_EXPORT void* CV_NULLABLE CVPixelBufferGetBaseAddressOfPlane(CVPixelBufferRef pixelBuffer, size_t planeIndex);
CV_EXPORT size_t CVPixelBufferGetBytesPerRowOfPlane(CVPixelBufferRef pixelBuffer, size_t planeIndex);
CV_EXPORT size_t CVPixelBufferGetWidthOfPlane(CVPixelBufferRef pixelBuffer, size_t planeIndex);
CV_EXPORT size_t CVPixelBufferGetHeightOfPlane(CVPixelBufferRef pixelBuffer, size_t planeIndex);
CV_EXPORT void* CV_NULLABLE CVPixelBufferGetIOSurface(CVPixelBufferRef pixelBuffer);
CV_EXPORT CVReturn CVPixelBufferLockBaseAddress(CVPixelBufferRef pixelBuffer, CVPixelBufferLockFlags lockFlags);
CV_EXPORT CVReturn CVPixelBufferUnlockBaseAddress(CVPixelBufferRef pixelBuffer, CVPixelBufferLockFlags unlockFlags);

CV_EXPORT CVReturn CVMetalTextureCacheCreate(
	CFAllocatorRef allocator,
	CFDictionaryRef cacheAttributes,
	/* id<MTLDevice> */ void *metalDevice,
	CFDictionaryRef textureAttributes,
	/* CVMetalTextureCacheRef* */ void *cacheOut);

CV_EXPORT CVReturn CVMetalTextureCacheCreateTextureFromImage(
	CFAllocatorRef allocator,
	/* CVMetalTextureCacheRef */ void *textureCache,
	CVImageBufferRef sourceImage,
	CFDictionaryRef textureAttributes,
	/* MTLPixelFormat */ int pixelFormat,
	size_t width,
	size_t height,
	size_t planeIndex,
	/* CVMetalTextureRef* */ void *textureOut);

CV_EXPORT /* id<MTLTexture> */ void* CVMetalTextureGetTexture(/* CVMetalTextureRef */ void *image);

__END_DECLS

#include <CoreVideo/CVPixelBufferIOSurface.h>

#endif
