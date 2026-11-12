/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

/*
 * VideoToolbox's sessions. Darling has no hardware codecs, nor software ones behind this API yet, so
 * sessions fail the way they do on a device without the codec: decoders with
 * kVTCouldNotFindVideoDecoderErr, encoders with kVTCouldNotFindVideoEncoderErr, and
 * VTIsHardwareDecodeSupported is false. Clients then use their software paths.
 */

#include <VideoToolbox/VideoToolbox.h>

#pragma mark - Sessions in general

OSStatus VTSessionSetProperty(VTSessionRef session, CFStringRef propertyKey, CFTypeRef propertyValue)
{
	return kVTInvalidSessionErr;
}

OSStatus VTSessionSetProperties(VTSessionRef session, CFDictionaryRef propertyDictionary)
{
	return kVTInvalidSessionErr;
}

OSStatus VTSessionCopyProperty(VTSessionRef session, CFStringRef propertyKey, CFAllocatorRef allocator,
	void *propertyValueOut)
{
	if (propertyValueOut != NULL)
		*(CFTypeRef *) propertyValueOut = NULL;
	return kVTInvalidSessionErr;
}

OSStatus VTSessionCopySupportedPropertyDictionary(VTSessionRef session, CFDictionaryRef *supportedPropertyDictionaryOut)
{
	if (supportedPropertyDictionaryOut != NULL)
		*supportedPropertyDictionaryOut = NULL;
	return kVTInvalidSessionErr;
}

#pragma mark - Decompression

OSStatus VTDecompressionSessionCreate(CFAllocatorRef allocator, CMVideoFormatDescriptionRef videoFormatDescription,
	CFDictionaryRef videoDecoderSpecification, CFDictionaryRef destinationImageBufferAttributes,
	const VTDecompressionOutputCallbackRecord *outputCallback, VTDecompressionSessionRef *decompressionSessionOut)
{
	if (decompressionSessionOut != NULL)
		*decompressionSessionOut = NULL;
	return kVTCouldNotFindVideoDecoderErr;
}

void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session) { }
CFTypeID VTDecompressionSessionGetTypeID(void) { return 0; }

OSStatus VTDecompressionSessionDecodeFrame(VTDecompressionSessionRef session, CMSampleBufferRef sampleBuffer,
	VTDecodeFrameFlags decodeFlags, void *sourceFrameRefCon, VTDecodeInfoFlags *infoFlagsOut)
{
	if (infoFlagsOut != NULL)
		*infoFlagsOut = 0;
	return kVTInvalidSessionErr;
}

OSStatus VTDecompressionSessionFinishDelayedFrames(VTDecompressionSessionRef session) { return kVTInvalidSessionErr; }

Boolean VTDecompressionSessionCanAcceptFormatDescription(VTDecompressionSessionRef session,
	CMFormatDescriptionRef newFormatDesc)
{
	return false;
}

OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session)
{
	return kVTInvalidSessionErr;
}

OSStatus VTDecompressionSessionCopyBlackPixelBuffer(VTDecompressionSessionRef session, CVPixelBufferRef *pixelBufferOut)
{
	if (pixelBufferOut != NULL)
		*pixelBufferOut = NULL;
	return kVTInvalidSessionErr;
}

Boolean VTIsHardwareDecodeSupported(CMVideoCodecType codecType)
{
	return false;
}

#pragma mark - Compression

OSStatus VTCompressionSessionCreate(CFAllocatorRef allocator, int32_t width, int32_t height, CMVideoCodecType codecType,
	CFDictionaryRef encoderSpecification, CFDictionaryRef sourceImageBufferAttributes,
	CFAllocatorRef compressedDataAllocator, VTCompressionOutputCallback outputCallback, void *outputCallbackRefCon,
	VTCompressionSessionRef *compressionSessionOut)
{
	if (compressionSessionOut != NULL)
		*compressionSessionOut = NULL;
	return kVTCouldNotFindVideoEncoderErr;
}

void VTCompressionSessionInvalidate(VTCompressionSessionRef session) { }
CFTypeID VTCompressionSessionGetTypeID(void) { return 0; }
CVPixelBufferPoolRef VTCompressionSessionGetPixelBufferPool(VTCompressionSessionRef session) { return NULL; }
OSStatus VTCompressionSessionPrepareToEncodeFrames(VTCompressionSessionRef session) { return kVTInvalidSessionErr; }

OSStatus VTCompressionSessionEncodeFrame(VTCompressionSessionRef session, CVImageBufferRef imageBuffer,
	CMTime presentationTimeStamp, CMTime duration, CFDictionaryRef frameProperties, void *sourceFrameRefcon,
	VTEncodeInfoFlags *infoFlagsOut)
{
	if (infoFlagsOut != NULL)
		*infoFlagsOut = 0;
	return kVTInvalidSessionErr;
}

OSStatus VTCompressionSessionCompleteFrames(VTCompressionSessionRef session, CMTime completeUntilPresentationTimeStamp)
{
	return kVTInvalidSessionErr;
}

#pragma mark - Pixel transfer

OSStatus VTPixelTransferSessionCreate(CFAllocatorRef allocator, VTPixelTransferSessionRef *pixelTransferSessionOut)
{
	if (pixelTransferSessionOut != NULL)
		*pixelTransferSessionOut = NULL;
	return kVTPixelTransferNotSupportedErr;
}

void VTPixelTransferSessionInvalidate(VTPixelTransferSessionRef session) { }
CFTypeID VTPixelTransferSessionGetTypeID(void) { return 0; }

OSStatus VTPixelTransferSessionTransferImage(VTPixelTransferSessionRef session, CVPixelBufferRef sourceBuffer,
	CVPixelBufferRef destinationBuffer)
{
	return kVTInvalidSessionErr;
}
