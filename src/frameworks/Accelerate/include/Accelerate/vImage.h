/*
 This file is part of Darling.

 Copyright (C) 2017-2026 Lubos Dolezel

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

// The part of the vImage API that Darling implements: image buffers, the buffer and CGImage utilities and the
// vImageConverter / vImageConvert_AnyToAny conversion between CGImage-style pixel formats. The names, values and
// semantics follow the public vImage documentation.

#ifndef _Accelerate_vImage_H_
#define _Accelerate_vImage_H_

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long vImagePixelCount;

typedef struct vImage_Buffer {
    void *data;
    vImagePixelCount height;
    vImagePixelCount width;
    size_t rowBytes;
} vImage_Buffer;

typedef uint32_t vImage_Flags;
typedef ssize_t vImage_Error;

enum {
    kvImageNoError = 0,
    kvImageRoiLargerThanInputBuffer = -21766,
    kvImageInvalidKernelSize = -21767,
    kvImageInvalidEdgeStyle = -21768,
    kvImageInvalidOffset_X = -21769,
    kvImageInvalidOffset_Y = -21770,
    kvImageMemoryAllocationError = -21771,
    kvImageNullPointerArgument = -21772,
    kvImageInvalidParameter = -21773,
    kvImageBufferSizeMismatch = -21774,
    kvImageUnknownFlagsBit = -21775,
    kvImageInternalError = -21776,
    kvImageInvalidRowBytes = -21777,
    kvImageInvalidImageFormat = -21778,
    kvImageColorSyncIsAbsent = -21779,
    kvImageOutOfPlaceOperationRequired = -21780,
    kvImageInvalidImageObject = -21781,
    kvImageInvalidCVImageFormat = -21782,
    kvImageUnsupportedConversion = -21783,
    kvImageCoreVideoIsAbsent = -21784
};

enum {
    kvImageNoFlags = 0,
    kvImageLeaveAlphaUnchanged = 1,
    kvImageCopyInPlace = 2,
    kvImageBackgroundColorFill = 4,
    kvImageEdgeExtend = 8,
    kvImageDoNotTile = 16,
    kvImageHighQualityResampling = 32,
    kvImageTruncateKernel = 64,
    kvImageGetTempBufferSize = 128,
    kvImagePrintDiagnosticsToConsole = 256,
    kvImageNoAllocate = 512,
    kvImageHDRContent = 1024,
    kvImageDoNotClamp = 2048
};

typedef uint8_t Pixel_8888[4];

// A pixel format that CoreGraphics can describe. colorSpace NULL means the device RGB colour space (the sRGB
// primaries and curve); version must be 0; decode is NULL or two values per colour component.
typedef struct vImage_CGImageFormat {
    uint32_t bitsPerComponent;
    uint32_t bitsPerPixel;
    CGColorSpaceRef colorSpace;
    CGBitmapInfo bitmapInfo;
    uint32_t version;
    const CGFloat *decode;
    CGColorRenderingIntent renderingIntent;
} vImage_CGImageFormat;

typedef struct vImageConverter *vImageConverterRef;

// CoreGraphics' colour conversion object (used only to say that Darling cannot create converters from it).
typedef struct CGColorConversionInfo *CGColorConversionInfoRef;

// The kind of data in one buffer of a converter's source or destination list. The list ends with kvImageBufferTypeCode_EndOfList.
typedef enum {
    kvImageBufferTypeCode_EndOfList = 0,
    kvImageBufferTypeCode_ColorSpaceChannel1,
    kvImageBufferTypeCode_ColorSpaceChannel2,
    kvImageBufferTypeCode_ColorSpaceChannel3,
    kvImageBufferTypeCode_ColorSpaceChannel4,
    kvImageBufferTypeCode_ColorSpaceChannel5,
    kvImageBufferTypeCode_ColorSpaceChannel6,
    kvImageBufferTypeCode_ColorSpaceChannel7,
    kvImageBufferTypeCode_ColorSpaceChannel8,
    kvImageBufferTypeCode_ColorSpaceChannel9,
    kvImageBufferTypeCode_ColorSpaceChannel10,
    kvImageBufferTypeCode_ColorSpaceChannel11,
    kvImageBufferTypeCode_ColorSpaceChannel12,
    kvImageBufferTypeCode_ColorSpaceChannel13,
    kvImageBufferTypeCode_ColorSpaceChannel14,
    kvImageBufferTypeCode_ColorSpaceChannel15,
    kvImageBufferTypeCode_ColorSpaceChannel16,
    kvImageBufferTypeCode_Alpha,
    kvImageBufferTypeCode_Indexed,
    kvImageBufferTypeCode_CVPixelBuffer_YCbCr,
    kvImageBufferTypeCode_Luminance,
    kvImageBufferTypeCode_Chroma,
    kvImageBufferTypeCode_Cb,
    kvImageBufferTypeCode_Cr,
    kvImageBufferTypeCode_CGFormat,
    kvImageBufferTypeCode_Chunky,
    kvImageBufferTypeCode_UniqueFormatCount,
    kvImageBufferTypeCode_Monochrome = kvImageBufferTypeCode_ColorSpaceChannel1,
    kvImageBufferTypeCode_RGB_Red = kvImageBufferTypeCode_ColorSpaceChannel1,
    kvImageBufferTypeCode_RGB_Green = kvImageBufferTypeCode_ColorSpaceChannel2,
    kvImageBufferTypeCode_RGB_Blue = kvImageBufferTypeCode_ColorSpaceChannel3,
    kvImageBufferTypeCode_CMYK_Cyan = kvImageBufferTypeCode_ColorSpaceChannel1,
    kvImageBufferTypeCode_CMYK_Magenta = kvImageBufferTypeCode_ColorSpaceChannel2,
    kvImageBufferTypeCode_CMYK_Yellow = kvImageBufferTypeCode_ColorSpaceChannel3,
    kvImageBufferTypeCode_CMYK_Black = kvImageBufferTypeCode_ColorSpaceChannel4,
    kvImageBufferTypeCode_XYZ_X = kvImageBufferTypeCode_ColorSpaceChannel1,
    kvImageBufferTypeCode_XYZ_Y = kvImageBufferTypeCode_ColorSpaceChannel2,
    kvImageBufferTypeCode_XYZ_Z = kvImageBufferTypeCode_ColorSpaceChannel3,
    kvImageBufferTypeCode_LAB_L = kvImageBufferTypeCode_ColorSpaceChannel1,
    kvImageBufferTypeCode_LAB_A = kvImageBufferTypeCode_ColorSpaceChannel2,
    kvImageBufferTypeCode_LAB_B = kvImageBufferTypeCode_ColorSpaceChannel3
} vImageBufferTypeCode;

// ---- vImage_Buffer utilities

// Fills in buf's height, width and the preferred rowBytes for pixels of pixelBits bits and allocates the pixels (free() them).
// With kvImageNoAllocate buf->data is NULL and the preferred alignment is returned (a positive value) instead.
vImage_Error vImageBuffer_Init(vImage_Buffer *buf, vImagePixelCount height, vImagePixelCount width, uint32_t pixelBits, vImage_Flags flags);

// The size of the buffer as a CGSize, rounded down where a CGFloat cannot hold it exactly.
CGSize vImageBuffer_GetSize(const vImage_Buffer *buf);

// ---- vImage_CGImageFormat utilities

uint32_t vImageCGImageFormat_GetComponentCount(const vImage_CGImageFormat *format);
Boolean vImageCGImageFormat_IsEqual(const vImage_CGImageFormat *f1, const vImage_CGImageFormat *f2);

// ---- vImage_Buffer and CGImage

vImage_Error vImageBuffer_InitWithCGImage(vImage_Buffer *buf, vImage_CGImageFormat *format, const CGFloat *backgroundColor, CGImageRef image, vImage_Flags flags);

CGImageRef vImageCreateCGImageFromBuffer(const vImage_Buffer *buf, const vImage_CGImageFormat *format,
                                         void (*callback)(void *userData, void *buf_data), void *userData,
                                         vImage_Flags flags, vImage_Error *error);

// ---- vImageConverter

void vImageConverter_Retain(vImageConverterRef converter);
void vImageConverter_Release(vImageConverterRef converter);

vImageConverterRef vImageConverter_CreateWithCGImageFormat(const vImage_CGImageFormat *srcFormat, const vImage_CGImageFormat *destFormat,
                                                           const CGFloat *backgroundColor, vImage_Flags flags, vImage_Error *error);

// These two need ColorSync, which Darling does not have: they fail with kvImageColorSyncIsAbsent.
vImageConverterRef vImageConverter_CreateWithColorSyncCodeFragment(CFTypeRef codeFragment, const vImage_CGImageFormat *srcFormat,
                                                                   const vImage_CGImageFormat *destFormat, const CGFloat *backgroundColor,
                                                                   vImage_Flags flags, vImage_Error *error);
vImageConverterRef vImageConverter_CreateWithCGColorConversionInfo(CGColorConversionInfoRef colorConversionInfoRef,
                                                                   const vImage_CGImageFormat *sFormat, const vImage_CGImageFormat *dFormat,
                                                                   const CGFloat *bg, vImage_Flags flags, vImage_Error *error);

// kvImageNoError if the converter can work in place (srcs[i].data == dests[i].data and equal rowBytes); srcs and dests are both NULL or both set.
vImage_Error vImageConverter_MustOperateOutOfPlace(const vImageConverterRef converter, const vImage_Buffer *srcs, const vImage_Buffer *dests, vImage_Flags flags);

unsigned long vImageConverter_GetNumberOfSourceBuffers(const vImageConverterRef converter);
unsigned long vImageConverter_GetNumberOfDestinationBuffers(const vImageConverterRef converter);
const vImageBufferTypeCode *vImageConverter_GetSourceBufferOrder(vImageConverterRef converter);
const vImageBufferTypeCode *vImageConverter_GetDestinationBufferOrder(vImageConverterRef converter);

// Converts the pixels of srcs to the format of dests; the source may be larger than the destination, not smaller.
vImage_Error vImageConvert_AnyToAny(const vImageConverterRef converter, const vImage_Buffer *srcs, const vImage_Buffer *dests,
                                    void *tempBuffer, vImage_Flags flags);

// ---- pixel format conversion, filtering and scaling of 8-bit ARGB images and planes

vImage_Error vImageBoxConvolve_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer, vImagePixelCount srcOffsetToROI_X,
                                        vImagePixelCount srcOffsetToROI_Y, uint32_t kernel_height, uint32_t kernel_width,
                                        const uint8_t *backgroundColor, vImage_Flags flags);
vImage_Error vImageConvolve_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer,
                                     vImagePixelCount srcOffsetToROI_X, vImagePixelCount srcOffsetToROI_Y,
                                     const int16_t *kernel, uint32_t kernel_height, uint32_t kernel_width,
                                     int32_t divisor, const Pixel_8888 backgroundColor, vImage_Flags flags);

// Fills every pixel of dest with color (the four channel bytes in memory order).
vImage_Error vImageBufferFill_ARGB8888(const vImage_Buffer *dest, const Pixel_8888 color, vImage_Flags flags);

vImage_Error vImageConvert_ARGB1555toARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_ARGB8888toARGB1555(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_BGRA8888toRGB565(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_Planar4toPlanar8(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_RGB565toARGB8888(uint8_t alpha, const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_RGB565toBGRA8888(uint8_t alpha, const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_RGBA8888toRGB565(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_Planar16FtoPlanarF(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);
vImage_Error vImageConvert_PlanarFtoPlanar16F(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);

vImage_Error vImagePermuteChannels_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, const uint8_t permuteMap[4], vImage_Flags flags);

vImage_Error vImageScale_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, void *tempBuffer, vImage_Flags flags);

vImage_Error vImageUnpremultiplyData_ARGB8888(const vImage_Buffer *src, const vImage_Buffer *dest, vImage_Flags flags);

#ifdef __cplusplus
}
#endif

#endif
