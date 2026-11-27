/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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
 * VTCompressionSession: H.264 and HEVC encoding on top of the host libavcodec (libx264 / libx265),
 * in the same way VTDecompressionSession decodes through it. Frames are converted to planar 4:2:0,
 * encoded on a per-session serial queue, and handed to the client as CMSampleBuffers whose format
 * description carries an avcC / hvcC atom and whose data is length-prefixed (AVCC style).
 */

#include <VideoToolbox/VideoToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <Block.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "VTCompressionInternal.h"

extern int darling_avcodec_available(void);
extern int darling_avutil_available(void);

#define kVTEncoderID_H264 CFSTR("com.apple.videotoolbox.videoencoder.h264")
#define kVTEncoderID_HEVC CFSTR("com.apple.videotoolbox.videoencoder.hevc.vcp")

typedef VTCompressionSessionRef VTCS;

#pragma mark - Types

typedef struct EncItem {
	CVPixelBufferRef pixelBuffer;
	CMTime pts;
	CMTime duration;
	void *refcon;
	bool forceKey;
	VTCompressionOutputHandler handler;
	int64_t seq;
	struct EncItem *next;
} EncItem;

struct OpaqueVTCompressionSession {
	CFRuntimeBase base;

	int32_t width, height;
	int codedWidth, codedHeight;
	CMVideoCodecType codecType;
	enum AVCodecID codecId;
	const AVCodec *codec;

	VTCompressionOutputCallback callback;
	void *callbackRefCon;
	CFAllocatorRef dataAllocator;

	OSType sourceFormat;
	CVPixelBufferPoolRef pool;

	dispatch_queue_t queue;

	pthread_mutex_t lock; /* protects the fields below up to "queue-only" */
	bool invalidated;
	bool propsDirty;
	CFMutableDictionaryRef props;

	/* queue-only state */
	AVCodecContext *avctx;
	AVFrame *frame;
	AVPacket *packet;
	CMVideoFormatDescriptionRef formatDesc;
	EncItem *pendHead, *pendTail;
	CMTime *dtsQueue;
	size_t dtsHead, dtsCount, dtsCap;
	int64_t nextSeq;
	bool fullRange;
};

static int queueKey;

#pragma mark - Codec availability

static const AVCodec *find_encoder(enum AVCodecID id)
{
	if (!darling_avcodec_available() || !darling_avutil_available())
		return NULL;
	if (id == AV_CODEC_ID_H264) {
		const AVCodec *c = avcodec_find_encoder_by_name("libx264");
		return c ? c : avcodec_find_encoder_by_name("libopenh264");
	}
	if (id == AV_CODEC_ID_HEVC)
		return avcodec_find_encoder_by_name("libx265");
	return NULL;
}

static enum AVCodecID codec_id_for(CMVideoCodecType t)
{
	if (t == kCMVideoCodecType_H264) return AV_CODEC_ID_H264;
	if (t == kCMVideoCodecType_HEVC) return AV_CODEC_ID_HEVC;
	return AV_CODEC_ID_NONE;
}

OSStatus VTCopyVideoEncoderList(CFDictionaryRef options, CFArrayRef *listOfVideoEncodersOut)
{
	if (listOfVideoEncodersOut == NULL)
		return kVTParameterErr;
	*listOfVideoEncodersOut = NULL;
	CFMutableArrayRef list = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	if (list == NULL)
		return kVTAllocationFailedErr;

	for (int i = 0; i < 2; i++) {
		CMVideoCodecType type = i == 0 ? kCMVideoCodecType_H264 : kCMVideoCodecType_HEVC;
		if (find_encoder(codec_id_for(type)) == NULL)
			continue;
		int32_t t32 = (int32_t)type;
		int32_t rating = i == 0 ? 200 : 0;
		CFNumberRef typeNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &t32);
		CFNumberRef ratingNum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &rating);
		CFMutableDictionaryRef d = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFStringRef name = i == 0 ? CFSTR("H.264") : CFSTR("HEVC");
		CFDictionarySetValue(d, kVTVideoEncoderList_CodecType, typeNum);
		CFDictionarySetValue(d, CFSTR("CodecName"), name);
		CFDictionarySetValue(d, CFSTR("DisplayName"), name);
		CFDictionarySetValue(d, CFSTR("EncoderID"), i == 0 ? kVTEncoderID_H264 : kVTEncoderID_HEVC);
		CFDictionarySetValue(d, kVTVideoEncoderList_EncoderName, i == 0 ? CFSTR("Apple H.264 (SW)") : CFSTR("Apple HEVC (SW)"));
		CFDictionarySetValue(d, CFSTR("PerformanceRating"), ratingNum);
		if (i == 1) {
			int32_t q = 100;
			CFNumberRef qn = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &q);
			CFDictionarySetValue(d, CFSTR("QualityRating"), qn);
			CFRelease(qn);
		}
		CFArrayAppendValue(list, d);
		CFRelease(d);
		CFRelease(typeNum);
		CFRelease(ratingNum);
	}
	*listOfVideoEncodersOut = list;
	return noErr;
}

#pragma mark - Annex B / NAL helpers

typedef struct Nal {
	const uint8_t *data;
	size_t size;
} Nal;

/* Splits an Annex B buffer into NAL units (start codes stripped). Returns the count found (capped at max). */
static size_t split_annexb(const uint8_t *buf, size_t size, Nal *out, size_t max)
{
	size_t n = 0, i = 0;
	long start = -1;
	while (i + 3 <= size) {
		if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1) {
			if (start >= 0 && n < max) {
				size_t end = i;
				while (end > (size_t)start && buf[end - 1] == 0)
					end--;
				out[n].data = buf + start;
				out[n].size = end - start;
				n++;
			}
			start = (long)(i + 3);
			i += 3;
		} else {
			i++;
		}
	}
	if (start >= 0 && n < max && (size_t)start < size) {
		out[n].data = buf + start;
		out[n].size = size - start;
		n++;
	}
	return n;
}

static size_t count_nals(const uint8_t *buf, size_t size)
{
	size_t n = 0;
	for (size_t i = 0; i + 3 <= size; i++) {
		if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1) {
			n++;
			i += 2;
		}
	}
	return n;
}

static void put16(uint8_t *p, unsigned v) { p[0] = v >> 8; p[1] = v; }

/* Annex B -> 4-byte length prefixed. Returns malloc'd buffer. */
static uint8_t *annexb_to_avcc(const uint8_t *buf, size_t size, size_t *outSize)
{
	size_t cnt = count_nals(buf, size);
	Nal *nals = calloc(cnt + 1, sizeof(Nal));
	if (nals == NULL)
		return NULL;
	cnt = split_annexb(buf, size, nals, cnt);
	size_t total = 0;
	for (size_t i = 0; i < cnt; i++)
		total += 4 + nals[i].size;
	uint8_t *out = malloc(total ? total : 1);
	if (out != NULL) {
		size_t p = 0;
		for (size_t i = 0; i < cnt; i++) {
			out[p] = nals[i].size >> 24; out[p + 1] = nals[i].size >> 16;
			out[p + 2] = nals[i].size >> 8; out[p + 3] = nals[i].size;
			memcpy(out + p + 4, nals[i].data, nals[i].size);
			p += 4 + nals[i].size;
		}
		*outSize = total;
	}
	free(nals);
	return out;
}

/* Builds an avcC record from the SPS/PPS units found in an Annex B buffer. */
static CFDataRef build_avcc(const uint8_t *buf, size_t size)
{
	size_t cnt = count_nals(buf, size);
	if (cnt == 0)
		return NULL;
	Nal *nals = calloc(cnt, sizeof(Nal));
	if (nals == NULL)
		return NULL;
	cnt = split_annexb(buf, size, nals, cnt);
	size_t spsN = 0, ppsN = 0, len = 7;
	const Nal *firstSps = NULL;
	for (size_t i = 0; i < cnt; i++) {
		if (nals[i].size == 0)
			continue;
		int t = nals[i].data[0] & 0x1f;
		if (t == 7 && spsN < 31) { spsN++; len += 2 + nals[i].size; if (!firstSps) firstSps = &nals[i]; }
		else if (t == 8 && ppsN < 255) { ppsN++; len += 2 + nals[i].size; }
	}
	CFDataRef result = NULL;
	if (firstSps != NULL && firstSps->size >= 4 && ppsN > 0) {
		uint8_t *rec = malloc(len);
		if (rec != NULL) {
			size_t p = 0;
			rec[p++] = 1;
			rec[p++] = firstSps->data[1];
			rec[p++] = firstSps->data[2];
			rec[p++] = firstSps->data[3];
			rec[p++] = 0xff;
			rec[p++] = 0xe0 | spsN;
			for (size_t i = 0; i < cnt; i++)
				if (nals[i].size && (nals[i].data[0] & 0x1f) == 7) { put16(rec + p, nals[i].size); p += 2; memcpy(rec + p, nals[i].data, nals[i].size); p += nals[i].size; }
			rec[p++] = ppsN;
			for (size_t i = 0; i < cnt; i++)
				if (nals[i].size && (nals[i].data[0] & 0x1f) == 8) { put16(rec + p, nals[i].size); p += 2; memcpy(rec + p, nals[i].data, nals[i].size); p += nals[i].size; }
			result = CFDataCreate(kCFAllocatorDefault, rec, p);
			free(rec);
		}
	}
	free(nals);
	return result;
}

typedef struct BitReader {
	const uint8_t *d;
	size_t size, pos; /* bit position */
	bool bad;
} BitReader;

static unsigned br_bit(BitReader *r)
{
	if (r->pos >= r->size * 8) { r->bad = true; return 0; }
	unsigned v = (r->d[r->pos >> 3] >> (7 - (r->pos & 7))) & 1;
	r->pos++;
	return v;
}

static uint32_t br_ue(BitReader *r)
{
	int zeros = 0;
	while (!r->bad && br_bit(r) == 0 && zeros < 32)
		zeros++;
	uint32_t v = 0;
	for (int i = 0; i < zeros; i++)
		v = (v << 1) | br_bit(r);
	return (1u << zeros) - 1 + v;
}

/* Removes emulation prevention bytes. */
static uint8_t *unescape_rbsp(const uint8_t *d, size_t n, size_t *outN)
{
	uint8_t *o = malloc(n ? n : 1);
	if (o == NULL)
		return NULL;
	size_t k = 0;
	for (size_t i = 0; i < n; i++) {
		if (i >= 2 && d[i] == 3 && d[i - 1] == 0 && d[i - 2] == 0)
			continue;
		o[k++] = d[i];
	}
	*outN = k;
	return o;
}

static CFDataRef build_hvcc(const uint8_t *buf, size_t size)
{
	size_t cnt = count_nals(buf, size);
	if (cnt == 0)
		return NULL;
	Nal *nals = calloc(cnt, sizeof(Nal));
	if (nals == NULL)
		return NULL;
	cnt = split_annexb(buf, size, nals, cnt);

	static const int types[3] = { 32, 33, 34 };
	size_t counts[3] = { 0, 0, 0 }, len = 23;
	const Nal *sps = NULL;
	for (size_t i = 0; i < cnt; i++) {
		if (nals[i].size < 3)
			continue;
		int t = (nals[i].data[0] >> 1) & 0x3f;
		for (int k = 0; k < 3; k++)
			if (t == types[k] && counts[k] < 65535) {
				counts[k]++;
				len += 2 + nals[i].size + (counts[k] == 1 ? 3 : 0);
				if (t == 33 && !sps) sps = &nals[i];
			}
	}
	CFDataRef result = NULL;
	if (sps != NULL && sps->size > 15 && counts[2] > 0) {
		size_t rn = 0;
		uint8_t *rbsp = unescape_rbsp(sps->data + 2, sps->size - 2, &rn);
		uint8_t *rec = malloc(len);
		if (rbsp != NULL && rec != NULL && rn >= 13) {
			int subLayers = (rbsp[0] >> 1) & 7;
			unsigned chroma = 1, bitDepthY = 8, bitDepthC = 8;
			if (subLayers == 0) {
				BitReader r = { rbsp, rn, 13 * 8, false };
				br_ue(&r);
				chroma = br_ue(&r);
				if (chroma == 3) br_bit(&r);
				br_ue(&r); br_ue(&r);
				if (br_bit(&r)) { br_ue(&r); br_ue(&r); br_ue(&r); br_ue(&r); }
				bitDepthY = br_ue(&r) + 8;
				bitDepthC = br_ue(&r) + 8;
				if (r.bad || chroma > 3 || bitDepthY > 16 || bitDepthC > 16) { chroma = 1; bitDepthY = bitDepthC = 8; }
			}
			size_t p = 0;
			rec[p++] = 1;
			rec[p++] = rbsp[1];
			memcpy(rec + p, rbsp + 2, 4); p += 4;
			memcpy(rec + p, rbsp + 6, 6); p += 6;
			rec[p++] = rbsp[12];
			put16(rec + p, 0xf000); p += 2;
			rec[p++] = 0xfc;
			rec[p++] = 0xfc | chroma;
			rec[p++] = 0xf8 | (bitDepthY - 8);
			rec[p++] = 0xf8 | (bitDepthC - 8);
			put16(rec + p, 0); p += 2;
			rec[p++] = ((subLayers + 1) << 3) | ((rbsp[0] & 1) << 2) | 3;
			int arrays = (counts[0] > 0) + (counts[1] > 0) + (counts[2] > 0);
			rec[p++] = arrays;
			for (int k = 0; k < 3; k++) {
				if (counts[k] == 0)
					continue;
				rec[p++] = 0x80 | types[k];
				put16(rec + p, counts[k]); p += 2;
				for (size_t i = 0; i < cnt; i++)
					if (nals[i].size >= 3 && ((nals[i].data[0] >> 1) & 0x3f) == types[k]) {
						put16(rec + p, nals[i].size); p += 2;
						memcpy(rec + p, nals[i].data, nals[i].size); p += nals[i].size;
					}
			}
			result = CFDataCreate(kCFAllocatorDefault, rec, p);
		}
		free(rbsp);
		free(rec);
	}
	free(nals);
	return result;
}

#pragma mark - Properties

static bool profile_level_valid(CMVideoCodecType codec, CFStringRef s)
{
	char buf[64];
	if (!CFStringGetCString(s, buf, sizeof(buf), kCFStringEncodingUTF8))
		return false;
	if (codec == kCMVideoCodecType_HEVC)
		return !strcmp(buf, "HEVC_Main_AutoLevel") || !strcmp(buf, "HEVC_Main10_AutoLevel");
	const char *rest = NULL;
	if (!strncmp(buf, "H264_Baseline_", 14)) rest = buf + 14;
	else if (!strncmp(buf, "H264_Main_", 10)) rest = buf + 10;
	else if (!strncmp(buf, "H264_High_", 10)) rest = buf + 10;
	if (rest == NULL)
		return false;
	if (!strcmp(rest, "AutoLevel"))
		return true;
	/* <major>_<minor> */
	return rest[0] >= '1' && rest[0] <= '5' && rest[1] == '_' && rest[2] >= '0' && rest[2] <= '9' && rest[3] == 0;
}

typedef enum { PK_BOOL, PK_NUMBER, PK_STRING, PK_ARRAY, PK_DICT, PK_DATA, PK_PROFILE } PropKind;

static const struct { const char *key; PropKind kind; } supportedProps[] = {
	{ "RealTime", PK_BOOL },
	{ "AllowFrameReordering", PK_BOOL },
	{ "AverageBitRate", PK_NUMBER },
	{ "ConstantBitRate", PK_NUMBER },
	{ "SpatialAdaptiveQPLevel", PK_NUMBER },
	{ "MaxKeyFrameInterval", PK_NUMBER },
	{ "MaxKeyFrameIntervalDuration", PK_NUMBER },
	{ "ExpectedFrameRate", PK_NUMBER },
	{ "MaxFrameDelayCount", PK_NUMBER },
	{ "DataRateLimits", PK_ARRAY },
	{ "ProfileLevel", PK_PROFILE },
	{ "ColorPrimaries", PK_STRING },
	{ "TransferFunction", PK_STRING },
	{ "YCbCrMatrix", PK_STRING },
	{ "PixelAspectRatio", PK_DICT },
	{ "MasteringDisplayColorVolume", PK_DATA },
	{ "ContentLightLevelInfo", PK_DATA },
};

static int find_prop(CFStringRef key)
{
	char buf[64];
	if (!CFStringGetCString(key, buf, sizeof(buf), kCFStringEncodingUTF8))
		return -1;
	for (size_t i = 0; i < sizeof(supportedProps) / sizeof(supportedProps[0]); i++)
		if (!strcmp(buf, supportedProps[i].key))
			return (int)i;
	return -1;
}

static bool is_session(VTSessionRef s)
{
	return s != NULL && CFGetTypeID(s) == VTCompressionSessionGetTypeID();
}

OSStatus VTCompressionSessionSetPropertyInternal(VTSessionRef sessionRef, CFStringRef key, CFTypeRef value)
{
	VTCS s = (VTCS)sessionRef;
	if (!is_session(sessionRef) || key == NULL)
		return kVTParameterErr;
	pthread_mutex_lock(&s->lock);
	if (s->invalidated) {
		pthread_mutex_unlock(&s->lock);
		return kVTInvalidSessionErr;
	}
	OSStatus st = noErr;
	if (CFEqual(key, kVTCompressionPropertyKey_EncoderID)) {
		st = kVTPropertyReadOnlyErr;
	} else {
		int idx = find_prop(key);
		if (idx < 0) {
			st = kVTPropertyNotSupportedErr;
		} else {
			CFTypeID t = value ? CFGetTypeID(value) : 0;
			bool ok = false;
			switch (supportedProps[idx].kind) {
			case PK_BOOL: ok = t == CFBooleanGetTypeID(); break;
			case PK_NUMBER: ok = t == CFNumberGetTypeID(); break;
			case PK_STRING: ok = t == CFStringGetTypeID(); break;
			case PK_ARRAY: ok = t == CFArrayGetTypeID(); break;
			case PK_DICT: ok = t == CFDictionaryGetTypeID(); break;
			case PK_DATA: ok = t == CFDataGetTypeID(); break;
			case PK_PROFILE: ok = t == CFStringGetTypeID() && profile_level_valid(s->codecType, value); break;
			}
			if (value == NULL || ok) {
				if (value == NULL)
					CFDictionaryRemoveValue(s->props, key);
				else
					CFDictionarySetValue(s->props, key, value);
				s->propsDirty = true;
			} else {
				st = kVTParameterErr;
			}
		}
	}
	pthread_mutex_unlock(&s->lock);
	return st;
}

OSStatus VTCompressionSessionSetPropertiesInternal(VTSessionRef session, CFDictionaryRef dict)
{
	if (!is_session(session) || dict == NULL)
		return kVTParameterErr;
	CFIndex n = CFDictionaryGetCount(dict);
	const void **keys = malloc((n ? n : 1) * sizeof(void *));
	const void **vals = malloc((n ? n : 1) * sizeof(void *));
	if (keys == NULL || vals == NULL) {
		free(keys); free(vals);
		return kVTAllocationFailedErr;
	}
	CFDictionaryGetKeysAndValues(dict, keys, vals);
	OSStatus first = noErr;
	for (CFIndex i = 0; i < n; i++) {
		OSStatus st = VTCompressionSessionSetPropertyInternal(session, keys[i], vals[i]);
		if (st != noErr && first == noErr)
			first = st;
	}
	free(keys); free(vals);
	return first;
}

OSStatus VTCompressionSessionCopyPropertyInternal(VTSessionRef sessionRef, CFStringRef key, CFAllocatorRef allocator, void *out)
{
	VTCS s = (VTCS)sessionRef;
	CFTypeRef *res = (CFTypeRef *)out;
	*res = NULL;
	if (!is_session(sessionRef) || key == NULL)
		return kVTParameterErr;
	pthread_mutex_lock(&s->lock);
	if (s->invalidated) {
		pthread_mutex_unlock(&s->lock);
		return kVTInvalidSessionErr;
	}
	OSStatus st = noErr;
	if (CFEqual(key, kVTCompressionPropertyKey_EncoderID)) {
		*res = CFRetain(s->codecType == kCMVideoCodecType_HEVC ? kVTEncoderID_HEVC : kVTEncoderID_H264);
	} else {
		int idx = find_prop(key);
		if (idx < 0) {
			st = kVTPropertyNotSupportedErr;
		} else {
			CFTypeRef v = CFDictionaryGetValue(s->props, key);
			if (v != NULL) {
				*res = CFRetain(v);
			} else if (supportedProps[idx].kind == PK_BOOL && !strcmp(supportedProps[idx].key, "AllowFrameReordering")) {
				*res = CFRetain(kCFBooleanTrue);
			} else if (!strcmp(supportedProps[idx].key, "ExpectedFrameRate")) {
				double d = 30.0;
				*res = CFNumberCreate(allocator, kCFNumberFloat64Type, &d);
			} else if (!strcmp(supportedProps[idx].key, "AverageBitRate") || !strcmp(supportedProps[idx].key, "MaxKeyFrameInterval")) {
				int64_t z = 0;
				*res = CFNumberCreate(allocator, kCFNumberSInt64Type, &z);
			}
		}
	}
	pthread_mutex_unlock(&s->lock);
	return st;
}

OSStatus VTCompressionSessionCopySupportedPropertyDictionaryInternal(VTSessionRef sessionRef, CFDictionaryRef *out)
{
	VTCS s = (VTCS)sessionRef;
	*out = NULL;
	if (!is_session(sessionRef))
		return kVTParameterErr;
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	for (size_t i = 0; i < sizeof(supportedProps) / sizeof(supportedProps[0]); i++) {
		CFMutableDictionaryRef e = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (supportedProps[i].kind == PK_PROFILE) {
			static const char *const h264[] = { "Baseline", "Main", "High" };
			static const char *const levels[] = { "3_0", "3_1", "3_2", "4_0", "4_1", "4_2", "5_0", "5_1", "5_2" };
			CFMutableArrayRef list = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
			if (s->codecType == kCMVideoCodecType_HEVC) {
				CFArrayAppendValue(list, CFSTR("HEVC_Main_AutoLevel"));
				CFArrayAppendValue(list, CFSTR("HEVC_Main10_AutoLevel"));
			} else {
				for (int p = 0; p < 3; p++) {
					CFStringRef n = CFStringCreateWithFormat(NULL, NULL, CFSTR("H264_%s_AutoLevel"), h264[p]);
					CFArrayAppendValue(list, n); CFRelease(n);
					for (size_t l = 0; l < sizeof(levels) / sizeof(levels[0]); l++) {
						n = CFStringCreateWithFormat(NULL, NULL, CFSTR("H264_%s_%s"), h264[p], levels[l]);
						CFArrayAppendValue(list, n); CFRelease(n);
					}
				}
			}
			CFDictionarySetValue(e, CFSTR("SupportedValueList"), list);
			CFRelease(list);
		}
		CFStringRef k = CFStringCreateWithCString(NULL, supportedProps[i].key, kCFStringEncodingUTF8);
		CFDictionarySetValue(d, k, e);
		CFRelease(k);
		CFRelease(e);
	}
	CFDictionarySetValue(d, kVTCompressionPropertyKey_EncoderID, CFDictionaryCreate(NULL, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
	*out = d;
	return noErr;
}

#pragma mark - Session object

static void free_item(EncItem *it)
{
	if (it->pixelBuffer) CFRelease(it->pixelBuffer);
	if (it->handler) Block_release(it->handler);
	free(it);
}

static void close_encoder(VTCS s)
{
	if (s->avctx) avcodec_free_context(&s->avctx);
	if (s->frame) av_frame_free(&s->frame);
	if (s->packet) av_packet_free(&s->packet);
	EncItem *it = s->pendHead;
	while (it) { EncItem *n = it->next; free_item(it); it = n; }
	s->pendHead = s->pendTail = NULL;
	s->dtsHead = s->dtsCount = 0;
}

static void VTCompressionSessionFinalize(CFTypeRef cf)
{
	VTCS s = (VTCS)cf;
	if (s->queue)
		dispatch_release(s->queue);
	close_encoder(s);
	free(s->dtsQueue);
	if (s->formatDesc) CFRelease(s->formatDesc);
	if (s->pool) CVPixelBufferPoolRelease(s->pool);
	if (s->props) CFRelease(s->props);
	if (s->dataAllocator) CFRelease(s->dataAllocator);
	pthread_mutex_destroy(&s->lock);
}

static const CFRuntimeClass VTCompressionSessionClass = {
	.version = 0,
	.className = "VTCompressionSession",
	.finalize = VTCompressionSessionFinalize,
};

CFTypeID VTCompressionSessionGetTypeID(void)
{
	static CFTypeID id;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		id = _CFRuntimeRegisterClass(&VTCompressionSessionClass);
	});
	return id;
}

static OSType pick_source_format(CFDictionaryRef attrs)
{
	OSType fmt = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
	if (attrs == NULL)
		return fmt;
	CFTypeRef v = CFDictionaryGetValue(attrs, kCVPixelBufferPixelFormatTypeKey);
	if (v != NULL && CFGetTypeID(v) == CFArrayGetTypeID() && CFArrayGetCount(v) > 0)
		v = CFArrayGetValueAtIndex(v, 0);
	int32_t n = 0;
	if (v != NULL && CFGetTypeID(v) == CFNumberGetTypeID() && CFNumberGetValue(v, kCFNumberSInt32Type, &n))
		fmt = (OSType)n;
	return fmt;
}

static CVPixelBufferPoolRef make_pool(int32_t w, int32_t h, OSType fmt, CFDictionaryRef srcAttrs)
{
	CFMutableDictionaryRef a = srcAttrs ? CFDictionaryCreateMutableCopy(NULL, 0, srcAttrs)
		: CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	int32_t zero = 0;
	CFNumberRef nw = CFNumberCreate(NULL, kCFNumberSInt32Type, &w);
	CFNumberRef nh = CFNumberCreate(NULL, kCFNumberSInt32Type, &h);
	CFNumberRef nf = CFNumberCreate(NULL, kCFNumberSInt32Type, &fmt);
	CFNumberRef nz = CFNumberCreate(NULL, kCFNumberSInt32Type, &zero);
	CFDictionarySetValue(a, kCVPixelBufferWidthKey, nw);
	CFDictionarySetValue(a, kCVPixelBufferHeightKey, nh);
	CFDictionarySetValue(a, kCVPixelBufferPixelFormatTypeKey, nf);
	if (srcAttrs == NULL) {
		CFDictionarySetValue(a, CFSTR("ExtendedPixelsRight"), nz);
		CFDictionarySetValue(a, CFSTR("ExtendedPixelsBottom"), nz);
	}
	if (CFDictionaryGetValue(a, kCVPixelBufferIOSurfacePropertiesKey) == NULL) {
		CFMutableDictionaryRef io = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(io, CFSTR("IOSurfacePurgeWhenNotInUse"), kCFBooleanTrue);
		CFDictionarySetValue(a, kCVPixelBufferIOSurfacePropertiesKey, io);
		CFRelease(io);
	}
	CFRelease(nw); CFRelease(nh); CFRelease(nf); CFRelease(nz);
	CVPixelBufferPoolRef pool = NULL;
	if (CVPixelBufferPoolCreate(kCFAllocatorDefault, NULL, a, &pool) != kCVReturnSuccess)
		pool = NULL;
	CFRelease(a);
	return pool;
}

OSStatus VTCompressionSessionCreate(CFAllocatorRef allocator, int32_t width, int32_t height, CMVideoCodecType codecType,
	CFDictionaryRef encoderSpecification, CFDictionaryRef sourceImageBufferAttributes,
	CFAllocatorRef compressedDataAllocator, VTCompressionOutputCallback outputCallback, void *outputCallbackRefCon,
	VTCompressionSessionRef *compressionSessionOut)
{
	if (compressionSessionOut == NULL)
		return kVTParameterErr;
	*compressionSessionOut = NULL;
	if (width <= 0 || height <= 0)
		return kVTParameterErr;

	enum AVCodecID id = codec_id_for(codecType);
	const AVCodec *codec = find_encoder(id);
	if (codec == NULL)
		return kVTCouldNotFindVideoEncoderErr;

	if (encoderSpecification != NULL) {
		/* No hardware encoder exists here. */
		CFTypeRef req = CFDictionaryGetValue(encoderSpecification, kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder);
		if (req != NULL && CFGetTypeID(req) == CFBooleanGetTypeID() && CFBooleanGetValue(req))
			return kVTCouldNotFindVideoEncoderErr;
		CFTypeRef eid = CFDictionaryGetValue(encoderSpecification, kVTCompressionPropertyKey_EncoderID);
		if (eid != NULL && !CFEqual(eid, codecType == kCMVideoCodecType_HEVC ? kVTEncoderID_HEVC : kVTEncoderID_H264))
			return kVTCouldNotFindVideoEncoderErr;
	}

	VTCS s = (VTCS)_CFRuntimeCreateInstance(allocator, VTCompressionSessionGetTypeID(),
		sizeof(struct OpaqueVTCompressionSession) - sizeof(CFRuntimeBase), NULL);
	if (s == NULL)
		return kVTAllocationFailedErr;
	memset((char *)s + sizeof(CFRuntimeBase), 0, sizeof(struct OpaqueVTCompressionSession) - sizeof(CFRuntimeBase));

	s->width = width;
	s->height = height;
	s->codedWidth = (width + 1) & ~1;
	s->codedHeight = (height + 1) & ~1;
	if (s->codedWidth < 16) s->codedWidth = 16;
	if (s->codedHeight < 16) s->codedHeight = 16;
	s->codecType = codecType;
	s->codecId = id;
	s->codec = codec;
	s->callback = outputCallback;
	s->callbackRefCon = outputCallbackRefCon;
	if (compressedDataAllocator != NULL)
		s->dataAllocator = CFRetain(compressedDataAllocator);
	s->sourceFormat = pick_source_format(sourceImageBufferAttributes);
	s->fullRange = s->sourceFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
	s->props = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	pthread_mutex_init(&s->lock, NULL);
	s->queue = dispatch_queue_create("org.darlinghq.videotoolbox.encode", DISPATCH_QUEUE_SERIAL);
	dispatch_queue_set_specific(s->queue, &queueKey, s, NULL);
	s->pool = make_pool(width, height, s->sourceFormat, sourceImageBufferAttributes);

	*compressionSessionOut = s;
	return noErr;
}

CVPixelBufferPoolRef VTCompressionSessionGetPixelBufferPool(VTCompressionSessionRef session)
{
	if (!is_session(session))
		return NULL;
	pthread_mutex_lock(&session->lock);
	CVPixelBufferPoolRef p = session->invalidated ? NULL : session->pool;
	pthread_mutex_unlock(&session->lock);
	return p;
}

#pragma mark - Encoder (queue only)

static bool get_prop_number(VTCS s, const char *key, double *out)
{
	CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
	CFTypeRef v = CFDictionaryGetValue(s->props, k);
	CFRelease(k);
	return v != NULL && CFNumberGetValue(v, kCFNumberFloat64Type, out);
}

static bool get_prop_bool(VTCS s, const char *key, bool def)
{
	CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
	CFTypeRef v = CFDictionaryGetValue(s->props, k);
	CFRelease(k);
	return v != NULL ? CFBooleanGetValue(v) : def;
}

static OSStatus open_encoder(VTCS s)
{
	pthread_mutex_lock(&s->lock);
	double fps = 30, bitrate = 0, keyint = 0, keydur = 0;
	get_prop_number(s, "ExpectedFrameRate", &fps);
	get_prop_number(s, "AverageBitRate", &bitrate);
	get_prop_number(s, "MaxKeyFrameInterval", &keyint);
	get_prop_number(s, "MaxKeyFrameIntervalDuration", &keydur);
	bool realtime = get_prop_bool(s, "RealTime", false);
	bool reorder = get_prop_bool(s, "AllowFrameReordering", true);
	char profile[80] = "";
	CFTypeRef pl = CFDictionaryGetValue(s->props, kVTCompressionPropertyKey_ProfileLevel);
	if (pl != NULL)
		CFStringGetCString(pl, profile, sizeof(profile), kCFStringEncodingUTF8);
	s->propsDirty = false;
	pthread_mutex_unlock(&s->lock);

	if (fps < 1 || fps > 1000)
		fps = 30;
	AVCodecContext *c = avcodec_alloc_context3(s->codec);
	if (c == NULL)
		return kVTAllocationFailedErr;
	c->width = s->codedWidth;
	c->height = s->codedHeight;
	c->pix_fmt = AV_PIX_FMT_YUV420P;
	c->time_base = (AVRational){ 1, (int)(fps + 0.5) };
	c->framerate = (AVRational){ (int)(fps + 0.5), 1 };
	c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	c->log_level_offset = 8;
	if (s->fullRange)
		c->color_range = AVCOL_RANGE_JPEG;
	if (bitrate > 0)
		c->bit_rate = (int64_t)bitrate;
	int gop = keyint > 0 ? (int)keyint : (keydur > 0 ? (int)(keydur * fps + 0.5) : 250);
	c->gop_size = gop < 1 ? 1 : gop;
	c->max_b_frames = (reorder && !realtime) ? 2 : 0;

	const char *prof = NULL;
	if (strstr(profile, "Baseline")) { prof = "baseline"; c->max_b_frames = 0; }
	else if (strstr(profile, "Main10")) prof = "main10";
	else if (strstr(profile, "Main")) prof = "main";
	else if (strstr(profile, "High")) prof = "high";
	if (s->codecId == AV_CODEC_ID_H264) {
		if (prof) av_opt_set(c->priv_data, "profile", prof, 0);
		av_opt_set(c->priv_data, "preset", realtime ? "ultrafast" : "veryfast", 0);
		if (realtime) av_opt_set(c->priv_data, "tune", "zerolatency", 0);
		av_opt_set(c->priv_data, "forced-idr", "1", 0);
		/* Explicit levels: "H264_<profile>_<major>_<minor>" */
		int major, minor;
		const char *lv = strrchr(profile, '_');
		if (lv && lv > profile && lv[-1] >= '0' && lv[-1] <= '9' && sscanf(lv - 2, "%d_%d", &major, &minor) == 2)
			c->level = major * 10 + minor;
	} else {
		av_opt_set(c->priv_data, "preset", realtime ? "ultrafast" : "veryfast", 0);
		if (realtime) av_opt_set(c->priv_data, "tune", "zerolatency", 0);
		av_opt_set(c->priv_data, "forced-idr", "1", 0);
		av_opt_set(c->priv_data, "x265-params", "log-level=error", 0);
	}

	if (avcodec_open2(c, s->codec, NULL) < 0) {
		avcodec_free_context(&c);
		return kVTVideoEncoderMalfunctionErr;
	}
	s->avctx = c;
	s->frame = av_frame_alloc();
	s->packet = av_packet_alloc();
	if (s->frame == NULL || s->packet == NULL) {
		close_encoder(s);
		return kVTAllocationFailedErr;
	}
	s->frame->format = AV_PIX_FMT_YUV420P;
	s->frame->width = c->width;
	s->frame->height = c->height;
	if (av_frame_get_buffer(s->frame, 0) < 0) {
		close_encoder(s);
		return kVTAllocationFailedErr;
	}
	return noErr;
}

static OSStatus make_format_description(VTCS s, const uint8_t *annexb, size_t size)
{
	CFDataRef record = s->codecId == AV_CODEC_ID_HEVC ? build_hvcc(annexb, size) : build_avcc(annexb, size);
	if (record == NULL)
		return kVTVideoEncoderMalfunctionErr;

	CFMutableDictionaryRef ext = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(atoms, s->codecId == AV_CODEC_ID_HEVC ? CFSTR("hvcC") : CFSTR("avcC"), record);
	CFDictionarySetValue(ext, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms, atoms);
	int32_t one = 1, zero = 0, depth = 24, q = 512, eight = 8;
	CFNumberRef n1 = CFNumberCreate(NULL, kCFNumberSInt32Type, &one);
	CFNumberRef n0 = CFNumberCreate(NULL, kCFNumberSInt32Type, &zero);
	CFNumberRef nd = CFNumberCreate(NULL, kCFNumberSInt32Type, &depth);
	CFNumberRef nq = CFNumberCreate(NULL, kCFNumberSInt32Type, &q);
	CFDictionarySetValue(ext, CFSTR("CVFieldCount"), n1);
	CFDictionarySetValue(ext, CFSTR("CVImageBufferChromaLocationBottomField"), CFSTR("Left"));
	CFDictionarySetValue(ext, CFSTR("CVImageBufferChromaLocationTopField"), CFSTR("Left"));
	CFDictionarySetValue(ext, kCMFormatDescriptionExtension_Depth, nd);
	CFDictionarySetValue(ext, kCMFormatDescriptionExtension_FormatName, s->codecId == AV_CODEC_ID_HEVC ? CFSTR("HEVC") : CFSTR("H.264"));
	CFDictionarySetValue(ext, kCMFormatDescriptionExtension_FullRangeVideo, s->fullRange ? kCFBooleanTrue : kCFBooleanFalse);
	CFDictionarySetValue(ext, CFSTR("RevisionLevel"), n0);
	CFDictionarySetValue(ext, CFSTR("SpatialQuality"), nq);
	CFDictionarySetValue(ext, CFSTR("TemporalQuality"), nq);
	CFDictionarySetValue(ext, CFSTR("Version"), n0);
	if (s->codecId == AV_CODEC_ID_HEVC) {
		CFNumberRef n8 = CFNumberCreate(NULL, kCFNumberSInt32Type, &eight);
		CFDictionarySetValue(ext, CFSTR("BitsPerComponent"), n8);
		CFRelease(n8);
	} else {
		int32_t one32 = 1;
		CFNumberRef sp = CFNumberCreate(NULL, kCFNumberSInt32Type, &one32);
		const void *k[] = { CFSTR("HorizontalSpacing"), CFSTR("VerticalSpacing") };
		const void *v[] = { sp, sp };
		CFDictionaryRef par = CFDictionaryCreate(NULL, k, v, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(ext, CFSTR("CVPixelAspectRatio"), par);
		CFRelease(par);
		CFRelease(sp);
	}
	CMVideoFormatDescriptionRef fd = NULL;
	OSStatus st = CMVideoFormatDescriptionCreate(NULL, s->codecType, s->width, s->height, ext, &fd);
	CFRelease(n1); CFRelease(n0); CFRelease(nd); CFRelease(nq);
	CFRelease(atoms); CFRelease(ext); CFRelease(record);
	if (st != noErr)
		return kVTAllocationFailedErr;
	if (s->formatDesc) CFRelease(s->formatDesc);
	s->formatDesc = fd;
	return noErr;
}

static void deliver(VTCS s, EncItem *it, OSStatus status, CMSampleBufferRef sb)
{
	VTEncodeInfoFlags flags = 0;
	if (it && it->handler)
		it->handler(status, flags, sb);
	else if (s->callback)
		s->callback(s->callbackRefCon, it ? it->refcon : NULL, status, flags, sb);
}

static EncItem *take_pending(VTCS s, int64_t seq)
{
	EncItem *prev = NULL;
	for (EncItem *it = s->pendHead; it; prev = it, it = it->next) {
		if (it->seq == seq) {
			if (prev) prev->next = it->next; else s->pendHead = it->next;
			if (s->pendTail == it) s->pendTail = prev;
			it->next = NULL;
			return it;
		}
	}
	return NULL;
}

static void emit_packet(VTCS s, AVPacket *pkt)
{
	EncItem *it = take_pending(s, pkt->pts);
	if (it == NULL)
		return;

	OSStatus st = noErr;
	if (s->formatDesc == NULL) {
		const uint8_t *hdr = s->avctx->extradata;
		size_t hl = s->avctx->extradata_size;
		st = hdr ? make_format_description(s, hdr, hl) : kVTVideoEncoderMalfunctionErr;
		if (st != noErr)
			st = make_format_description(s, pkt->data, pkt->size);
	}
	CMTime dts = it->pts;
	if (s->dtsCount > 0) {
		dts = s->dtsQueue[s->dtsHead++];
		s->dtsCount--;
	}
	bool key = (pkt->flags & AV_PKT_FLAG_KEY) != 0;
	CMSampleBufferRef sb = NULL;
	if (st == noErr) {
		size_t sz = 0;
		uint8_t *data = annexb_to_avcc(pkt->data, pkt->size, &sz);
		CMBlockBufferRef bb = NULL;
		if (data == NULL) {
			st = kVTAllocationFailedErr;
		} else {
			CFAllocatorRef alloc = s->dataAllocator ? s->dataAllocator : kCFAllocatorDefault;
			void *mem = CFAllocatorAllocate(alloc, sz, 0);
			if (mem == NULL) {
				st = kVTAllocationFailedErr;
			} else {
				memcpy(mem, data, sz);
				/* The block buffer owns the memory and frees it through the same allocator. */
				st = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, mem, sz, alloc, NULL, 0, sz, 0, &bb);
				if (st != noErr)
					CFAllocatorDeallocate(alloc, mem);
			}
			if (st == noErr) {
				CMSampleTimingInfo timing = { it->duration, it->pts, dts };
				st = CMSampleBufferCreate(kCFAllocatorDefault, bb, true, NULL, NULL, s->formatDesc, 1, 1, &timing, 1, &sz, &sb);
			}
			if (bb) CFRelease(bb);
			free(data);
		}
	}
	if (st == noErr && sb != NULL) {
		CFArrayRef arr = CMSampleBufferGetSampleAttachmentsArray(sb, true);
		CFMutableDictionaryRef a = arr && CFArrayGetCount(arr) ? (CFMutableDictionaryRef)CFArrayGetValueAtIndex(arr, 0) : NULL;
		if (a) {
			bool nonRef = s->codecId == AV_CODEC_ID_H264 && pkt->size > 4;
			if (nonRef) {
				/* NAL header of the first unit after its start code: nal_ref_idc == 0 means nothing refers to it. */
				const uint8_t *p = pkt->data;
				size_t i = 0;
				while (i + 3 < (size_t)pkt->size && !(p[i] == 0 && p[i + 1] == 0 && p[i + 2] == 1)) i++;
				nonRef = i + 3 < (size_t)pkt->size && ((p[i + 3] >> 5) & 3) == 0 && (p[i + 3] & 0x1f) == 1;
			}
			CFDictionarySetValue(a, CFSTR("DependsOnOthers"), key ? kCFBooleanFalse : kCFBooleanTrue);
			CFDictionarySetValue(a, CFSTR("EarlierDisplayTimesAllowed"), CMTimeCompare(it->pts, dts) > 0 ? kCFBooleanTrue : kCFBooleanFalse);
			if (!key)
				CFDictionarySetValue(a, kCMSampleAttachmentKey_NotSync, kCFBooleanTrue);
			else if (s->codecId == AV_CODEC_ID_HEVC)
				CFDictionarySetValue(a, kCMSampleAttachmentKey_NotSync, kCFBooleanFalse);
			if (nonRef && CMTimeCompare(it->pts, dts) < 0)
				CFDictionarySetValue(a, kCMSampleAttachmentKey_IsDependedOnByOthers, kCFBooleanFalse);
		}
	}
	pthread_mutex_lock(&s->lock);
	bool inv = s->invalidated;
	pthread_mutex_unlock(&s->lock);
	if (!inv)
		deliver(s, it, st, st == noErr ? sb : NULL);
	if (sb) CFRelease(sb);
	free_item(it);
}

static void receive_all(VTCS s)
{
	for (;;) {
		int r = avcodec_receive_packet(s->avctx, s->packet);
		if (r < 0)
			break;
		emit_packet(s, s->packet);
		av_packet_unref(s->packet);
	}
}

static void fail_pending(VTCS s, OSStatus st)
{
	EncItem *it = s->pendHead;
	s->pendHead = s->pendTail = NULL;
	s->dtsCount = s->dtsHead = 0;
	while (it) {
		EncItem *n = it->next;
		deliver(s, it, st, NULL);
		free_item(it);
		it = n;
	}
}

static inline uint8_t clamp8(int v) { return v < 0 ? 0 : v > 255 ? 255 : v; }

/* Copies/converts/scales the source buffer into the encoder's planar frame, replicating edges to the coded size. */
static bool fill_frame(VTCS s, CVPixelBufferRef pb)
{
	AVFrame *f = s->frame;
	if (av_frame_make_writable(f) < 0)
		return false;
	OSType fmt = CVPixelBufferGetPixelFormatType(pb);
	size_t sw = CVPixelBufferGetWidth(pb), sh = CVPixelBufferGetHeight(pb);
	if (sw == 0 || sh == 0)
		return false;
	int cw = f->width, ch = f->height;
	CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly);

	const uint8_t *sy = NULL, *suv = NULL, *su = NULL, *sv = NULL, *sp = NULL;
	size_t ysr = 0, uvsr = 0, usr = 0, vsr = 0;
	bool bgra = false, argb = false;
	if (fmt == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange || fmt == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
		sy = CVPixelBufferGetBaseAddressOfPlane(pb, 0); ysr = CVPixelBufferGetBytesPerRowOfPlane(pb, 0);
		suv = CVPixelBufferGetBaseAddressOfPlane(pb, 1); uvsr = CVPixelBufferGetBytesPerRowOfPlane(pb, 1);
	} else if (fmt == kCVPixelFormatType_420YpCbCr8Planar) {
		sy = CVPixelBufferGetBaseAddressOfPlane(pb, 0); ysr = CVPixelBufferGetBytesPerRowOfPlane(pb, 0);
		su = CVPixelBufferGetBaseAddressOfPlane(pb, 1); usr = CVPixelBufferGetBytesPerRowOfPlane(pb, 1);
		sv = CVPixelBufferGetBaseAddressOfPlane(pb, 2); vsr = CVPixelBufferGetBytesPerRowOfPlane(pb, 2);
	} else if (fmt == kCVPixelFormatType_32BGRA || fmt == kCVPixelFormatType_32ARGB) {
		sp = CVPixelBufferGetBaseAddress(pb); ysr = CVPixelBufferGetBytesPerRow(pb);
		bgra = fmt == kCVPixelFormatType_32BGRA; argb = !bgra;
	} else if (fmt == kCVPixelFormatType_OneComponent8) {
		sy = CVPixelBufferGetBaseAddress(pb); ysr = CVPixelBufferGetBytesPerRow(pb);
	}
	for (int y = 0; y < ch; y++) {
		size_t syy = (size_t)(y < s->height ? y : s->height - 1) * sh / s->height;
		uint8_t *dy = f->data[0] + (size_t)y * f->linesize[0];
		for (int x = 0; x < cw; x++) {
			size_t sxx = (size_t)(x < s->width ? x : s->width - 1) * sw / s->width;
			if (sp) {
				const uint8_t *px = sp + syy * ysr + sxx * 4;
				int r = bgra ? px[2] : px[1], g = bgra ? px[1] : px[2], b = bgra ? px[0] : px[3];
				dy[x] = clamp8(((47 * r + 157 * g + 16 * b + 128) >> 8) + 16);
			} else {
				dy[x] = sy[syy * ysr + sxx];
			}
		}
	}
	for (int y = 0; y < ch / 2; y++) {
		size_t dh2 = (s->height + 1) / 2;
		size_t syy = (size_t)(y < (int)dh2 ? y : (int)dh2 - 1) * ((sh + 1) / 2) / dh2;
		uint8_t *du = f->data[1] + (size_t)y * f->linesize[1];
		uint8_t *dv = f->data[2] + (size_t)y * f->linesize[2];
		for (int x = 0; x < cw / 2; x++) {
			size_t dw2 = (s->width + 1) / 2;
			size_t sxx = (size_t)(x < (int)dw2 ? x : (int)dw2 - 1) * ((sw + 1) / 2) / dw2;
			if (sp) {
				size_t ry = syy * 2 < sh ? syy * 2 : sh - 1, rx = sxx * 2 < sw ? sxx * 2 : sw - 1;
				const uint8_t *px = sp + ry * ysr + rx * 4;
				int r = bgra ? px[2] : px[1], g = bgra ? px[1] : px[2], b = bgra ? px[0] : px[3];
				du[x] = clamp8(((-26 * r - 87 * g + 112 * b + 128) >> 8) + 128);
				dv[x] = clamp8(((112 * r - 102 * g - 10 * b + 128) >> 8) + 128);
			} else if (suv) {
				const uint8_t *p = suv + syy * uvsr + sxx * 2;
				du[x] = p[0]; dv[x] = p[1];
			} else if (su) {
				du[x] = su[syy * usr + sxx]; dv[x] = sv[syy * vsr + sxx];
			} else {
				du[x] = dv[x] = 128;
			}
		}
	}
	(void)argb;
	CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly);
	return true;
}

static void encode_item(VTCS s, EncItem *it)
{
	pthread_mutex_lock(&s->lock);
	bool inv = s->invalidated;
	bool dirty = s->propsDirty;
	pthread_mutex_unlock(&s->lock);
	if (inv) {
		free_item(it);
		return;
	}
	if (s->avctx != NULL && dirty) {
		/* Changed properties apply to a new encoder instance: finish the frames of the old one first. */
		avcodec_send_frame(s->avctx, NULL);
		receive_all(s);
		fail_pending(s, kVTVideoEncoderMalfunctionErr);
		close_encoder(s);
	}
	OSStatus st = noErr;
	if (s->avctx == NULL)
		st = open_encoder(s);
	if (st == noErr && !fill_frame(s, it->pixelBuffer))
		st = kVTPixelTransferNotSupportedErr;
	if (st == noErr) {
		it->seq = s->nextSeq++;
		s->frame->pts = it->seq;
		s->frame->duration = 1;
		s->frame->pict_type = it->forceKey ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_NONE;
		if (avcodec_send_frame(s->avctx, s->frame) < 0) {
			st = kVTVideoEncoderMalfunctionErr;
		} else {
			if (s->dtsCount + s->dtsHead >= s->dtsCap) {
				if (s->dtsHead > 0) {
					memmove(s->dtsQueue, s->dtsQueue + s->dtsHead, s->dtsCount * sizeof(CMTime));
					s->dtsHead = 0;
				}
				if (s->dtsCount >= s->dtsCap) {
					size_t nc = s->dtsCap ? s->dtsCap * 2 : 16;
					CMTime *nq = realloc(s->dtsQueue, nc * sizeof(CMTime));
					if (nq) { s->dtsQueue = nq; s->dtsCap = nc; }
				}
			}
			if (s->dtsCount + s->dtsHead < s->dtsCap)
				s->dtsQueue[s->dtsHead + s->dtsCount++] = it->pts;
			CVPixelBufferRelease(it->pixelBuffer);
			it->pixelBuffer = NULL;
			it->next = NULL;
			if (s->pendTail) s->pendTail->next = it; else s->pendHead = it;
			s->pendTail = it;
			receive_all(s);
			return;
		}
	}
	deliver(s, it, st, NULL);
	free_item(it);
}

static void complete_all(VTCS s)
{
	if (s->avctx == NULL)
		return;
	avcodec_send_frame(s->avctx, NULL);
	receive_all(s);
	fail_pending(s, kVTVideoEncoderMalfunctionErr);
	if (s->avctx->codec->capabilities & AV_CODEC_CAP_ENCODER_FLUSH)
		avcodec_flush_buffers(s->avctx);
	else
		close_encoder(s);
}

#pragma mark - Public API

OSStatus VTCompressionSessionPrepareToEncodeFrames(VTCompressionSessionRef session)
{
	if (!is_session(session))
		return kVTParameterErr;
	pthread_mutex_lock(&session->lock);
	bool inv = session->invalidated;
	pthread_mutex_unlock(&session->lock);
	if (inv)
		/*
		 * QUIRK: PrepareToEncodeFrames on an invalidated session returns noErr, while EncodeFrame, CompleteFrames
		 * and the property calls return kVTInvalidSessionErr. Not documented. Evidence: black-box observation on
		 * macOS 26.7.3 and 10.14.4 (tier 2).
		 * Test: videotoolbox_compression, "prepare after invalidate".
		 */
		return noErr;
	__block OSStatus st = noErr;
	CFRetain(session);
	dispatch_sync(session->queue, ^{
		if (session->avctx == NULL)
			st = open_encoder(session);
	});
	CFRelease(session);
	return st;
}

static OSStatus submit(VTCS s, CVImageBufferRef buf, CMTime pts, CMTime dur, CFDictionaryRef props, void *refcon,
	VTEncodeInfoFlags *flagsOut, VTCompressionOutputHandler handler)
{
	if (flagsOut) *flagsOut = 0;
	if (!is_session(s) || buf == NULL || CFGetTypeID(buf) != CVPixelBufferGetTypeID())
		return kVTParameterErr;
	pthread_mutex_lock(&s->lock);
	bool inv = s->invalidated;
	pthread_mutex_unlock(&s->lock);
	if (inv)
		return kVTInvalidSessionErr;
	OSType f = CVPixelBufferGetPixelFormatType(buf);
	if (f != kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange && f != kCVPixelFormatType_420YpCbCr8BiPlanarFullRange &&
	    f != kCVPixelFormatType_420YpCbCr8Planar && f != kCVPixelFormatType_32BGRA && f != kCVPixelFormatType_32ARGB &&
	    f != kCVPixelFormatType_OneComponent8)
		return kVTPixelTransferNotSupportedErr;

	EncItem *it = calloc(1, sizeof(EncItem));
	if (it == NULL)
		return kVTAllocationFailedErr;
	it->pixelBuffer = CVPixelBufferRetain(buf);
	it->pts = pts;
	it->duration = dur;
	it->refcon = refcon;
	if (props != NULL) {
		CFTypeRef fk = CFDictionaryGetValue(props, kVTEncodeFrameOptionKey_ForceKeyFrame);
		it->forceKey = fk != NULL && CFGetTypeID(fk) == CFBooleanGetTypeID() && CFBooleanGetValue(fk);
	}
	if (handler)
		it->handler = Block_copy(handler);
	CFRetain(s);
	dispatch_async(s->queue, ^{
		encode_item(s, it);
		CFRelease(s);
	});
	if (flagsOut) *flagsOut = kVTEncodeInfo_Asynchronous;
	return noErr;
}

OSStatus VTCompressionSessionEncodeFrame(VTCompressionSessionRef session, CVImageBufferRef imageBuffer,
	CMTime presentationTimeStamp, CMTime duration, CFDictionaryRef frameProperties, void *sourceFrameRefcon,
	VTEncodeInfoFlags *infoFlagsOut)
{
	return submit(session, imageBuffer, presentationTimeStamp, duration, frameProperties, sourceFrameRefcon, infoFlagsOut, NULL);
}

OSStatus VTCompressionSessionEncodeFrameWithOutputHandler(VTCompressionSessionRef session, CVImageBufferRef imageBuffer,
	CMTime presentationTimeStamp, CMTime duration, CFDictionaryRef frameProperties, VTEncodeInfoFlags *infoFlagsOut,
	VTCompressionOutputHandler outputHandler)
{
	if (outputHandler == NULL)
		return kVTParameterErr;
	return submit(session, imageBuffer, presentationTimeStamp, duration, frameProperties, NULL, infoFlagsOut, outputHandler);
}

OSStatus VTCompressionSessionCompleteFrames(VTCompressionSessionRef session, CMTime completeUntilPresentationTimeStamp)
{
	if (!is_session(session))
		return kVTParameterErr;
	pthread_mutex_lock(&session->lock);
	bool inv = session->invalidated;
	pthread_mutex_unlock(&session->lock);
	if (inv)
		return kVTInvalidSessionErr;
	if (dispatch_get_specific(&queueKey) == session) {
		complete_all(session);
		return noErr;
	}
	CFRetain(session);
	dispatch_sync(session->queue, ^{ complete_all(session); });
	CFRelease(session);
	return noErr;
}

void VTCompressionSessionInvalidate(VTCompressionSessionRef session)
{
	if (!is_session(session))
		return;
	pthread_mutex_lock(&session->lock);
	if (session->invalidated) {
		pthread_mutex_unlock(&session->lock);
		return;
	}
	session->invalidated = true;
	CVPixelBufferPoolRef pool = session->pool;
	session->pool = NULL;
	pthread_mutex_unlock(&session->lock);
	if (pool)
		CVPixelBufferPoolRelease(pool);
	if (dispatch_get_specific(&queueKey) == session)
		return; /* torn down when the last reference goes */
	CFRetain(session);
	dispatch_sync(session->queue, ^{ close_encoder(session); });
	CFRelease(session);
}
