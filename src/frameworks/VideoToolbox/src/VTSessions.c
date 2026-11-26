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

#include <VideoToolbox/VideoToolbox.h>
#include "VTCompressionInternal.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <Block.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern int darling_avcodec_available(void);
extern int darling_avutil_available(void);

#pragma mark - Codec Mapping

static enum AVCodecID map_fourcc_to_avcodecid(CMVideoCodecType codecType)
{
	switch (codecType) {
	case kCMVideoCodecType_H264:
	case 0x64766831: /* 'dvh1' */
	case 0x68323634: /* 'h264' */
		return AV_CODEC_ID_H264;
	case kCMVideoCodecType_HEVC:
	case 0x68657631: /* 'hev1' */
	case 0x64766865: /* 'dvhe' */
		return AV_CODEC_ID_HEVC;
	case kCMVideoCodecType_VP9:
		return AV_CODEC_ID_VP9;
	case 0x61763031: /* 'av01' */
		return AV_CODEC_ID_AV1;
	case kCMVideoCodecType_MPEG4Video:
		return AV_CODEC_ID_MPEG4;
	case kCMVideoCodecType_MPEG2Video:
	case 0x6d706567: /* 'mpeg' */
		return AV_CODEC_ID_MPEG2VIDEO;
	case kCMVideoCodecType_MPEG1Video:
		return AV_CODEC_ID_MPEG1VIDEO;
	case kCMVideoCodecType_H263:
	case 0x73323633: /* 's263' */
		return AV_CODEC_ID_H263;
	case kCMVideoCodecType_AppleProRes422:
	case kCMVideoCodecType_AppleProRes422HQ:
	case kCMVideoCodecType_AppleProRes422LT:
	case kCMVideoCodecType_AppleProRes422Proxy:
	case kCMVideoCodecType_AppleProRes4444:
	case 0x61703478: /* 'ap4x' */
		return AV_CODEC_ID_PRORES;
	case kCMVideoCodecType_JPEG:
	case 0x646d6231: /* 'dmb1' */
		return AV_CODEC_ID_MJPEG;
	default:
		return AV_CODEC_ID_NONE;
	}
}

#pragma mark - Pending Frames Tracking

typedef struct PendingFrame {
	void *sourceFrameRefCon;
	CMTime pts;
	CMTime duration;
	VTDecompressionOutputHandler outputHandler;
	int64_t opaqueId;
	struct PendingFrame *next;
} PendingFrame;

struct OpaqueVTDecompressionSession {
	CFRuntimeBase base;

	CMVideoFormatDescriptionRef formatDesc;
	enum AVCodecID codecId;
	const AVCodec *codec;
	AVCodecContext *avctx;
	bool extradataIsAvcC;

	int32_t width;
	int32_t height;
	OSType outputPixelFormat;
	CFDictionaryRef destinationAttributes;

	VTDecompressionOutputCallbackRecord outputCallbackRecord;

	dispatch_queue_t decodeQueue;
	pthread_mutex_t lock;
	bool invalidated;
	bool isRealTime;

	CFMutableDictionaryRef customProperties;

	PendingFrame *pendingFramesHead;
	PendingFrame *pendingFramesTail;
	int32_t framesInDecoder;
	int64_t nextOpaqueId;
};

static void add_pending_frame(struct OpaqueVTDecompressionSession *session, PendingFrame *pf)
{
	pf->next = NULL;
	if (session->pendingFramesTail) {
		session->pendingFramesTail->next = pf;
		session->pendingFramesTail = pf;
	} else {
		session->pendingFramesHead = pf;
		session->pendingFramesTail = pf;
	}
}

static PendingFrame* pop_pending_frame(struct OpaqueVTDecompressionSession *session, int64_t opaqueId)
{
	PendingFrame *prev = NULL;
	PendingFrame *curr = session->pendingFramesHead;

	if (opaqueId != 0) {
		while (curr) {
			if (curr->opaqueId == opaqueId) {
				if (prev) {
					prev->next = curr->next;
				} else {
					session->pendingFramesHead = curr->next;
				}
				if (session->pendingFramesTail == curr) {
					session->pendingFramesTail = prev;
				}
				curr->next = NULL;
				return curr;
			}
			prev = curr;
			curr = curr->next;
		}
	}

	curr = session->pendingFramesHead;
	if (curr) {
		session->pendingFramesHead = curr->next;
		if (session->pendingFramesTail == curr) {
			session->pendingFramesTail = NULL;
		}
		curr->next = NULL;
		return curr;
	}
	return NULL;
}

static void remove_pending_frame(struct OpaqueVTDecompressionSession *session, PendingFrame *target)
{
	if (!target)
		return;

	PendingFrame *prev = NULL;
	PendingFrame *curr = session->pendingFramesHead;
	while (curr) {
		if (curr == target) {
			if (prev) {
				prev->next = curr->next;
			} else {
				session->pendingFramesHead = curr->next;
			}
			if (session->pendingFramesTail == curr) {
				session->pendingFramesTail = prev;
			}
			curr->next = NULL;
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

#pragma mark - CFRuntime Class

static void VTDecompressionSessionFinalize(CFTypeRef cf)
{
	struct OpaqueVTDecompressionSession *session = (struct OpaqueVTDecompressionSession *)cf;
	session->invalidated = true;

	/*
	 * Every queued decode item holds a reference to the session, so none is outstanding here. The
	 * last reference may be dropped by an item running on the decode queue itself, so the queue
	 * must not be waited on.
	 */
	if (session->decodeQueue) {
		dispatch_release(session->decodeQueue);
		session->decodeQueue = NULL;
	}

	pthread_mutex_lock(&session->lock);
	if (session->avctx) {
		avcodec_free_context(&session->avctx);
		session->avctx = NULL;
	}

	PendingFrame *curr = session->pendingFramesHead;
	while (curr) {
		PendingFrame *next = curr->next;
		if (curr->outputHandler) {
			Block_release(curr->outputHandler);
		}
		free(curr);
		curr = next;
	}
	session->pendingFramesHead = NULL;
	session->pendingFramesTail = NULL;
	pthread_mutex_unlock(&session->lock);

	pthread_mutex_destroy(&session->lock);

	if (session->formatDesc) {
		CFRelease(session->formatDesc);
		session->formatDesc = NULL;
	}
	if (session->destinationAttributes) {
		CFRelease(session->destinationAttributes);
		session->destinationAttributes = NULL;
	}
	if (session->customProperties) {
		CFRelease(session->customProperties);
		session->customProperties = NULL;
	}
}

static const CFRuntimeClass VTDecompressionSessionClass = {
	.version = 0,
	.className = "VTDecompressionSession",
	.init = NULL,
	.copy = NULL,
	.finalize = VTDecompressionSessionFinalize,
	.equal = NULL,
	.hash = NULL,
	.copyFormattingDesc = NULL,
	.copyDebugDesc = NULL,
};

static CFTypeID _VTDecompressionSessionTypeID = _kCFRuntimeNotATypeID;

CFTypeID VTDecompressionSessionGetTypeID(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		_VTDecompressionSessionTypeID = _CFRuntimeRegisterClass(&VTDecompressionSessionClass);
	});
	return _VTDecompressionSessionTypeID;
}

void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session)
{
	if (session == NULL)
		return;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID())
		return;

	pthread_mutex_lock(&session->lock);
	if (session->invalidated) {
		pthread_mutex_unlock(&session->lock);
		return;
	}
	session->invalidated = true;
	pthread_mutex_unlock(&session->lock);

	if (session->decodeQueue) {
		dispatch_sync(session->decodeQueue, ^{
			pthread_mutex_lock(&session->lock);
			if (session->avctx) {
				avcodec_flush_buffers(session->avctx);
			}
			pthread_mutex_unlock(&session->lock);
		});
	}
}

#pragma mark - NALU Formatting

/* True when the buffer is a sequence of NAL units with 4-byte big-endian length prefixes that fills it exactly. */
static bool is_length_prefixed(const uint8_t *src, size_t srcLen)
{
	size_t offset = 0;
	while (offset + 4 <= srcLen) {
		size_t len = ((size_t)src[offset] << 24) | ((size_t)src[offset + 1] << 16) | ((size_t)src[offset + 2] << 8) | src[offset + 3];
		if (len == 0 || len > srcLen - offset - 4)
			return false;
		offset += 4 + len;
	}
	return offset == srcLen;
}

static uint8_t* prepare_packet_data(const uint8_t *src, size_t srcLen, bool isAvcC, size_t *outLen, bool *mustFree)
{
	*mustFree = false;
	*outLen = srcLen;
	if (srcLen < 4)
		return (uint8_t*)src;

	bool isAnnexB = (src[0] == 0 && src[1] == 0 && (src[2] == 1 || (src[2] == 0 && srcLen > 3 && src[3] == 1)));
	/* A length prefix of 256..511 (00 00 01 xx) looks like a start code: length-prefixed data that tiles exactly is not Annex B. */
	if (isAvcC && isAnnexB && is_length_prefixed(src, srcLen))
		isAnnexB = false;

	if (isAvcC) {
		if (isAnnexB) {
			uint8_t *dst = (uint8_t*)malloc(srcLen + 4);
			if (!dst) return (uint8_t*)src;
			memcpy(dst, src, srcLen);
			size_t i = 0;
			while (i < srcLen) {
				size_t start = i;
				size_t scLen = 0;
				if (i + 4 <= srcLen && dst[i] == 0 && dst[i+1] == 0 && dst[i+2] == 0 && dst[i+3] == 1) {
					scLen = 4;
				} else if (i + 3 <= srcLen && dst[i] == 0 && dst[i+1] == 0 && dst[i+2] == 1) {
					scLen = 3;
				}
				if (scLen > 0) {
					size_t nextNalu = srcLen;
					for (size_t j = start + scLen; j + 3 <= srcLen; j++) {
						if (dst[j] == 0 && dst[j+1] == 0 && (dst[j+2] == 1 || (j + 4 <= srcLen && dst[j+2] == 0 && dst[j+3] == 1))) {
							nextNalu = j;
							break;
						}
					}
					uint32_t naluDataLen = (uint32_t)(nextNalu - (start + scLen));
					dst[start] = (uint8_t)(naluDataLen >> 24);
					dst[start+1] = (uint8_t)(naluDataLen >> 16);
					dst[start+2] = (uint8_t)(naluDataLen >> 8);
					dst[start+3] = (uint8_t)(naluDataLen);
					i = nextNalu;
				} else {
					i++;
				}
			}
			*mustFree = true;
			return dst;
		}
		return (uint8_t*)src;
	} else {
		if (!isAnnexB) {
			uint8_t *dst = (uint8_t*)malloc(srcLen + AV_INPUT_BUFFER_PADDING_SIZE);
			if (!dst) return (uint8_t*)src;
			memcpy(dst, src, srcLen);
			memset(dst + srcLen, 0, AV_INPUT_BUFFER_PADDING_SIZE);
			size_t offset = 0;
			while (offset + 4 <= srcLen) {
				uint32_t naluLen = ((uint32_t)dst[offset] << 24) |
				                   ((uint32_t)dst[offset + 1] << 16) |
				                   ((uint32_t)dst[offset + 2] << 8) |
				                   ((uint32_t)dst[offset + 3]);
				if (naluLen == 0 || offset + 4 + naluLen > srcLen)
					break;
				dst[offset] = 0;
				dst[offset + 1] = 0;
				dst[offset + 2] = 0;
				dst[offset + 3] = 1;
				offset += 4 + naluLen;
			}
			*mustFree = true;
			return dst;
		}
	}
	return (uint8_t*)src;
}

static void propagate_format_attachments_to_pixel_buffer(CMVideoFormatDescriptionRef formatDesc, CVPixelBufferRef pixbuf)
{
	if (!formatDesc || !pixbuf) return;
	CFDictionaryRef extensions = CMFormatDescriptionGetExtensions(formatDesc);
	if (!extensions) return;

	CFTypeRef val = CFDictionaryGetValue(extensions, kCVImageBufferColorPrimariesKey);
	if (val) CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferColorPrimariesKey, val, kCVAttachmentMode_ShouldPropagate);

	val = CFDictionaryGetValue(extensions, kCVImageBufferTransferFunctionKey);
	if (val) CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferTransferFunctionKey, val, kCVAttachmentMode_ShouldPropagate);

	val = CFDictionaryGetValue(extensions, kCVImageBufferYCbCrMatrixKey);
	if (val) CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferYCbCrMatrixKey, val, kCVAttachmentMode_ShouldPropagate);
}

static inline uint8_t clamp_u8(int v)
{
	return (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
}

static bool output_format_supported(OSType format)
{
	switch (format) {
	case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
	case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
	case kCVPixelFormatType_420YpCbCr8Planar:
	case kCVPixelFormatType_32BGRA:
	case kCVPixelFormatType_32ARGB:
		return true;
	default:
		return false;
	}
}

// Whether the conversion below can turn a decoded frame of this format into the output format.
static bool frame_convertible(int frameFormat, OSType outputFormat)
{
	switch (outputFormat) {
	case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
	case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
		return frameFormat == AV_PIX_FMT_YUV420P || frameFormat == AV_PIX_FMT_YUVJ420P || frameFormat == AV_PIX_FMT_NV12;
	case kCVPixelFormatType_420YpCbCr8Planar:
		return frameFormat == AV_PIX_FMT_YUV420P || frameFormat == AV_PIX_FMT_YUVJ420P;
	case kCVPixelFormatType_32BGRA:
		return frameFormat == AV_PIX_FMT_YUV420P || frameFormat == AV_PIX_FMT_YUVJ420P || frameFormat == AV_PIX_FMT_BGRA;
	case kCVPixelFormatType_32ARGB:
		return frameFormat == AV_PIX_FMT_YUV420P || frameFormat == AV_PIX_FMT_YUVJ420P || frameFormat == AV_PIX_FMT_ARGB;
	default:
		return false;
	}
}

// Converts a 4:2:0 frame to packed 8-bit RGB. The matrix follows the frame's colour space (BT.709 when it is
// unspecified and the picture is HD), the range its range flag.
static void convert_yuv420_to_rgb32(const AVFrame *frame, uint8_t *dst, size_t dstStride, bool argb)
{
	bool full = frame->format == AV_PIX_FMT_YUVJ420P || frame->color_range == AVCOL_RANGE_JPEG;
	bool bt709 = frame->colorspace == AVCOL_SPC_BT709 ||
		(frame->colorspace == AVCOL_SPC_UNSPECIFIED && frame->height >= 720);
	int yScale = full ? 256 : 298;
	int yOffset = full ? 0 : 16;
	int rv, gu, gv, bu;
	if (bt709) {
		rv = full ? 403 : 459; gu = full ? -48 : -55; gv = full ? -120 : -136; bu = full ? 475 : 541;
	} else {
		rv = full ? 359 : 409; gu = full ? -88 : -100; gv = full ? -183 : -208; bu = full ? 454 : 516;
	}
	size_t rI = argb ? 1 : 2, gI = argb ? 2 : 1, bI = argb ? 3 : 0, aI = argb ? 0 : 3;

	for (int y = 0; y < frame->height; y++) {
		uint8_t *rowDst = dst + (size_t)y * dstStride;
		const uint8_t *rowY = frame->data[0] + (size_t)y * frame->linesize[0];
		const uint8_t *rowU = frame->data[1] + (size_t)(y / 2) * frame->linesize[1];
		const uint8_t *rowV = frame->data[2] + (size_t)(y / 2) * frame->linesize[2];
		for (int x = 0; x < frame->width; x++) {
			int c = yScale * ((int)rowY[x] - yOffset);
			int d = (int)rowU[x / 2] - 128;
			int e = (int)rowV[x / 2] - 128;
			rowDst[4 * x + bI] = clamp_u8((c + bu * d + 128) >> 8);
			rowDst[4 * x + gI] = clamp_u8((c + gu * d + gv * e + 128) >> 8);
			rowDst[4 * x + rI] = clamp_u8((c + rv * e + 128) >> 8);
			rowDst[4 * x + aI] = 255;
		}
	}
}

static OSStatus create_pixel_buffer_from_avframe(
	struct OpaqueVTDecompressionSession *session,
	AVFrame *frame,
	CVPixelBufferRef *pixelBufferOut)
{
	if (!pixelBufferOut)
		return kVTParameterErr;
	*pixelBufferOut = NULL;

	size_t width = frame->width;
	size_t height = frame->height;
	OSType pixelFormat = session->outputPixelFormat;
	if (pixelFormat == 0)
		pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
	if (!frame_convertible(frame->format, pixelFormat))
		return kVTVideoDecoderUnsupportedDataFormatErr;

	CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionaryRef emptyDict = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, emptyDict);
	CFRelease(emptyDict);

	CVPixelBufferRef pixbuf = NULL;
	CVReturn cvret = CVPixelBufferCreate(kCFAllocatorDefault, width, height, pixelFormat, attrs, &pixbuf);
	CFRelease(attrs);

	if (cvret != kCVReturnSuccess || pixbuf == NULL) {
		cvret = CVPixelBufferCreate(kCFAllocatorDefault, width, height, pixelFormat, NULL, &pixbuf);
		if (cvret != kCVReturnSuccess || pixbuf == NULL)
			return kVTAllocationFailedErr;
	}

	cvret = CVPixelBufferLockBaseAddress(pixbuf, 0);
	if (cvret != kCVReturnSuccess) {
		CFRelease(pixbuf);
		return kVTAllocationFailedErr;
	}

	if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange ||
	    pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
		uint8_t *dstY = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 0);
		size_t dstStrideY = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 0);
		uint8_t *dstUV = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 1);
		size_t dstStrideUV = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 1);

		if (frame->format == AV_PIX_FMT_NV12) {
			for (size_t y = 0; y < height; y++) {
				memcpy(dstY + y * dstStrideY, frame->data[0] + y * frame->linesize[0], width);
			}
			size_t uvHeight = (height + 1) / 2;
			for (size_t y = 0; y < uvHeight; y++) {
				memcpy(dstUV + y * dstStrideUV, frame->data[1] + y * frame->linesize[1], width);
			}
		} else {
			const uint8_t *srcY = frame->data[0];
			int srcStrideY = frame->linesize[0];
			const uint8_t *srcU = frame->data[1];
			int srcStrideU = frame->linesize[1];
			const uint8_t *srcV = frame->data[2];
			int srcStrideV = frame->linesize[2];

			for (size_t y = 0; y < height; y++) {
				memcpy(dstY + y * dstStrideY, srcY + y * srcStrideY, width);
			}
			size_t uvHeight = (height + 1) / 2;
			size_t uvWidth = (width + 1) / 2;
			for (size_t y = 0; y < uvHeight; y++) {
				uint8_t *duv = dstUV + y * dstStrideUV;
				const uint8_t *su = srcU + y * srcStrideU;
				const uint8_t *sv = srcV + y * srcStrideV;
				for (size_t x = 0; x < uvWidth; x++) {
					duv[2 * x] = su[x];
					duv[2 * x + 1] = sv[x];
				}
			}
		}
	} else if (pixelFormat == kCVPixelFormatType_420YpCbCr8Planar) {
		for (int p = 0; p < 3; p++) {
			uint8_t *dst = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, p);
			size_t dstStride = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, p);
			const uint8_t *src = frame->data[p];
			int srcStride = frame->linesize[p];
			size_t pWidth = (p == 0) ? width : (width + 1) / 2;
			size_t pHeight = (p == 0) ? height : (height + 1) / 2;
			for (size_t y = 0; y < pHeight; y++) {
				memcpy(dst + y * dstStride, src + y * srcStride, pWidth);
			}
		}
	} else if (pixelFormat == kCVPixelFormatType_32BGRA || pixelFormat == kCVPixelFormatType_32ARGB) {
		uint8_t *dst = (uint8_t*)CVPixelBufferGetBaseAddress(pixbuf);
		size_t dstStride = CVPixelBufferGetBytesPerRow(pixbuf);
		if (frame->format == AV_PIX_FMT_BGRA || frame->format == AV_PIX_FMT_ARGB) {
			for (size_t y = 0; y < height; y++) {
				memcpy(dst + y * dstStride, frame->data[0] + y * frame->linesize[0], width * 4);
			}
		} else {
			convert_yuv420_to_rgb32(frame, dst, dstStride, pixelFormat == kCVPixelFormatType_32ARGB);
		}
	}

	CVPixelBufferUnlockBaseAddress(pixbuf, 0);

	if (session->formatDesc) {
		propagate_format_attachments_to_pixel_buffer(session->formatDesc, pixbuf);
	}

	if (frame->color_primaries == AVCOL_PRI_BT709)
		CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferColorPrimariesKey, kCVImageBufferColorPrimaries_ITU_R_709_2, kCVAttachmentMode_ShouldPropagate);
	else if (frame->color_primaries == AVCOL_PRI_BT2020)
		CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferColorPrimariesKey, kCVImageBufferColorPrimaries_ITU_R_2020, kCVAttachmentMode_ShouldPropagate);

	if (frame->color_trc == AVCOL_TRC_BT709)
		CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferTransferFunctionKey, kCVImageBufferTransferFunction_ITU_R_709_2, kCVAttachmentMode_ShouldPropagate);
	else if (frame->color_trc == AVCOL_TRC_SMPTE2084)
		CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferTransferFunctionKey, kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ, kCVAttachmentMode_ShouldPropagate);

	if (frame->colorspace == AVCOL_SPC_BT709)
		CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_709_2, kCVAttachmentMode_ShouldPropagate);
	else if (frame->colorspace == AVCOL_SPC_BT2020_NCL)
		CVBufferSetAttachment((CVBufferRef)pixbuf, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_2020, kCVAttachmentMode_ShouldPropagate);

	*pixelBufferOut = pixbuf;
	return noErr;
}

#pragma mark - Session Properties

OSStatus VTDecompressionSessionSetProperty(VTDecompressionSessionRef session, CFStringRef propertyKey, CFTypeRef propertyValue)
{
	if (session == NULL || propertyKey == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_RealTime)) {
		if (propertyValue == NULL || CFGetTypeID(propertyValue) != CFBooleanGetTypeID())
			return kVTParameterErr;
		session->isRealTime = CFBooleanGetValue((CFBooleanRef)propertyValue);
		return noErr;
	}
	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder)) {
		return kVTPropertyReadOnlyErr;
	}
	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_ThreadCount)) {
		if (propertyValue == NULL || CFGetTypeID(propertyValue) != CFNumberGetTypeID())
			return kVTParameterErr;
		int count = 0;
		CFNumberGetValue((CFNumberRef)propertyValue, kCFNumberIntType, &count);
		pthread_mutex_lock(&session->lock);
		if (session->avctx) session->avctx->thread_count = count;
		pthread_mutex_unlock(&session->lock);
		return noErr;
	}

	pthread_mutex_lock(&session->lock);
	if (propertyValue != NULL) {
		CFDictionarySetValue(session->customProperties, propertyKey, propertyValue);
	} else {
		CFDictionaryRemoveValue(session->customProperties, propertyKey);
	}
	pthread_mutex_unlock(&session->lock);
	return noErr;
}

OSStatus VTDecompressionSessionSetProperties(VTDecompressionSessionRef session, CFDictionaryRef propertyDictionary)
{
	if (session == NULL || propertyDictionary == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	CFIndex count = CFDictionaryGetCount(propertyDictionary);
	if (count == 0)
		return noErr;

	const void *keys[count];
	const void *values[count];
	CFDictionaryGetKeysAndValues(propertyDictionary, keys, values);

	OSStatus lastErr = noErr;
	for (CFIndex i = 0; i < count; i++) {
		OSStatus err = VTDecompressionSessionSetProperty(session, (CFStringRef)keys[i], (CFTypeRef)values[i]);
		if (err != noErr)
			lastErr = err;
	}
	return lastErr;
}

OSStatus VTDecompressionSessionCopyProperty(VTDecompressionSessionRef session, CFStringRef propertyKey,
	CFAllocatorRef allocator, void *propertyValueOut)
{
	if (propertyValueOut != NULL)
		*(CFTypeRef *)propertyValueOut = NULL;
	if (session == NULL || propertyKey == NULL || propertyValueOut == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_RealTime)) {
		*(CFTypeRef *)propertyValueOut = session->isRealTime ? CFRetain(kCFBooleanTrue) : CFRetain(kCFBooleanFalse);
		return noErr;
	}
	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder)) {
		*(CFTypeRef *)propertyValueOut = CFRetain(kCFBooleanFalse);
		return noErr;
	}
	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_ThreadCount)) {
		pthread_mutex_lock(&session->lock);
		int count = session->avctx ? session->avctx->thread_count : 0;
		pthread_mutex_unlock(&session->lock);
		*(CFTypeRef *)propertyValueOut = CFNumberCreate(allocator, kCFNumberIntType, &count);
		return noErr;
	}
	if (CFEqual(propertyKey, kVTDecompressionPropertyKey_NumberOfFramesBeingDecoded)) {
		pthread_mutex_lock(&session->lock);
		int count = session->framesInDecoder;
		pthread_mutex_unlock(&session->lock);
		*(CFTypeRef *)propertyValueOut = CFNumberCreate(allocator, kCFNumberIntType, &count);
		return noErr;
	}

	pthread_mutex_lock(&session->lock);
	CFTypeRef val = CFDictionaryGetValue(session->customProperties, propertyKey);
	if (val != NULL) {
		*(CFTypeRef *)propertyValueOut = CFRetain(val);
		pthread_mutex_unlock(&session->lock);
		return noErr;
	}
	pthread_mutex_unlock(&session->lock);
	return kVTPropertyNotSupportedErr;
}

OSStatus VTDecompressionSessionCopySupportedPropertyDictionary(VTDecompressionSessionRef session,
	CFDictionaryRef *supportedPropertyDictionaryOut)
{
	if (supportedPropertyDictionaryOut != NULL)
		*supportedPropertyDictionaryOut = NULL;
	if (session == NULL || supportedPropertyDictionaryOut == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	const void *keys[] = {
		kVTDecompressionPropertyKey_RealTime,
		kVTDecompressionPropertyKey_UsingHardwareAcceleratedVideoDecoder,
		kVTDecompressionPropertyKey_ThreadCount,
		kVTDecompressionPropertyKey_NumberOfFramesBeingDecoded,
		kVTDecompressionPropertyKey_MinOutputPresentationTimeStampOfFramesBeingDecoded,
		kVTDecompressionPropertyKey_MaxOutputPresentationTimeStampOfFramesBeingDecoded,
		kVTDecompressionPropertyKey_DeinterlaceMode,
		kVTDecompressionPropertyKey_FieldMode,
	};
	CFDictionaryRef emptyDict = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	const void *values[sizeof(keys)/sizeof(keys[0])];
	for (size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
		values[i] = emptyDict;
	}

	CFDictionaryRef supported = CFDictionaryCreate(kCFAllocatorDefault, keys, values,
		sizeof(keys)/sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFRelease(emptyDict);

	*supportedPropertyDictionaryOut = supported;
	return noErr;
}

OSStatus VTDecompressionSessionCopySerializableProperties(VTDecompressionSessionRef session,
	CFAllocatorRef allocator, CFDictionaryRef *dictionaryOut)
{
	if (dictionaryOut != NULL)
		*dictionaryOut = NULL;
	if (session == NULL || dictionaryOut == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	pthread_mutex_lock(&session->lock);
	CFDictionaryRef copy = CFDictionaryCreateCopy(allocator, session->customProperties);
	pthread_mutex_unlock(&session->lock);

	*dictionaryOut = copy;
	return noErr;
}

OSStatus VTDecompressionSessionGetMinAndMaxOutputPresentationTimeStampOfFramesBeingDecoded(
	VTDecompressionSessionRef session, CMTime *minPresentationTimeStampOut, CMTime *maxPresentationTimeStampOut)
{
	if (session == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	pthread_mutex_lock(&session->lock);
	CMTime minPTS = kCMTimeInvalid;
	CMTime maxPTS = kCMTimeInvalid;

	PendingFrame *curr = session->pendingFramesHead;
	while (curr) {
		if (CMTIME_IS_VALID(curr->pts)) {
			if (CMTIME_IS_INVALID(minPTS) || CMTimeCompare(curr->pts, minPTS) < 0)
				minPTS = curr->pts;
			if (CMTIME_IS_INVALID(maxPTS) || CMTimeCompare(curr->pts, maxPTS) > 0)
				maxPTS = curr->pts;
		}
		curr = curr->next;
	}

	if (minPresentationTimeStampOut)
		*minPresentationTimeStampOut = minPTS;
	if (maxPresentationTimeStampOut)
		*maxPresentationTimeStampOut = maxPTS;
	pthread_mutex_unlock(&session->lock);
	return noErr;
}

OSStatus VTDecompressionSessionGetMinOutputPresentationTimeStampOfFramesBeingDecoded(
	VTDecompressionSessionRef session, CMTime *minPresentationTimeStampOut)
{
	return VTDecompressionSessionGetMinAndMaxOutputPresentationTimeStampOfFramesBeingDecoded(session, minPresentationTimeStampOut, NULL);
}

#pragma mark - VTSession Generic Helpers

OSStatus VTSessionSetProperty(VTSessionRef session, CFStringRef propertyKey, CFTypeRef propertyValue)
{
	if (session == NULL || propertyKey == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) == VTDecompressionSessionGetTypeID())
		return VTDecompressionSessionSetProperty((VTDecompressionSessionRef)session, propertyKey, propertyValue);
	if (CFGetTypeID(session) == VTCompressionSessionGetTypeID())
		return VTCompressionSessionSetPropertyInternal(session, propertyKey, propertyValue);
	return kVTInvalidSessionErr;
}

OSStatus VTSessionSetProperties(VTSessionRef session, CFDictionaryRef propertyDictionary)
{
	if (session == NULL || propertyDictionary == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) == VTDecompressionSessionGetTypeID())
		return VTDecompressionSessionSetProperties((VTDecompressionSessionRef)session, propertyDictionary);
	if (CFGetTypeID(session) == VTCompressionSessionGetTypeID())
		return VTCompressionSessionSetPropertiesInternal(session, propertyDictionary);
	return kVTInvalidSessionErr;
}

OSStatus VTSessionCopyProperty(VTSessionRef session, CFStringRef propertyKey, CFAllocatorRef allocator,
	void *propertyValueOut)
{
	if (propertyValueOut != NULL)
		*(CFTypeRef *)propertyValueOut = NULL;
	if (session == NULL || propertyKey == NULL || propertyValueOut == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) == VTDecompressionSessionGetTypeID())
		return VTDecompressionSessionCopyProperty((VTDecompressionSessionRef)session, propertyKey, allocator, propertyValueOut);
	if (CFGetTypeID(session) == VTCompressionSessionGetTypeID())
		return VTCompressionSessionCopyPropertyInternal(session, propertyKey, allocator, propertyValueOut);
	return kVTInvalidSessionErr;
}

OSStatus VTSessionCopySupportedPropertyDictionary(VTSessionRef session, CFDictionaryRef *supportedPropertyDictionaryOut)
{
	if (supportedPropertyDictionaryOut != NULL)
		*supportedPropertyDictionaryOut = NULL;
	if (session == NULL || supportedPropertyDictionaryOut == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) == VTDecompressionSessionGetTypeID())
		return VTDecompressionSessionCopySupportedPropertyDictionary((VTDecompressionSessionRef)session, supportedPropertyDictionaryOut);
	if (CFGetTypeID(session) == VTCompressionSessionGetTypeID())
		return VTCompressionSessionCopySupportedPropertyDictionaryInternal(session, supportedPropertyDictionaryOut);
	return kVTInvalidSessionErr;
}

#pragma mark - Decompression Session Creation

OSStatus VTDecompressionSessionCreateWithOptions(
	CFAllocatorRef allocator,
	CMVideoFormatDescriptionRef videoFormatDescription,
	CFDictionaryRef videoDecoderSpecification,
	CFDictionaryRef destinationImageBufferAttributes,
	const VTDecompressionOutputCallbackRecord *outputCallback,
	CFDictionaryRef options,
	VTDecompressionSessionRef *decompressionSessionOut)
{
	if (decompressionSessionOut == NULL || videoFormatDescription == NULL)
		return kVTParameterErr;
	*decompressionSessionOut = NULL;

	if (videoDecoderSpecification != NULL) {
		CFTypeRef reqHW = CFDictionaryGetValue(videoDecoderSpecification, kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder);
		if (reqHW != NULL && CFGetTypeID(reqHW) == CFBooleanGetTypeID() && CFBooleanGetValue((CFBooleanRef)reqHW)) {
			return kVTCouldNotFindVideoDecoderErr;
		}
	}

	if (!darling_avcodec_available() || !darling_avutil_available()) {
		return kVTCouldNotFindVideoDecoderErr;
	}

	CMVideoCodecType codecType = CMFormatDescriptionGetMediaSubType(videoFormatDescription);
	enum AVCodecID codecId = map_fourcc_to_avcodecid(codecType);
	if (codecId == AV_CODEC_ID_NONE) {
		return kVTCouldNotFindVideoDecoderErr;
	}

	const AVCodec *codec = avcodec_find_decoder(codecId);
	if (codec == NULL) {
		return kVTCouldNotFindVideoDecoderErr;
	}

	AVCodecContext *avctx = avcodec_alloc_context3(codec);
	if (avctx == NULL) {
		return kVTAllocationFailedErr;
	}

	CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(videoFormatDescription);
	avctx->width = dims.width;
	avctx->height = dims.height;
#ifdef AV_CODEC_FLAG_COPY_OPAQUE
	avctx->flags |= AV_CODEC_FLAG_COPY_OPAQUE;
#endif

	bool extradataIsAvcC = false;
	CFDictionaryRef atoms = (CFDictionaryRef)CMFormatDescriptionGetExtension(videoFormatDescription,
		kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms);
	if (atoms != NULL && CFGetTypeID(atoms) == CFDictionaryGetTypeID()) {
		CFDataRef extraData = NULL;
		if (codecId == AV_CODEC_ID_H264) {
			extraData = (CFDataRef)CFDictionaryGetValue(atoms, CFSTR("avcC"));
			if (extraData) extradataIsAvcC = true;
		} else if (codecId == AV_CODEC_ID_HEVC) {
			extraData = (CFDataRef)CFDictionaryGetValue(atoms, CFSTR("hvcC"));
			if (extraData) extradataIsAvcC = true;
		} else if (codecId == AV_CODEC_ID_VP9) {
			extraData = (CFDataRef)CFDictionaryGetValue(atoms, CFSTR("vpcC"));
		} else if (codecId == AV_CODEC_ID_AV1) {
			extraData = (CFDataRef)CFDictionaryGetValue(atoms, CFSTR("av1C"));
		} else if (codecId == AV_CODEC_ID_MPEG4) {
			extraData = (CFDataRef)CFDictionaryGetValue(atoms, CFSTR("esds"));
		}

		if (extraData != NULL && CFGetTypeID(extraData) == CFDataGetTypeID()) {
			CFIndex length = CFDataGetLength(extraData);
			if (length > 0) {
				avctx->extradata = (uint8_t*)av_malloc(length + AV_INPUT_BUFFER_PADDING_SIZE);
				if (avctx->extradata) {
					memset(avctx->extradata + length, 0, AV_INPUT_BUFFER_PADDING_SIZE);
					memcpy(avctx->extradata, CFDataGetBytePtr(extraData), length);
					avctx->extradata_size = (int)length;
				}
			}
		}
	}

	if (avcodec_open2(avctx, codec, NULL) < 0) {
		avcodec_free_context(&avctx);
		return kVTVideoDecoderUnsupportedDataFormatErr;
	}

	VTDecompressionSessionRef session = (VTDecompressionSessionRef)_CFRuntimeCreateInstance(
		allocator,
		VTDecompressionSessionGetTypeID(),
		sizeof(struct OpaqueVTDecompressionSession) - sizeof(CFRuntimeBase),
		NULL);
	if (session == NULL) {
		avcodec_free_context(&avctx);
		return kVTAllocationFailedErr;
	}

	memset((char*)session + sizeof(CFRuntimeBase), 0, sizeof(struct OpaqueVTDecompressionSession) - sizeof(CFRuntimeBase));

	session->formatDesc = (CMVideoFormatDescriptionRef)CFRetain(videoFormatDescription);
	session->width = dims.width;
	session->height = dims.height;
	session->codecId = codecId;
	session->codec = codec;
	session->avctx = avctx;
	session->extradataIsAvcC = extradataIsAvcC;
	session->isRealTime = true;
	session->decodeQueue = dispatch_queue_create("org.darlinghq.videotoolbox.decode", DISPATCH_QUEUE_SERIAL);
	/*
	 * Recursive: output callbacks run with the lock held and may call back into the session
	 * (for instance to read a property).
	 */
	pthread_mutexattr_t lockAttr;
	pthread_mutexattr_init(&lockAttr);
	pthread_mutexattr_settype(&lockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&session->lock, &lockAttr);
	pthread_mutexattr_destroy(&lockAttr);
	session->customProperties = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	session->nextOpaqueId = 1;

	if (outputCallback != NULL) {
		session->outputCallbackRecord = *outputCallback;
	}

	session->outputPixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
	bool outputFormatRequested = false;
	bool outputFormatFound = false;
	if (destinationImageBufferAttributes != NULL) {
		session->destinationAttributes = (CFDictionaryRef)CFRetain(destinationImageBufferAttributes);
		CFTypeRef fmt = CFDictionaryGetValue(destinationImageBufferAttributes, kCVPixelBufferPixelFormatTypeKey);
		if (fmt != NULL) {
			// A single format or a list in order of preference: the first one that can be produced is used.
			if (CFGetTypeID(fmt) == CFNumberGetTypeID()) {
				OSType candidate = 0;
				outputFormatRequested = true;
				if (CFNumberGetValue((CFNumberRef)fmt, kCFNumberSInt32Type, &candidate) && output_format_supported(candidate)) {
					session->outputPixelFormat = candidate;
					outputFormatFound = true;
				}
			} else if (CFGetTypeID(fmt) == CFArrayGetTypeID() && CFArrayGetCount((CFArrayRef)fmt) > 0) {
				outputFormatRequested = true;
				for (CFIndex i = 0; i < CFArrayGetCount((CFArrayRef)fmt) && !outputFormatFound; i++) {
					CFTypeRef entry = CFArrayGetValueAtIndex((CFArrayRef)fmt, i);
					OSType candidate = 0;
					if (entry && CFGetTypeID(entry) == CFNumberGetTypeID() &&
					    CFNumberGetValue((CFNumberRef)entry, kCFNumberSInt32Type, &candidate) &&
					    output_format_supported(candidate)) {
						session->outputPixelFormat = candidate;
						outputFormatFound = true;
					}
				}
			}
		}
	}
	if (outputFormatRequested && !outputFormatFound) {
		CFRelease(session);
		return kVTPixelTransferNotSupportedErr;
	}

	*decompressionSessionOut = session;
	return noErr;
}

OSStatus VTDecompressionSessionCreate(
	CFAllocatorRef allocator,
	CMVideoFormatDescriptionRef videoFormatDescription,
	CFDictionaryRef videoDecoderSpecification,
	CFDictionaryRef destinationImageBufferAttributes,
	const VTDecompressionOutputCallbackRecord *outputCallback,
	VTDecompressionSessionRef *decompressionSessionOut)
{
	return VTDecompressionSessionCreateWithOptions(
		allocator,
		videoFormatDescription,
		videoDecoderSpecification,
		destinationImageBufferAttributes,
		outputCallback,
		NULL,
		decompressionSessionOut);
}

#pragma mark - Decompression Operations

Boolean VTDecompressionSessionCanAcceptFormatDescription(VTDecompressionSessionRef session, CMFormatDescriptionRef newFormatDesc)
{
	if (session == NULL || newFormatDesc == NULL)
		return false;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return false;

	CMVideoCodecType newCodec = CMFormatDescriptionGetMediaSubType(newFormatDesc);
	CMVideoCodecType curCodec = CMFormatDescriptionGetMediaSubType(session->formatDesc);
	if (newCodec != curCodec)
		return false;

	CMVideoDimensions newDims = CMVideoFormatDescriptionGetDimensions((CMVideoFormatDescriptionRef)newFormatDesc);
	if (newDims.width != session->width || newDims.height != session->height)
		return false;

	return true;
}

OSStatus VTDecompressionSessionCopyBlackPixelBuffer(VTDecompressionSessionRef session, CVPixelBufferRef *pixelBufferOut)
{
	if (pixelBufferOut == NULL)
		return kVTParameterErr;
	*pixelBufferOut = NULL;
	if (session == NULL)
		return kVTInvalidSessionErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	size_t width = session->width;
	size_t height = session->height;
	OSType pixelFormat = session->outputPixelFormat;
	if (pixelFormat == 0)
		pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;

	CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionaryRef emptyDict = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(attrs, kCVPixelBufferIOSurfacePropertiesKey, emptyDict);
	CFRelease(emptyDict);

	CVPixelBufferRef pixbuf = NULL;
	CVReturn cvret = CVPixelBufferCreate(kCFAllocatorDefault, width, height, pixelFormat, attrs, &pixbuf);
	CFRelease(attrs);

	if (cvret != kCVReturnSuccess || pixbuf == NULL) {
		cvret = CVPixelBufferCreate(kCFAllocatorDefault, width, height, pixelFormat, NULL, &pixbuf);
		if (cvret != kCVReturnSuccess || pixbuf == NULL)
			return kVTAllocationFailedErr;
	}

	cvret = CVPixelBufferLockBaseAddress(pixbuf, 0);
	if (cvret != kCVReturnSuccess) {
		CFRelease(pixbuf);
		return kVTAllocationFailedErr;
	}

	if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
		uint8_t *dstY = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 0);
		size_t dstStrideY = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 0);
		for (size_t y = 0; y < height; y++) {
			memset(dstY + y * dstStrideY, 16, width);
		}
		uint8_t *dstUV = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 1);
		size_t dstStrideUV = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 1);
		size_t uvHeight = (height + 1) / 2;
		for (size_t y = 0; y < uvHeight; y++) {
			memset(dstUV + y * dstStrideUV, 0x80, width);
		}
	} else if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
		uint8_t *dstY = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 0);
		size_t dstStrideY = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 0);
		for (size_t y = 0; y < height; y++) {
			memset(dstY + y * dstStrideY, 0, width);
		}
		uint8_t *dstUV = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 1);
		size_t dstStrideUV = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 1);
		size_t uvHeight = (height + 1) / 2;
		for (size_t y = 0; y < uvHeight; y++) {
			memset(dstUV + y * dstStrideUV, 0x80, width);
		}
	} else if (pixelFormat == kCVPixelFormatType_420YpCbCr8Planar) {
		uint8_t *dstY = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 0);
		size_t dstStrideY = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 0);
		for (size_t y = 0; y < height; y++) {
			memset(dstY + y * dstStrideY, 16, width);
		}
		size_t uvHeight = (height + 1) / 2;
		size_t uvWidth = (width + 1) / 2;
		for (int p = 1; p <= 2; p++) {
			uint8_t *dst = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, p);
			size_t dstStride = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, p);
			for (size_t y = 0; y < uvHeight; y++) {
				memset(dst + y * dstStride, 0x80, uvWidth);
			}
		}
	} else {
		uint8_t *dst = (uint8_t*)CVPixelBufferGetBaseAddress(pixbuf);
		size_t totalBytes = CVPixelBufferGetDataSize(pixbuf);
		memset(dst, 0, totalBytes);
	}

	CVPixelBufferUnlockBaseAddress(pixbuf, 0);

	if (session->formatDesc) {
		propagate_format_attachments_to_pixel_buffer(session->formatDesc, pixbuf);
	}

	*pixelBufferOut = pixbuf;
	return noErr;
}

static OSStatus VTDecompressionSessionDrainFrames_locked(struct OpaqueVTDecompressionSession *session)
{
	if (!session->avctx)
		return noErr;

	avcodec_send_packet(session->avctx, NULL);

	AVFrame *frame = av_frame_alloc();
	if (!frame)
		return kVTAllocationFailedErr;

	while (1) {
		int ret = avcodec_receive_frame(session->avctx, frame);
		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN) || ret < 0)
			break;

		PendingFrame *pf = pop_pending_frame(session, (int64_t)(uintptr_t)frame->opaque);
		CVPixelBufferRef pixbuf = NULL;
		OSStatus cvStatus = create_pixel_buffer_from_avframe(session, frame, &pixbuf);

		VTDecodeInfoFlags infoFlags = 0;
		void *refCon = pf ? pf->sourceFrameRefCon : NULL;
		CMTime pts = pf ? pf->pts : kCMTimeInvalid;
		CMTime duration = pf ? pf->duration : kCMTimeInvalid;
		VTDecompressionOutputHandler handler = pf ? pf->outputHandler : NULL;

		if (cvStatus == noErr && pixbuf != NULL) {
			if (handler != NULL) {
				handler(noErr, infoFlags, pixbuf, pts, duration);
			} else if (session->outputCallbackRecord.decompressionOutputCallback != NULL) {
				session->outputCallbackRecord.decompressionOutputCallback(
					session->outputCallbackRecord.decompressionOutputRefCon,
					refCon,
					noErr,
					infoFlags,
					pixbuf,
					pts,
					duration);
			}
			CVPixelBufferRelease(pixbuf);
		}

		if (pf) {
			if (pf->outputHandler) Block_release(pf->outputHandler);
			free(pf);
		}
		av_frame_unref(frame);
	}

	av_frame_free(&frame);
	avcodec_flush_buffers(session->avctx);
	session->framesInDecoder = 0;
	return noErr;
}

OSStatus VTDecompressionSessionFinishDelayedFrames(VTDecompressionSessionRef session)
{
	if (session == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	// Frames queued for asynchronous decoding come first, then the decoder's delayed frames.
	__block OSStatus ret = noErr;
	dispatch_sync(session->decodeQueue, ^{
		pthread_mutex_lock(&session->lock);
		ret = VTDecompressionSessionDrainFrames_locked(session);
		pthread_mutex_unlock(&session->lock);
	});
	return ret;
}

OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session)
{
	if (session == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;

	// Only waits for the queued frames. Frames the decoder holds back stay until
	// VTDecompressionSessionFinishDelayedFrames, and draining here would also reset the decoder's
	// reference frames.
	dispatch_sync(session->decodeQueue, ^{});

	return noErr;
}

typedef struct DecodeWorkItem {
	struct OpaqueVTDecompressionSession *session;
	CMSampleBufferRef sampleBuffer;
	VTDecodeFrameFlags decodeFlags;
	void *sourceFrameRefCon;
	VTDecompressionOutputHandler outputHandler;
} DecodeWorkItem;

static void execute_decode_item(DecodeWorkItem *item)
{
	struct OpaqueVTDecompressionSession *session = item->session;
	CMSampleBufferRef sampleBuffer = item->sampleBuffer;
	VTDecodeFrameFlags decodeFlags = item->decodeFlags;
	void *refCon = item->sourceFrameRefCon;
	VTDecompressionOutputHandler handler = item->outputHandler;

	pthread_mutex_lock(&session->lock);
	if (session->invalidated || !session->avctx) {
		pthread_mutex_unlock(&session->lock);
		if (handler) {
			handler(kVTInvalidSessionErr, 0, NULL, kCMTimeInvalid, kCMTimeInvalid);
			Block_release(handler);
		} else if (session->outputCallbackRecord.decompressionOutputCallback) {
			session->outputCallbackRecord.decompressionOutputCallback(
				session->outputCallbackRecord.decompressionOutputRefCon,
				refCon, kVTInvalidSessionErr, 0, NULL, kCMTimeInvalid, kCMTimeInvalid);
		}
		CFRelease(sampleBuffer);
		CFRelease(session);
		free(item);
		return;
	}

	CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
	size_t dataLength = CMBlockBufferGetDataLength(blockBuffer);

	CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
	CMTime duration = CMSampleBufferGetDuration(sampleBuffer);

	uint8_t *rawBuf = (uint8_t*)malloc(dataLength + AV_INPUT_BUFFER_PADDING_SIZE);
	if (!rawBuf) {
		pthread_mutex_unlock(&session->lock);
		if (handler) {
			handler(kVTAllocationFailedErr, 0, NULL, pts, duration);
			Block_release(handler);
		} else if (session->outputCallbackRecord.decompressionOutputCallback) {
			session->outputCallbackRecord.decompressionOutputCallback(
				session->outputCallbackRecord.decompressionOutputRefCon,
				refCon, kVTAllocationFailedErr, 0, NULL, pts, duration);
		}
		CFRelease(sampleBuffer);
		CFRelease(session);
		free(item);
		return;
	}
	memset(rawBuf + dataLength, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	CMBlockBufferCopyDataBytes(blockBuffer, 0, dataLength, rawBuf);

	size_t packetLen = dataLength;
	bool mustFreePacketData = false;
	uint8_t *packetData = rawBuf;
	if (session->codecId == AV_CODEC_ID_H264 || session->codecId == AV_CODEC_ID_HEVC) {
		packetData = prepare_packet_data(rawBuf, dataLength, session->extradataIsAvcC, &packetLen, &mustFreePacketData);
	}

	AVPacket *pkt = av_packet_alloc();
	if (!pkt) {
		free(rawBuf);
		if (mustFreePacketData) free(packetData);
		pthread_mutex_unlock(&session->lock);
		if (handler) {
			handler(kVTAllocationFailedErr, 0, NULL, pts, duration);
			Block_release(handler);
		} else if (session->outputCallbackRecord.decompressionOutputCallback) {
			session->outputCallbackRecord.decompressionOutputCallback(
				session->outputCallbackRecord.decompressionOutputRefCon,
				refCon, kVTAllocationFailedErr, 0, NULL, pts, duration);
		}
		CFRelease(sampleBuffer);
		CFRelease(session);
		free(item);
		return;
	}

	pkt->data = packetData;
	pkt->size = (int)packetLen;
	if (CMTIME_IS_VALID(pts))
		pkt->pts = pts.value;
	if (CMTIME_IS_VALID(duration))
		pkt->duration = duration.value;

	PendingFrame *pf = (PendingFrame*)calloc(1, sizeof(PendingFrame));
	pf->sourceFrameRefCon = refCon;
	pf->pts = pts;
	pf->duration = duration;
	pf->opaqueId = session->nextOpaqueId++;
	if (handler)
		pf->outputHandler = Block_copy(handler);
	add_pending_frame(session, pf);

	pkt->opaque = (void*)(uintptr_t)pf->opaqueId;
	session->framesInDecoder++;

	int sendRet = avcodec_send_packet(session->avctx, pkt);
	av_packet_free(&pkt);
	free(rawBuf);
	if (mustFreePacketData)
		free(packetData);

	if (sendRet < 0) {
		remove_pending_frame(session, pf);
		session->framesInDecoder--;
		OSStatus errStatus = (sendRet == AVERROR_INVALIDDATA) ? kVTVideoDecoderBadDataErr : kVTVideoDecoderMalfunctionErr;
		if (pf->outputHandler) {
			pf->outputHandler(errStatus, 0, NULL, pts, duration);
			Block_release(pf->outputHandler);
		} else if (session->outputCallbackRecord.decompressionOutputCallback) {
			session->outputCallbackRecord.decompressionOutputCallback(
				session->outputCallbackRecord.decompressionOutputRefCon,
				refCon, errStatus, 0, NULL, pts, duration);
		}
		free(pf);
		pthread_mutex_unlock(&session->lock);
		if (handler) Block_release(handler);
		CFRelease(sampleBuffer);
		CFRelease(session);
		free(item);
		return;
	}

	AVFrame *frame = av_frame_alloc();
	while (1) {
		int recRet = avcodec_receive_frame(session->avctx, frame);
		if (recRet == AVERROR(EAGAIN) || recRet == AVERROR_EOF || recRet < 0)
			break;

		PendingFrame *matchedPF = pop_pending_frame(session, (int64_t)(uintptr_t)frame->opaque);
		session->framesInDecoder--;

		void *frameRefCon = matchedPF ? matchedPF->sourceFrameRefCon : refCon;
		CMTime framePTS = matchedPF ? matchedPF->pts : pts;
		CMTime frameDuration = matchedPF ? matchedPF->duration : duration;
		VTDecompressionOutputHandler frameHandler = matchedPF ? matchedPF->outputHandler : NULL;

		VTDecodeInfoFlags frameInfoFlags = (decodeFlags & kVTDecodeFrame_EnableAsynchronousDecompression) ? kVTDecodeInfo_Asynchronous : 0;

		if (decodeFlags & kVTDecodeFrame_DoNotOutputFrame) {
			frameInfoFlags |= kVTDecodeInfo_FrameDropped;
			if (frameHandler) {
				frameHandler(noErr, frameInfoFlags, NULL, framePTS, frameDuration);
			} else if (session->outputCallbackRecord.decompressionOutputCallback) {
				session->outputCallbackRecord.decompressionOutputCallback(
					session->outputCallbackRecord.decompressionOutputRefCon,
					frameRefCon, noErr, frameInfoFlags, NULL, framePTS, frameDuration);
			}
		} else {
			CVPixelBufferRef pixbuf = NULL;
			OSStatus convErr = create_pixel_buffer_from_avframe(session, frame, &pixbuf);
			if (convErr == noErr && pixbuf != NULL) {
				if (frameHandler) {
					frameHandler(noErr, frameInfoFlags, pixbuf, framePTS, frameDuration);
				} else if (session->outputCallbackRecord.decompressionOutputCallback) {
					session->outputCallbackRecord.decompressionOutputCallback(
						session->outputCallbackRecord.decompressionOutputRefCon,
						frameRefCon, noErr, frameInfoFlags, pixbuf, framePTS, frameDuration);
				}
				CVPixelBufferRelease(pixbuf);
			} else {
				if (frameHandler) {
					frameHandler(convErr != noErr ? convErr : kVTVideoDecoderMalfunctionErr, frameInfoFlags, NULL, framePTS, frameDuration);
				} else if (session->outputCallbackRecord.decompressionOutputCallback) {
					session->outputCallbackRecord.decompressionOutputCallback(
						session->outputCallbackRecord.decompressionOutputRefCon,
						frameRefCon, convErr != noErr ? convErr : kVTVideoDecoderMalfunctionErr, frameInfoFlags, NULL, framePTS, frameDuration);
				}
			}
		}

		if (matchedPF) {
			if (matchedPF->outputHandler) Block_release(matchedPF->outputHandler);
			free(matchedPF);
		}
		av_frame_unref(frame);
	}
	av_frame_free(&frame);

	pthread_mutex_unlock(&session->lock);
	if (handler) Block_release(handler);
	CFRelease(sampleBuffer);
	CFRelease(session);
	free(item);
}

OSStatus VTDecompressionSessionDecodeFrame(
	VTDecompressionSessionRef session,
	CMSampleBufferRef sampleBuffer,
	VTDecodeFrameFlags decodeFlags,
	void *sourceFrameRefCon,
	VTDecodeInfoFlags *infoFlagsOut)
{
	if (infoFlagsOut) *infoFlagsOut = 0;
	if (session == NULL || sampleBuffer == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;
	if (session->outputCallbackRecord.decompressionOutputCallback == NULL)
		return kVTParameterErr;

	CMFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
	if (format != NULL && !VTDecompressionSessionCanAcceptFormatDescription(session, format))
		return kVTFormatDescriptionChangeNotSupportedErr;

	CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
	if (blockBuffer == NULL || CMBlockBufferGetDataLength(blockBuffer) == 0)
		return kVTParameterErr;

	DecodeWorkItem *item = (DecodeWorkItem*)calloc(1, sizeof(DecodeWorkItem));
	if (!item) return kVTAllocationFailedErr;
	item->session = (struct OpaqueVTDecompressionSession *)CFRetain(session);
	item->sampleBuffer = (CMSampleBufferRef)CFRetain(sampleBuffer);
	item->decodeFlags = decodeFlags;
	item->sourceFrameRefCon = sourceFrameRefCon;
	item->outputHandler = NULL;

	if (decodeFlags & kVTDecodeFrame_EnableAsynchronousDecompression) {
		if (infoFlagsOut) *infoFlagsOut |= kVTDecodeInfo_Asynchronous;
		dispatch_async(session->decodeQueue, ^{
			execute_decode_item(item);
		});
		return noErr;
	} else {
		dispatch_sync(session->decodeQueue, ^{
			execute_decode_item(item);
		});
		return noErr;
	}
}

OSStatus VTDecompressionSessionDecodeFrameWithOutputHandler(
	VTDecompressionSessionRef session,
	CMSampleBufferRef sampleBuffer,
	VTDecodeFrameFlags decodeFlags,
	VTDecodeInfoFlags *infoFlagsOut,
	VTDecompressionOutputHandler outputHandler)
{
	if (infoFlagsOut) *infoFlagsOut = 0;
	if (session == NULL || sampleBuffer == NULL || outputHandler == NULL)
		return kVTParameterErr;
	if (CFGetTypeID(session) != VTDecompressionSessionGetTypeID() || session->invalidated)
		return kVTInvalidSessionErr;
	if (session->outputCallbackRecord.decompressionOutputCallback != NULL)
		return kVTParameterErr;

	CMFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
	if (format != NULL && !VTDecompressionSessionCanAcceptFormatDescription(session, format))
		return kVTFormatDescriptionChangeNotSupportedErr;

	CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
	if (blockBuffer == NULL || CMBlockBufferGetDataLength(blockBuffer) == 0)
		return kVTParameterErr;

	DecodeWorkItem *item = (DecodeWorkItem*)calloc(1, sizeof(DecodeWorkItem));
	if (!item) return kVTAllocationFailedErr;
	item->session = (struct OpaqueVTDecompressionSession *)CFRetain(session);
	item->sampleBuffer = (CMSampleBufferRef)CFRetain(sampleBuffer);
	item->decodeFlags = decodeFlags;
	item->sourceFrameRefCon = NULL;
	item->outputHandler = Block_copy(outputHandler);

	if (decodeFlags & kVTDecodeFrame_EnableAsynchronousDecompression) {
		if (infoFlagsOut) *infoFlagsOut |= kVTDecodeInfo_Asynchronous;
		dispatch_async(session->decodeQueue, ^{
			execute_decode_item(item);
		});
		return noErr;
	} else {
		dispatch_sync(session->decodeQueue, ^{
			execute_decode_item(item);
		});
		return noErr;
	}
}

OSStatus VTDecompressionSessionDecodeFrameWithOptions(
	VTDecompressionSessionRef session,
	CMSampleBufferRef sampleBuffer,
	VTDecodeFrameFlags decodeFlags,
	CFDictionaryRef frameOptions,
	void *sourceFrameRefCon,
	VTDecodeInfoFlags *infoFlagsOut)
{
	return VTDecompressionSessionDecodeFrame(session, sampleBuffer, decodeFlags, sourceFrameRefCon, infoFlagsOut);
}

OSStatus VTDecompressionSessionDecodeFrameWithOptionsAndOutputHandler(
	VTDecompressionSessionRef session,
	CMSampleBufferRef sampleBuffer,
	VTDecodeFrameFlags decodeFlags,
	CFDictionaryRef frameOptions,
	VTDecodeInfoFlags *infoFlagsOut,
	VTDecompressionOutputHandler outputHandler)
{
	return VTDecompressionSessionDecodeFrameWithOutputHandler(session, sampleBuffer, decodeFlags, infoFlagsOut, outputHandler);
}

Boolean VTIsHardwareDecodeSupported(CMVideoCodecType codecType)
{
	return false;
}

// Darling has no MV-HEVC decoder (test videotoolbox-stereo-mvhevc-decode.c).
Boolean VTIsStereoMVHEVCDecodeSupported(void)
{
	return false;
}

#pragma mark - Pixel Transfer Stubs

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
