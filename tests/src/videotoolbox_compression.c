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
 * VTCompressionSession contract: H.264 / HEVC sessions encode CVPixelBuffers into CMSampleBuffers that carry an
 * avcC / hvcC format description and sync attachments, and decode back through VTDecompressionSession with a
 * high PSNR. The expectations were also observed on macOS (the test runs there too).
 * Exit status 77 means SKIP (no host encoder available).
 */

#include <VideoToolbox/VideoToolbox.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL line %d: %s\n", __LINE__, #cond); \
	} \
} while (0)

#define W 320
#define H 240
#define NFRAMES 30
#define PSNR_MIN 30.0

typedef struct Out {
	pthread_mutex_t lock;
	int count;
	CMSampleBufferRef sb[64];
	long refcon[64];
	OSStatus status[64];
	VTEncodeInfoFlags flags[64];
} Out;

static void out_callback(void *ref, void *src, OSStatus st, VTEncodeInfoFlags fl, CMSampleBufferRef sb)
{
	Out *o = ref;
	pthread_mutex_lock(&o->lock);
	if (o->count < 64) {
		o->sb[o->count] = sb ? (CMSampleBufferRef)CFRetain(sb) : NULL;
		o->refcon[o->count] = (long)src;
		o->status[o->count] = st;
		o->flags[o->count] = fl;
		o->count++;
	}
	pthread_mutex_unlock(&o->lock);
}

static void out_free(Out *o)
{
	for (int i = 0; i < o->count; i++)
		if (o->sb[i]) CFRelease(o->sb[i]);
	o->count = 0;
}

static uint8_t luma_at(int n, int x, int y)
{
	/* smooth gradient plus a moving square: compresses well but is not trivial */
	int v = 40 + (x * 120) / W + (y * 60) / H;
	if (x >= (n * 5) % (W - 40) && x < (n * 5) % (W - 40) + 40 && y >= 80 && y < 120)
		v = 220;
	return (uint8_t)v;
}

static CVPixelBufferRef make_nv12(int n, OSType fmt)
{
	CVPixelBufferRef pb = NULL;
	if (CVPixelBufferCreate(NULL, W, H, fmt, NULL, &pb) != kCVReturnSuccess)
		return NULL;
	CVPixelBufferLockBaseAddress(pb, 0);
	uint8_t *y = CVPixelBufferGetBaseAddressOfPlane(pb, 0);
	size_t ys = CVPixelBufferGetBytesPerRowOfPlane(pb, 0);
	for (int j = 0; j < H; j++)
		for (int i = 0; i < W; i++)
			y[j * ys + i] = luma_at(n, i, j);
	uint8_t *uv = CVPixelBufferGetBaseAddressOfPlane(pb, 1);
	size_t uvs = CVPixelBufferGetBytesPerRowOfPlane(pb, 1);
	for (int j = 0; j < H / 2; j++)
		for (int i = 0; i < W / 2; i++) {
			uv[j * uvs + 2 * i] = 110 + i / 8;
			uv[j * uvs + 2 * i + 1] = 140 - j / 8;
		}
	CVPixelBufferUnlockBaseAddress(pb, 0);
	return pb;
}

typedef struct Dec {
	pthread_mutex_t lock;
	double psnrMin;
	int frames;
	int frameIndexByPts[64]; /* pts value -> source frame index */
} Dec;

static void dec_callback(void *ref, void *src, OSStatus st, VTDecodeInfoFlags fl, CVImageBufferRef img, CMTime pts, CMTime dur)
{
	Dec *d = ref;
	if (st != noErr || img == NULL) {
		failures++;
		printf("FAIL decode status %d\n", (int)st);
		return;
	}
	int n = (int)(long)src;
	CVPixelBufferLockBaseAddress(img, kCVPixelBufferLock_ReadOnly);
	const uint8_t *y = CVPixelBufferGetBaseAddressOfPlane(img, 0);
	size_t ys = CVPixelBufferGetBytesPerRowOfPlane(img, 0);
	double se = 0;
	for (int j = 0; j < H; j++)
		for (int i = 0; i < W; i++) {
			int diff = (int)y[j * ys + i] - (int)luma_at(n, i, j);
			se += diff * diff;
		}
	CVPixelBufferUnlockBaseAddress(img, kCVPixelBufferLock_ReadOnly);
	double mse = se / (W * H);
	double psnr = mse == 0 ? 99 : 10 * log10(255.0 * 255.0 / mse);
	pthread_mutex_lock(&d->lock);
	if (psnr < d->psnrMin) d->psnrMin = psnr;
	d->frames++;
	pthread_mutex_unlock(&d->lock);
}

static bool sample_has_key(CMSampleBufferRef sb)
{
	CFArrayRef a = CMSampleBufferGetSampleAttachmentsArray(sb, false);
	if (a == NULL || CFArrayGetCount(a) == 0)
		return true;
	CFDictionaryRef d = CFArrayGetValueAtIndex(a, 0);
	CFTypeRef ns = CFDictionaryGetValue(d, kCMSampleAttachmentKey_NotSync);
	return ns == NULL || !CFBooleanGetValue(ns);
}

static bool sample_depends(CMSampleBufferRef sb)
{
	CFArrayRef a = CMSampleBufferGetSampleAttachmentsArray(sb, false);
	if (a == NULL || CFArrayGetCount(a) == 0)
		return false;
	CFTypeRef v = CFDictionaryGetValue(CFArrayGetValueAtIndex(a, 0), CFSTR("DependsOnOthers"));
	return v != NULL && CFBooleanGetValue(v);
}

static void set_bool(VTCompressionSessionRef s, CFStringRef k, bool v)
{
	CHECK(VTSessionSetProperty(s, k, v ? kCFBooleanTrue : kCFBooleanFalse) == noErr);
}

static void set_int(VTCompressionSessionRef s, CFStringRef k, int v)
{
	CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &v);
	CHECK(VTSessionSetProperty(s, k, n) == noErr);
	CFRelease(n);
}

/* Encodes N frames, decodes them back, checks the stream. Returns false if the codec is unavailable. */
static bool run_codec(CMVideoCodecType codec, bool reorder, const char *name)
{
	Out out;
	memset(&out, 0, sizeof(out));
	pthread_mutex_init(&out.lock, NULL);
	VTCompressionSessionRef s = NULL;
	OSStatus st = VTCompressionSessionCreate(NULL, W, H, codec, NULL, NULL, NULL, out_callback, &out, &s);
	if (st == kVTCouldNotFindVideoEncoderErr && codec != kCMVideoCodecType_H264) {
		printf("SKIP %s: no encoder\n", name);
		return false;
	}
	if (st != noErr || s == NULL) {
		failures++;
		printf("FAIL %s: VTCompressionSessionCreate returned %d\n", name, (int)st);
		return true;
	}
	CHECK(CFGetTypeID(s) == VTCompressionSessionGetTypeID());
	CVPixelBufferPoolRef pool = VTCompressionSessionGetPixelBufferPool(s);
	CHECK(pool != NULL);

	set_bool(s, kVTCompressionPropertyKey_AllowFrameReordering, reorder);
	set_int(s, kVTCompressionPropertyKey_AverageBitRate, 1500000);
	set_int(s, kVTCompressionPropertyKey_MaxKeyFrameInterval, 100);
	CHECK(VTCompressionSessionPrepareToEncodeFrames(s) == noErr);

	const int forceAt = 15;
	for (int n = 0; n < NFRAMES; n++) {
		CVPixelBufferRef pb = make_nv12(n, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
		CFDictionaryRef props = NULL;
		if (n == forceAt) {
			const void *k[] = { kVTEncodeFrameOptionKey_ForceKeyFrame };
			const void *v[] = { kCFBooleanTrue };
			props = CFDictionaryCreate(NULL, k, v, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		}
		VTEncodeInfoFlags fl = 0;
		st = VTCompressionSessionEncodeFrame(s, pb, CMTimeMake(n * 100, 3000), CMTimeMake(100, 3000), props, (void *)(long)n, &fl);
		CHECK(st == noErr);
		if (props) CFRelease(props);
		CFRelease(pb);
	}
	CHECK(VTCompressionSessionCompleteFrames(s, kCMTimeInvalid) == noErr);

	pthread_mutex_lock(&out.lock);
	CHECK(out.count == NFRAMES);
	pthread_mutex_unlock(&out.lock);
	if (out.count != NFRAMES) {
		printf("  %s: got %d samples\n", name, out.count);
		VTCompressionSessionInvalidate(s);
		CFRelease(s);
		out_free(&out);
		return true;
	}

	/* Every source frame comes back exactly once with its timing and no error. */
	int seen[NFRAMES] = { 0 };
	int keyCount = 0;
	for (int i = 0; i < out.count; i++) {
		CHECK(out.status[i] == noErr);
		CHECK(out.sb[i] != NULL);
		long r = out.refcon[i];
		CHECK(r >= 0 && r < NFRAMES);
		if (r >= 0 && r < NFRAMES) seen[r]++;
		if (!out.sb[i]) continue;
		CMTime pts = CMSampleBufferGetPresentationTimeStamp(out.sb[i]);
		CHECK(pts.value == r * 100 && pts.timescale == 3000);
		CHECK(CMSampleBufferGetNumSamples(out.sb[i]) == 1);
		CHECK(CMBlockBufferGetDataLength(CMSampleBufferGetDataBuffer(out.sb[i])) > 0);
		CHECK(CMSampleBufferGetDataBuffer(out.sb[i]) != NULL);
		bool key = sample_has_key(out.sb[i]);
		CHECK(key == !sample_depends(out.sb[i]));
		if (key) keyCount++;
	}
	for (int n = 0; n < NFRAMES; n++)
		CHECK(seen[n] == 1);
	CHECK(keyCount >= 2); /* the first frame and the forced one */
	for (int i = 0; i < out.count; i++)
		if (out.refcon[i] == 0 || out.refcon[i] == forceAt)
			CHECK(out.sb[i] && sample_has_key(out.sb[i]));
	CHECK(out.refcon[0] == 0 && sample_has_key(out.sb[0]));

	/* Format description: codec, size, configuration record. */
	CMFormatDescriptionRef fd = CMSampleBufferGetFormatDescription(out.sb[0]);
	CHECK(fd != NULL);
	CMVideoFormatDescriptionRef vfd = (CMVideoFormatDescriptionRef)fd;
	CHECK(CMFormatDescriptionGetMediaSubType(fd) == codec);
	CMVideoDimensions dm = CMVideoFormatDescriptionGetDimensions(vfd);
	CHECK(dm.width == W && dm.height == H);
	CFDictionaryRef atoms = CMFormatDescriptionGetExtension(fd, kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms);
	CHECK(atoms != NULL);
	CFDataRef cfg = atoms ? CFDictionaryGetValue(atoms, codec == kCMVideoCodecType_H264 ? CFSTR("avcC") : CFSTR("hvcC")) : NULL;
	CHECK(cfg != NULL && CFDataGetLength(cfg) > 20 && CFDataGetBytePtr(cfg)[0] == 1);
	if (cfg && codec == kCMVideoCodecType_H264)
		CHECK((CFDataGetBytePtr(cfg)[4] & 3) == 3 && (CFDataGetBytePtr(cfg)[5] & 0x1f) >= 1); /* 4-byte lengths, >= 1 SPS */
	/* Samples are length-prefixed: the 4-byte prefix covers the whole NAL unit. */
	{
		CMBlockBufferRef bb = CMSampleBufferGetDataBuffer(out.sb[0]);
		size_t len = CMBlockBufferGetDataLength(bb);
		uint8_t head[4];
		CHECK(CMBlockBufferCopyDataBytes(bb, 0, 4, head) == noErr);
		size_t first = ((size_t)head[0] << 24) | (head[1] << 16) | (head[2] << 8) | head[3];
		CHECK(first > 0 && first + 4 <= len);
	}

	/* Decode back through the decoder and compare. */
	Dec dec;
	memset(&dec, 0, sizeof(dec));
	pthread_mutex_init(&dec.lock, NULL);
	dec.psnrMin = 99;
	VTDecompressionOutputCallbackRecord rec = { dec_callback, &dec };
	VTDecompressionSessionRef ds = NULL;
	st = VTDecompressionSessionCreate(NULL, vfd, NULL, NULL, &rec, &ds);
	CHECK(st == noErr);
	if (st == noErr) {
		/* decode in decode order; frame index travels as the source refcon */
		for (int i = 0; i < out.count; i++) {
			VTDecodeInfoFlags fl = 0;
			st = VTDecompressionSessionDecodeFrame(ds, out.sb[i], 0, (void *)out.refcon[i], &fl);
			CHECK(st == noErr);
		}
		VTDecompressionSessionFinishDelayedFrames(ds);
		VTDecompressionSessionWaitForAsynchronousFrames(ds);
		CHECK(dec.frames == NFRAMES);
		printf("  %s reorder=%d: decoded %d frames, min PSNR %.1f dB\n", name, reorder, dec.frames, dec.psnrMin);
		CHECK(dec.psnrMin >= PSNR_MIN);
		VTDecompressionSessionInvalidate(ds);
		CFRelease(ds);
	}

	/* After Invalidate the session refuses work. */
	VTCompressionSessionInvalidate(s);
	CVPixelBufferRef pb = make_nv12(0, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
	CHECK(VTCompressionSessionEncodeFrame(s, pb, CMTimeMake(0, 30), kCMTimeInvalid, NULL, NULL, NULL) == kVTInvalidSessionErr);
	CHECK(VTCompressionSessionCompleteFrames(s, kCMTimeInvalid) == kVTInvalidSessionErr);
	CHECK(VTSessionSetProperty(s, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue) == kVTInvalidSessionErr);
	/* prepare after invalidate: QUIRK, returns noErr on macOS (see VTCompressionSession.c) */
	CHECK(VTCompressionSessionPrepareToEncodeFrames(s) == noErr);
	CFRelease(pb);
	CFRelease(s);
	out_free(&out);
	return true;
}

int main(void)
{
	VTCompressionSessionRef s = NULL;
	OSStatus st;

	setvbuf(stdout, NULL, _IOLBF, 0);
#ifdef __i386__
	/* the 32-bit guest has no host FFmpeg (the decoder tests skip the same way); the Mac never refuses H.264 */
	st = VTCompressionSessionCreate(NULL, W, H, kCMVideoCodecType_H264, NULL, NULL, NULL, out_callback, NULL, &s);
	if (st == kVTCouldNotFindVideoEncoderErr) {
		printf("SKIP: no 32-bit host encoder\n");
		return 77;
	}
	if (s) { VTCompressionSessionInvalidate(s); CFRelease(s); s = NULL; }
#endif
	/* Arguments */
	CHECK(VTCompressionSessionCreate(NULL, 0, H, kCMVideoCodecType_H264, NULL, NULL, NULL, out_callback, NULL, &s) == kVTParameterErr);
	CHECK(s == NULL);
	CHECK(VTCompressionSessionCreate(NULL, W, 0, kCMVideoCodecType_H264, NULL, NULL, NULL, out_callback, NULL, &s) == kVTParameterErr);
	CHECK(VTCompressionSessionCreate(NULL, W, H, 0x61626364 /* 'abcd' */, NULL, NULL, NULL, out_callback, NULL, &s) == kVTCouldNotFindVideoEncoderErr);
	{
		const void *k[] = { kVTVideoEncoderSpecification_RequireHardwareAcceleratedVideoEncoder };
		const void *v[] = { kCFBooleanTrue };
		CFDictionaryRef spec = CFDictionaryCreate(NULL, k, v, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		st = VTCompressionSessionCreate(NULL, W, H, kCMVideoCodecType_H264, spec, NULL, NULL, out_callback, NULL, &s);
		CHECK(st == noErr || st == kVTCouldNotFindVideoEncoderErr); /* no hardware encoder here, the Mac may have one */
		if (s) { VTCompressionSessionInvalidate(s); CFRelease(s); s = NULL; }
		CFRelease(spec);
	}

	/* The encoder list names H.264 */
	CFArrayRef list = NULL;
	OSStatus (*copyList)(CFDictionaryRef, CFArrayRef *) = (OSStatus (*)(CFDictionaryRef, CFArrayRef *))VTCopyVideoEncoderList;
	CHECK(copyList(NULL, &list) == noErr && list != NULL);
	bool listed = false;
	for (CFIndex i = 0; list && i < CFArrayGetCount(list); i++) {
		CFDictionaryRef e = CFArrayGetValueAtIndex(list, i);
		CFNumberRef t = CFDictionaryGetValue(e, kVTVideoEncoderList_CodecType);
		int32_t v = 0;
		if (t && CFNumberGetValue(t, kCFNumberSInt32Type, &v) && v == kCMVideoCodecType_H264)
			listed = true;
	}
	CHECK(listed);
	if (list) CFRelease(list);

	/* Properties and pool */
	st = VTCompressionSessionCreate(NULL, W, H, kCMVideoCodecType_H264, NULL, NULL, NULL, out_callback, NULL, &s);
	CHECK(st == noErr);
	if (st != noErr) {
		printf("FAIL: cannot create an H.264 session (%d)\n", (int)st);
		printf("%d failure(s)\n", failures);
		return 1;
	}
	CVPixelBufferPoolRef pool = VTCompressionSessionGetPixelBufferPool(s);
	CHECK(pool != NULL);
	if (pool) {
		CFDictionaryRef pa = CVPixelBufferPoolGetPixelBufferAttributes(pool);
		int32_t w = 0, h = 0, f = 0;
		CHECK(pa != NULL);
		if (pa) {
			CFNumberGetValue(CFDictionaryGetValue(pa, kCVPixelBufferWidthKey), kCFNumberSInt32Type, &w);
			CFNumberGetValue(CFDictionaryGetValue(pa, kCVPixelBufferHeightKey), kCFNumberSInt32Type, &h);
			CFNumberGetValue(CFDictionaryGetValue(pa, kCVPixelBufferPixelFormatTypeKey), kCFNumberSInt32Type, &f);
		}
		CHECK(w == W && h == H && f == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
		CVPixelBufferRef pb = NULL;
		CHECK(CVPixelBufferPoolCreatePixelBuffer(NULL, pool, &pb) == kCVReturnSuccess && pb != NULL);
		if (pb) CFRelease(pb);
	}
	CHECK(VTSessionSetProperty(s, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue) == noErr);
	CHECK(VTSessionSetProperty(s, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_Main_AutoLevel) == noErr);
	CHECK(VTSessionSetProperty(s, kVTCompressionPropertyKey_RealTime, CFSTR("x")) == kVTParameterErr);
	CHECK(VTSessionSetProperty(s, kVTCompressionPropertyKey_AverageBitRate, kCFBooleanTrue) == kVTParameterErr);
	CHECK(VTSessionSetProperty(s, kVTCompressionPropertyKey_ProfileLevel, CFSTR("Nonsense")) == kVTParameterErr);
	CHECK(VTSessionSetProperty(s, CFSTR("NoSuchProperty"), kCFBooleanTrue) == kVTPropertyNotSupportedErr);
	CFTypeRef v = NULL;
	CHECK(VTSessionCopyProperty(s, kVTCompressionPropertyKey_RealTime, NULL, &v) == noErr && v == kCFBooleanTrue);
	if (v) CFRelease(v);
	v = NULL;
	CHECK(VTSessionCopyProperty(s, CFSTR("NoSuchProperty"), NULL, &v) == kVTPropertyNotSupportedErr);
	CFDictionaryRef sup = NULL;
	CHECK(VTSessionCopySupportedPropertyDictionary(s, &sup) == noErr && sup != NULL);
	if (sup) {
		CHECK(CFDictionaryContainsKey(sup, kVTCompressionPropertyKey_AverageBitRate));
		CHECK(CFDictionaryContainsKey(sup, kVTCompressionPropertyKey_ProfileLevel));
		CFRelease(sup);
	}
	VTCompressionSessionInvalidate(s);
	CFRelease(s);

	/* A BGRA source frame is accepted */
	{
		Out out;
		memset(&out, 0, sizeof(out));
		pthread_mutex_init(&out.lock, NULL);
		VTCompressionSessionRef bs = NULL;
		CHECK(VTCompressionSessionCreate(NULL, W, H, kCMVideoCodecType_H264, NULL, NULL, NULL, out_callback, &out, &bs) == noErr);
		CVPixelBufferRef pb = NULL;
		CVPixelBufferCreate(NULL, W, H, kCVPixelFormatType_32BGRA, NULL, &pb);
		CVPixelBufferLockBaseAddress(pb, 0);
		memset(CVPixelBufferGetBaseAddress(pb), 0x80, CVPixelBufferGetBytesPerRow(pb) * H);
		CVPixelBufferUnlockBaseAddress(pb, 0);
		CHECK(VTCompressionSessionEncodeFrame(bs, pb, CMTimeMake(0, 30), CMTimeMake(1, 30), NULL, (void *)1, NULL) == noErr);
		CHECK(VTCompressionSessionCompleteFrames(bs, kCMTimeInvalid) == noErr);
		CHECK(out.count == 1 && out.status[0] == noErr && out.sb[0] != NULL);
		CFRelease(pb);
		VTCompressionSessionInvalidate(bs);
		CFRelease(bs);
		out_free(&out);
	}

	run_codec(kCMVideoCodecType_H264, false, "H.264");
	run_codec(kCMVideoCodecType_H264, true, "H.264");
	run_codec(kCMVideoCodecType_HEVC, false, "HEVC");

	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
