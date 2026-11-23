/*
 * AudioConverter decoding compressed audio to float PCM: AAC, MP3, AC-3, E-AC-3, FLAC and Apple
 * Lossless, with the magic cookies of those formats in the forms Core Audio documents (the ES
 * descriptor for AAC, the "fLaC" marker and STREAMINFO block for FLAC), and the error codes of a
 * converter that cannot be set up.
 *
 * The fixtures are the tones-*.* files of tests/avplayer-fixtures (440 Hz for the first second,
 * 880 Hz for the second one). Packets are read with AudioFile, except for the E-AC-3 elementary
 * stream, which is cut into its synchronization frames here. Run with the fixture directory:
 *   audioconverter_compressed <dir>
 *
 * Exit status is 0 when every check passed, 77 if the host FFmpeg is not available.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

struct packets {
	unsigned char *data;
	AudioStreamPacketDescription *desc;
	UInt32 count;
	UInt32 next;
	int ended;
};

static OSStatus feed(AudioConverterRef conv, UInt32 *numPackets, AudioBufferList *data,
	AudioStreamPacketDescription **descs, void *user)
{
	struct packets *p = user;
	if (p->next >= p->count) {
		*numPackets = 0;
		data->mBuffers[0].mDataByteSize = 0;
		data->mBuffers[0].mData = NULL;
		p->ended = 1;
		return noErr;
	}
	AudioStreamPacketDescription *d = &p->desc[p->next++];
	data->mNumberBuffers = 1;
	data->mBuffers[0].mNumberChannels = 1;
	data->mBuffers[0].mDataByteSize = d->mDataByteSize;
	data->mBuffers[0].mData = p->data + d->mStartOffset;
	if (descs)
		*descs = d;
	*numPackets = 1;
	return noErr;
}

static int read_packets(const char *path, struct packets *p, AudioStreamBasicDescription *fmt, CFDataRef *cookie)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)path, strlen(path), false);
	AudioFileID file = NULL;
	OSStatus st = AudioFileOpenURL(url, kAudioFileReadPermission, 0, &file);
	CFRelease(url);
	if (st != noErr)
		return -1;
	UInt32 size = sizeof(*fmt);
	AudioFileGetProperty(file, kAudioFilePropertyDataFormat, &size, fmt);
	UInt64 count = 0;
	size = sizeof(count);
	AudioFileGetProperty(file, kAudioFilePropertyAudioDataPacketCount, &size, &count);
	UInt32 maxPacket = 0;
	size = sizeof(maxPacket);
	AudioFileGetProperty(file, kAudioFilePropertyPacketSizeUpperBound, &size, &maxPacket);
	UInt32 cookieSize = 0, writable = 0;
	*cookie = NULL;
	if (AudioFileGetPropertyInfo(file, kAudioFilePropertyMagicCookieData, &cookieSize, &writable) == noErr && cookieSize) {
		void *bytes = malloc(cookieSize);
		AudioFileGetProperty(file, kAudioFilePropertyMagicCookieData, &cookieSize, bytes);
		*cookie = CFDataCreate(NULL, bytes, cookieSize);
		free(bytes);
	}
	size_t capacity = (size_t)(maxPacket ? maxPacket : 16384) * (size_t)(count ? count : 1) + 65536;
	p->data = malloc(capacity);
	p->desc = calloc((size_t)count + 1, sizeof(*p->desc));
	size_t used = 0;
	for (UInt64 i = 0; i < count; i++) {
		UInt32 bytes = (UInt32)(capacity - used), n = 1;
		AudioStreamPacketDescription d;
		st = AudioFileReadPacketData(file, false, &bytes, &d, (SInt64)i, &n, p->data + used);
		if (st != noErr || n != 1)
			break;
		d.mStartOffset = (SInt64)used;
		p->desc[p->count++] = d;
		used += bytes;
	}
	AudioFileClose(file);
	return p->count ? 0 : -1;
}

// E-AC-3 elementary stream: each syncframe starts with 0x0B77 and states its size in 16-bit words.
static int split_eac3(const char *path, struct packets *p, int *sampleRate, int *channels)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return -1;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	p->data = malloc((size_t)size);
	if (fread(p->data, 1, (size_t)size, f) != (size_t)size) {
		fclose(f);
		return -1;
	}
	fclose(f);
	p->desc = calloc((size_t)size / 64 + 1, sizeof(*p->desc));
	long at = 0;
	static const int rates[] = { 48000, 44100, 32000 };
	while (at + 6 < size && p->data[at] == 0x0b && p->data[at + 1] == 0x77) {
		int frameWords = (((p->data[at + 2] & 7) << 8) | p->data[at + 3]) + 1;
		int fscod = p->data[at + 4] >> 6;
		if (p->count == 0) {
			*sampleRate = fscod < 3 ? rates[fscod] : 0;
			static const int acmodChannels[] = { 2, 1, 2, 3, 3, 4, 4, 5 };
			*channels = acmodChannels[(p->data[at + 4] >> 1) & 7];
		}
		p->desc[p->count].mStartOffset = at;
		p->desc[p->count].mDataByteSize = (UInt32)(frameWords * 2);
		p->desc[p->count].mVariableFramesInPacket = 0;
		p->count++;
		at += frameWords * 2;
	}
	return p->count ? 0 : -1;
}

static AudioStreamBasicDescription pcm_float(double rate, UInt32 channels)
{
	AudioStreamBasicDescription pcm = {0};
	pcm.mSampleRate = rate;
	pcm.mFormatID = kAudioFormatLinearPCM;
	pcm.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
	pcm.mChannelsPerFrame = channels;
	pcm.mBitsPerChannel = 32;
	pcm.mFramesPerPacket = 1;
	pcm.mBytesPerFrame = pcm.mBytesPerPacket = 4 * channels;
	return pcm;
}

// Decodes everything; returns the number of frames, filling *out (interleaved).
static long decode_all(AudioConverterRef conv, struct packets *p, UInt32 channels, float **out)
{
	long capacity = 44100 * 4, total = 0;
	float *samples = malloc((size_t)capacity * channels * sizeof(float));
	for (int guard = 0; guard < 4000; guard++) {
		if (total + 4096 > capacity) {
			capacity *= 2;
			samples = realloc(samples, (size_t)capacity * channels * sizeof(float));
		}
		AudioBufferList list;
		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = channels;
		list.mBuffers[0].mDataByteSize = 4096 * channels * sizeof(float);
		list.mBuffers[0].mData = samples + total * channels;
		UInt32 frames = 4096;
		OSStatus st = AudioConverterFillComplexBuffer(conv, feed, p, &frames, &list, NULL);
		long got = list.mBuffers[0].mDataByteSize / (channels * sizeof(float));
		total += got;
		if (st != noErr || (got == 0 && p->ended))
			break;
	}
	*out = samples;
	return total;
}

// Zero-crossing frequency of the frames [from, to) of the first channel.
static double frequency(const float *samples, UInt32 channels, double rate, long from, long to)
{
	long crossings = 0;
	for (long i = from + 1; i < to; i++) {
		if ((samples[(i - 1) * channels] < 0) != (samples[i * channels] < 0))
			crossings++;
	}
	return crossings * rate / (double)(to - from) / 2.0;
}

static double peak(const float *samples, UInt32 channels, long from, long to)
{
	double m = 0;
	for (long i = from; i < to; i++) {
		double v = fabs(samples[i * channels]);
		if (v > m)
			m = v;
	}
	return m;
}

// The checks every fixture shares: about two seconds, 440 Hz and then 880 Hz at an amplitude of 0.5.
static void check_tones(const char *name, const float *samples, long frames, UInt32 channels, double rate)
{
	printf("%s: %ld frames (%.3f s)\n", name, frames, frames / rate);
	CHECK(frames > rate * 1.9 && frames < rate * 2.2, "%s: %ld frames", name, frames);
	if (frames < rate * 1.9)
		return;
	double f1 = frequency(samples, channels, rate, (long)(rate * 0.3), (long)(rate * 0.8));
	double f2 = frequency(samples, channels, rate, (long)(rate * 1.3), (long)(rate * 1.8));
	double p = peak(samples, channels, (long)(rate * 0.3), (long)(rate * 1.8));
	CHECK(fabs(f1 - 440) < 12, "%s: first tone %.1f Hz", name, f1);
	CHECK(fabs(f2 - 880) < 24, "%s: second tone %.1f Hz", name, f2);
	CHECK(p > 0.4 && p < 0.65, "%s: amplitude %.3f", name, p);
}

static CFDataRef aac_cookie_in_es_descriptor(CFDataRef asc)
{
	// ES_Descriptor { ES_ID, flags, DecoderConfigDescriptor { 13 bytes, DecoderSpecificInfo } }
	CFIndex n = CFDataGetLength(asc);
	UInt8 head[] = {
		0x03, 0x80, 0x80, 0x80, (UInt8)(3 + 5 + 13 + 5 + n), 0, 1, 0,
		0x04, 0x80, 0x80, 0x80, (UInt8)(13 + 5 + n), 0x40, 0x15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0x05, 0x80, 0x80, 0x80, (UInt8)n,
	};
	CFMutableDataRef d = CFDataCreateMutable(NULL, 0);
	CFDataAppendBytes(d, head, sizeof(head));
	CFDataAppendBytes(d, CFDataGetBytePtr(asc), n);
	return d;
}

static void decode_file(const char *dir, const char *file, UInt32 channelsOut, int cookieStyle)
{
	char path[1024];
	snprintf(path, sizeof path, "%s/%s", dir, file);
	struct packets p = {0};
	AudioStreamBasicDescription in = {0};
	CFDataRef cookie = NULL;
	if (read_packets(path, &p, &in, &cookie) != 0) {
		printf("FAIL %s: cannot read the packets\n", file);
		failures++;
		return;
	}
	AudioStreamBasicDescription out = pcm_float(in.mSampleRate, in.mChannelsPerFrame > 1 ? 2 : 1);
	AudioConverterRef conv = NULL;
	OSStatus st = AudioConverterNew(&in, &out, &conv);
	CHECK(st == noErr, "%s: AudioConverterNew = %d", file, (int)st);
	if (st != noErr)
		return;

	if (cookie && cookieStyle) {
		CFDataRef use = cookie;
		if (cookieStyle == 2 && in.mFormatID == kAudioFormatMPEG4AAC)
			use = aac_cookie_in_es_descriptor(cookie);
		if (cookieStyle == 2 && in.mFormatID == kAudioFormatFLAC) {
			CFMutableDataRef wrapped = CFDataCreateMutable(NULL, 0);
			UInt8 head[] = { 'f', 'L', 'a', 'C', 0x80, 0, 0, 34 };
			CFDataAppendBytes(wrapped, head, sizeof head);
			CFDataAppendBytes(wrapped, CFDataGetBytePtr(cookie), CFDataGetLength(cookie));
			use = wrapped;
		}
		st = AudioConverterSetProperty(conv, kAudioConverterDecompressionMagicCookie, (UInt32)CFDataGetLength(use), CFDataGetBytePtr(use));
		CHECK(st == noErr, "%s: setting the magic cookie (style %d) = %d", file, cookieStyle, (int)st);
	}
	float *samples = NULL;
	long frames = decode_all(conv, &p, out.mChannelsPerFrame, &samples);
	check_tones(file, samples, frames, out.mChannelsPerFrame, out.mSampleRate);
	(void)channelsOut;
	free(samples);
	AudioConverterDispose(conv);
	free(p.data);
	free(p.desc);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("usage: audioconverter_compressed <fixture directory>\n");
		return 1;
	}
	const char *dir = argv[1];

	/* Without the host FFmpeg libraries compressed audio is unsupported: the status says so */
	AudioStreamBasicDescription aac = {0}, pcm = pcm_float(44100, 2);
	aac.mSampleRate = 44100;
	aac.mFormatID = kAudioFormatMPEG4AAC;
	aac.mFramesPerPacket = 1024;
	aac.mChannelsPerFrame = 2;
	AudioConverterRef probe = NULL;
	OSStatus st = AudioConverterNew(&aac, &pcm, &probe);
	if (st == kAudioConverterErr_FormatNotSupported) {
		printf("SKIP: no host FFmpeg\n");
		return 77;
	}
	CHECK(st == noErr, "AAC converter = %d", (int)st);
	if (st == noErr)
		AudioConverterDispose(probe);

	/* The cookie as the container gives it (AudioSpecificConfig, STREAMINFO) and in the documented wrapping */
	decode_file(dir, "tones-aac.m4a", 2, 1);
	decode_file(dir, "tones-aac.m4a", 2, 2);
	decode_file(dir, "tones-mp3.mp3", 2, 0);
	decode_file(dir, "tones-ac3.ac3", 2, 0);
	decode_file(dir, "tones-flac.flac", 1, 1);
	decode_file(dir, "tones-flac.flac", 1, 2);
	decode_file(dir, "tones-alac.m4a", 1, 1);

	/* E-AC-3 */
	{
		char path[1024];
		snprintf(path, sizeof path, "%s/tones-eac3.eac3", dir);
		struct packets p = {0};
		int rate = 0, channels = 0;
		CHECK(split_eac3(path, &p, &rate, &channels) == 0, "E-AC-3 stream is readable");
		AudioStreamBasicDescription in = {0};
		in.mSampleRate = rate;
		in.mFormatID = kAudioFormatEnhancedAC3;
		in.mFramesPerPacket = 1536;
		in.mChannelsPerFrame = channels;
		AudioStreamBasicDescription out = pcm_float(rate, 2);
		AudioConverterRef conv = NULL;
		st = AudioConverterNew(&in, &out, &conv);
		CHECK(st == noErr, "E-AC-3 converter = %d", (int)st);
		if (st == noErr) {
			float *samples = NULL;
			long frames = decode_all(conv, &p, 2, &samples);
			check_tones("tones-eac3.eac3", samples, frames, 2, rate);
			free(samples);
			AudioConverterDispose(conv);
		}
		free(p.data);
		free(p.desc);
	}

	/* Opus */
	{
		AudioStreamBasicDescription in = {0}, out = pcm_float(48000, 2);
		in.mSampleRate = 48000;
		in.mFormatID = kAudioFormatOpus;
		in.mFramesPerPacket = 960;
		in.mChannelsPerFrame = 2;
		AudioConverterRef conv = NULL;
		st = AudioConverterNew(&in, &out, &conv);
		CHECK(st == noErr, "Opus converter = %d", (int)st);
		if (st == noErr)
			AudioConverterDispose(conv);
	}

	/* A decoder that needs its configuration (Apple Lossless) is created without it, and cannot convert
	 * until the cookie is set */
	{
		AudioStreamBasicDescription in = {0}, out = pcm_float(22050, 1);
		in.mSampleRate = 22050;
		in.mFormatID = kAudioFormatAppleLossless;
		in.mFramesPerPacket = 4096;
		in.mChannelsPerFrame = 1;
		AudioConverterRef conv = NULL;
		st = AudioConverterNew(&in, &out, &conv);
		CHECK(st == noErr, "Apple Lossless converter without a cookie = %d", (int)st);
		if (st == noErr) {
			float samples[64];
			AudioBufferList list;
			list.mNumberBuffers = 1;
			list.mBuffers[0].mNumberChannels = 1;
			list.mBuffers[0].mDataByteSize = sizeof samples;
			list.mBuffers[0].mData = samples;
			struct packets none = {0};
			UInt32 frames = 16;
			st = AudioConverterFillComplexBuffer(conv, feed, &none, &frames, &list, NULL);
			CHECK(st != noErr, "converting without the cookie fails (%d)", (int)st);
			const UInt8 junk[3] = { 1, 2, 3 };
			st = AudioConverterSetProperty(conv, kAudioConverterDecompressionMagicCookie, sizeof junk, junk);
			CHECK(st == kAudioConverterErr_FormatNotSupported, "a cookie the decoder rejects is a format error (%d)", (int)st);
			AudioConverterDispose(conv);
		}
	}

	/* Formats with no Core Audio decoder are refused */
	{
		AudioStreamBasicDescription in = {0}, out = pcm_float(44100, 2);
		in.mSampleRate = 44100;
		in.mFormatID = 'vorb';
		in.mChannelsPerFrame = 2;
		AudioConverterRef conv = NULL;
		st = AudioConverterNew(&in, &out, &conv);
		CHECK(st != noErr && conv == NULL, "an unknown compressed format is refused (%d)", (int)st);
	}

	if (failures) {
		printf("RESULT FAIL (%d)\n", failures);
		return 1;
	}
	printf("RESULT PASS\n");
	return 0;
}
