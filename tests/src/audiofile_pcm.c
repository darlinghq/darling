// Copyright (C) 2026 Lubos Dolezel
//
// AudioFile and ExtAudioFile with linear PCM in WAVE, AIFF, AIFF-C and CAF files, which are
// read and written without any codec library. The fixtures in tests/coreaudio/pcm-fixtures were
// made by afconvert on macOS (see the README there): this program reads them, and writes the
// same audio to compare the files byte for byte.
//
// Usage: audiofile_pcm <fixtures directory>
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
static const char* g_fixtures;
static char g_dir[256];

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long a_ = (a), b_ = (b); if (a_ != b_) { printf("FAIL %s:%d: %s = %lld (0x%llx), expected %lld (0x%llx)\n", __FILE__, __LINE__, #a, a_, a_, b_, b_); g_failures++; } } while (0)

#define FRAMES 1000

static CFURLRef urlFor(const char* path)
{
	return CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), false);
}

static void fixturePath(char* out, const char* name)
{
	snprintf(out, 512, "%s/%s", g_fixtures, name);
}

static void tempPath(char* out, const char* name)
{
	snprintf(out, 512, "%s/%s", g_dir, name);
}

// The audio of every fixture is this, before it is converted to the format of the file
static short value16(int channel, int frame)
{
	if (channel == 0)
		return (short) (frame * 60 - 30000);
	if (channel == 1)
		return (short) (15000 - frame * 30);
	return (short) (frame * (channel + 1) * 7 - 2000);
}

static AudioStreamBasicDescription fmt(double rate, int channels, int bits, int isFloat, int isSigned, int bigEndian)
{
	AudioStreamBasicDescription d;

	memset(&d, 0, sizeof(d));
	d.mSampleRate = rate;
	d.mFormatID = kAudioFormatLinearPCM;
	d.mFormatFlags = (isFloat ? kAudioFormatFlagIsFloat : (isSigned ? kAudioFormatFlagIsSignedInteger : 0))
		| (bigEndian ? kAudioFormatFlagIsBigEndian : 0) | kAudioFormatFlagIsPacked;
	d.mChannelsPerFrame = channels;
	d.mBitsPerChannel = bits;
	d.mFramesPerPacket = 1;
	d.mBytesPerFrame = d.mBytesPerPacket = channels * bits / 8;
	return d;
}

// The audio in the byte layout of 'f'
static unsigned char* makeAudio(const AudioStreamBasicDescription* f, int frames)
{
	const int bytes = f->mBitsPerChannel / 8;
	unsigned char* out = malloc((size_t) frames * f->mBytesPerFrame);
	const int big = (f->mFormatFlags & kAudioFormatFlagIsBigEndian) != 0;

	for (int i = 0; i < frames; i++)
	{
		for (UInt32 c = 0; c < f->mChannelsPerFrame; c++)
		{
			const short v = value16(c, i);
			unsigned char* p = out + ((size_t) i * f->mChannelsPerFrame + c) * bytes;
			unsigned char tmp[8];

			if (f->mFormatFlags & kAudioFormatFlagIsFloat)
			{
				if (bytes == 4)
				{
					float x = v / 32768.0f;
					memcpy(tmp, &x, 4);
				}
				else
				{
					double x = v / 32768.0;
					memcpy(tmp, &x, 8);
				}
				for (int b = 0; b < bytes; b++)
					p[b] = tmp[b];		// the host is little-endian
				if (big)
					for (int b = 0; b < bytes / 2; b++)
					{
						unsigned char t = p[b];
						p[b] = p[bytes - 1 - b];
						p[bytes - 1 - b] = t;
					}
			}
			else
			{
				long long x;

				if (bytes == 1)
					x = (f->mFormatFlags & kAudioFormatFlagIsSignedInteger) ? (v >> 8) : ((v >> 8) + 128);
				else
					x = (long long) v * (1LL << (8 * (bytes - 2)));
				for (int b = 0; b < bytes; b++)
					p[big ? bytes - 1 - b : b] = (unsigned char) (x >> (8 * b));
			}
		}
	}
	return out;
}

static unsigned char* readFile(const char* path, long* size)
{
	FILE* f = fopen(path, "rb");
	unsigned char* data;

	if (!f)
	{
		printf("FAIL cannot open %s\n", path);
		g_failures++;
		*size = 0;
		return calloc(1, 1);
	}
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(*size + 1);
	if (fread(data, 1, *size, f) != (size_t) *size)
		*size = 0;
	fclose(f);
	return data;
}

static void writeFile(const char* path, const void* data, size_t size)
{
	FILE* f = fopen(path, "wb");

	if (!f || fwrite(data, 1, size, f) != size)
	{
		printf("FAIL cannot write %s\n", path);
		g_failures++;
	}
	if (f)
		fclose(f);
}

static UInt32 be32(const unsigned char* p) { return (UInt32) p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static UInt32 le32(const unsigned char* p) { return (UInt32) p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0]; }

static void describe(const AudioStreamBasicDescription* d, char* out)
{
	sprintf(out, "rate %g flags %x bpp %u fpp %u bpf %u ch %u bits %u", d->mSampleRate, (unsigned) d->mFormatFlags, (unsigned) d->mBytesPerPacket,
		(unsigned) d->mFramesPerPacket, (unsigned) d->mBytesPerFrame, (unsigned) d->mChannelsPerFrame, (unsigned) d->mBitsPerChannel);
}

struct expect
{
	const char* name;
	AudioFileTypeID type;
	AudioStreamBasicDescription format;
	int frames;
	AudioChannelLayoutTag layout;
};

static OSStatus getProp(AudioFileID f, AudioFilePropertyID id, void* out, UInt32 size)
{
	return AudioFileGetProperty(f, id, &size, out);
}

static void checkProperties(AudioFileID f, const struct expect* e, const char* what)
{
	AudioStreamBasicDescription d;
	UInt64 u64 = 0;
	SInt64 s64 = 0;
	UInt32 u32 = 0, size;
	Float64 duration = 0;
	AudioChannelLayout layout;
	char got[200], want[200];

	CHECK_EQ(getProp(f, kAudioFilePropertyDataFormat, &d, sizeof(d)), noErr);
	describe(&d, got);
	describe(&e->format, want);
	if (strcmp(got, want) != 0 || d.mFormatID != kAudioFormatLinearPCM)
	{
		printf("FAIL %s: data format [%s], expected [%s]\n", what, got, want);
		g_failures++;
	}

	CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataByteCount, &u64, 8), noErr);
	CHECK_EQ(u64, (long long) e->frames * e->format.mBytesPerFrame);
	CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &u64, 8), noErr);
	CHECK_EQ(u64, e->frames);
	CHECK_EQ(getProp(f, kAudioFilePropertyDataOffset, &s64, 8), noErr);
	CHECK_EQ(s64, 4096);
	CHECK_EQ(getProp(f, kAudioFilePropertyMaximumPacketSize, &u32, 4), noErr);
	CHECK_EQ(u32, e->format.mBytesPerFrame);
	CHECK_EQ(getProp(f, kAudioFilePropertyPacketSizeUpperBound, &u32, 4), noErr);
	CHECK_EQ(u32, e->format.mBytesPerFrame);
	CHECK_EQ(getProp(f, kAudioFilePropertyBitRate, &u32, 4), noErr);
	CHECK_EQ(u32, (long long) (e->format.mSampleRate * e->format.mBytesPerFrame * 8));
	CHECK_EQ(getProp(f, kAudioFilePropertyIsOptimized, &u32, 4), noErr);
	CHECK_EQ(u32, 1);
	CHECK_EQ(getProp(f, kAudioFilePropertyFileFormat, &u32, 4), noErr);
	CHECK_EQ(u32, e->type);
	CHECK_EQ(getProp(f, kAudioFilePropertyEstimatedDuration, &duration, 8), noErr);
	CHECK(fabs(duration - e->frames / e->format.mSampleRate) < 1e-9);

	// A magic cookie exists for compressed audio only
	size = 0;
	CHECK_EQ(AudioFileGetPropertyInfo(f, kAudioFilePropertyMagicCookieData, &size, NULL), kAudioFileUnsupportedPropertyError);

	size = sizeof(layout);
	if (e->layout)
	{
		AudioChannelLayout* l = malloc(64);

		size = 64;
		CHECK_EQ(AudioFileGetPropertyInfo(f, kAudioFilePropertyChannelLayout, &size, NULL), noErr);
		CHECK(size >= 12);
		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyChannelLayout, &size, l), noErr);
		CHECK_EQ(l->mChannelLayoutTag, e->layout);
		free(l);
	}
	else
	{
		// A file without a layout answers with a bad size
		UInt32 info = 99;

		CHECK_EQ(AudioFileGetPropertyInfo(f, kAudioFilePropertyChannelLayout, &info, NULL), kAudioFileBadPropertySizeError);
		CHECK_EQ(info, 0);
	}
}

// The packets and bytes of the file are the audio
static void checkAudio(AudioFileID f, const struct expect* e)
{
	const UInt32 bpf = e->format.mBytesPerFrame;
	unsigned char* want = makeAudio(&e->format, e->frames);
	unsigned char* got = malloc((size_t) e->frames * bpf + 16);
	UInt32 bytes = e->frames * bpf, packets;

	CHECK_EQ(AudioFileReadBytes(f, false, 0, &bytes, got), noErr);
	CHECK_EQ(bytes, e->frames * bpf);
	CHECK(memcmp(got, want, e->frames * bpf) == 0);

	memset(got, 0, (size_t) e->frames * bpf);
	packets = e->frames;
	bytes = e->frames * bpf;
	CHECK_EQ(AudioFileReadPacketData(f, false, &bytes, NULL, 0, &packets, got), noErr);
	CHECK_EQ(packets, e->frames);
	CHECK_EQ(bytes, e->frames * bpf);
	CHECK(memcmp(got, want, e->frames * bpf) == 0);

	// A read from the middle, by packets
	memset(got, 0, (size_t) e->frames * bpf);
	packets = 10;
	CHECK_EQ(AudioFileReadPackets(f, false, &bytes, NULL, e->frames / 2, &packets, got), noErr);
	CHECK_EQ(packets, 10);
	CHECK_EQ(bytes, 10 * bpf);
	CHECK(memcmp(got, want + (e->frames / 2) * bpf, 10 * bpf) == 0);

	// Only what is there is returned at the end, and nothing past it
	packets = 50;
	bytes = 50 * bpf;
	CHECK_EQ(AudioFileReadPacketData(f, false, &bytes, NULL, e->frames - 20, &packets, got), noErr);
	CHECK_EQ(packets, 20);
	CHECK(memcmp(got, want + (e->frames - 20) * bpf, 20 * bpf) == 0);

	packets = 5;
	bytes = 5 * bpf;
	CHECK_EQ(AudioFileReadPacketData(f, false, &bytes, NULL, e->frames + 10, &packets, got), noErr);
	CHECK_EQ(packets, 0);

	packets = 0;
	bytes = 100;
	CHECK_EQ(AudioFileReadPacketData(f, false, &bytes, NULL, 0, &packets, got), kAudio_ParamError);

	// Bytes can start anywhere
	bytes = 7;
	CHECK_EQ(AudioFileReadBytes(f, false, 5, &bytes, got), noErr);
	CHECK(memcmp(got, want + 5, 7) == 0);

	free(got);
	free(want);
}

static const struct expect FIXTURES[] = {
	{ "pcm16.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 }, FRAMES, 0 },
	{ "pcm24.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 }, FRAMES, 0 },
	{ "pcm32-mono.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 4, 1, 4, 1, 32, 0 }, FRAMES, 0 },
	{ "float32.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 }, FRAMES, 0 },
	{ "float64.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 1, 64, 0 }, FRAMES, 0 },
	{ "pcm8.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsPacked, 2, 1, 2, 2, 8, 0 }, FRAMES, 0 },
	{ "stereo-layout.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 }, FRAMES, kAudioChannelLayoutTag_Stereo },
	{ "pcm16.aiff", kAudioFileAIFFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 }, FRAMES, 0 },
	{ "pcm24.aiff", kAudioFileAIFFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 }, FRAMES, 0 },
	{ "pcm8-mono.aiff", kAudioFileAIFFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 1, 1, 1, 1, 8, 0 }, FRAMES, 0 },
	{ "twos16.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 }, FRAMES, 0 },
	{ "in24.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 }, FRAMES, 0 },
	{ "in32.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 }, FRAMES, 0 },
	{ "int8.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 1, 1, 1, 1, 8, 0 }, FRAMES, 0 },
	{ "fl32.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 }, FRAMES, 0 },
	{ "fl64.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 16, 1, 16, 2, 64, 0 }, FRAMES, 0 },
	{ "float32.caf", kAudioFileCAFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 }, FRAMES, 0 },
	{ "pcm24be.caf", kAudioFileCAFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 }, FRAMES, 0 },
	{ "stereo-layout.caf", kAudioFileCAFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 }, FRAMES, kAudioChannelLayoutTag_Stereo },
};

static void test_fixtures(void)
{
	for (size_t i = 0; i < sizeof(FIXTURES) / sizeof(FIXTURES[0]); i++)
	{
		const struct expect* e = &FIXTURES[i];
		char path[512];
		AudioFileID f = NULL;
		CFURLRef url;

		fixturePath(path, e->name);
		url = urlFor(path);

		// The file type is found from the contents
		OSStatus status = AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f);
		if (status != noErr)
		{
			printf("FAIL open %s = %d\n", e->name, (int) status);
			g_failures++;
			CFRelease(url);
			continue;
		}
		checkProperties(f, e, e->name);
		checkAudio(f, e);
		AudioFileClose(f);
		CFRelease(url);
	}
}

// The six-channel fixtures have 100 frames and no 4096-byte frames of audio like the others
static void test_six_channels(void)
{
	static const struct { const char* name; AudioFileTypeID type; int big; AudioChannelLayoutTag layout; } FILES[] = {
		{ "pcm16-6ch.wav", kAudioFileWAVEType, 0, 0 },
		{ "pcm16-6ch.caf", kAudioFileCAFType, 0, 0 },
		{ "layout51.wav", kAudioFileWAVEType, 0, kAudioChannelLayoutTag_MPEG_5_1_A },
		{ "layout51.caf", kAudioFileCAFType, 0, kAudioChannelLayoutTag_MPEG_5_1_A },
		{ "layout51.aiff", kAudioFileAIFFType, 1, kAudioChannelLayoutTag_MPEG_5_1_A },
	};

	for (size_t i = 0; i < sizeof(FILES) / sizeof(FILES[0]); i++)
	{
		struct expect e = { FILES[i].name, FILES[i].type, fmt(44100, 6, 16, 0, 1, FILES[i].big), 100, FILES[i].layout };
		char path[512];
		AudioFileID f;
		CFURLRef url;

		fixturePath(path, FILES[i].name);
		url = urlFor(path);
		CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
		checkProperties(f, &e, FILES[i].name);
		checkAudio(f, &e);
		AudioFileClose(f);
		CFRelease(url);
	}
}

// afconvert adds an 'info' chunk to CAF files (the duration and the source bit depth), which AudioFile does not
// write, so for those the description chunks and the audio are compared
static int sameFile(const struct expect* e, const unsigned char* want, long wantSize, const unsigned char* got, long gotSize, long* at)
{
	*at = 0;
	if (wantSize != gotSize)
		return 0;

	if (e->type == kAudioFileCAFType)
	{
		const long described = e->layout ? 52 + 24 : 52;
		const long dataChunk = 4096 - 16;

		if (memcmp(want, got, described) != 0)
			return *at = 0, 0;
		if (memcmp(want + dataChunk, got + dataChunk, wantSize - dataChunk) != 0)
			return *at = dataChunk, 0;
		return 1;
	}

	while (*at < wantSize && want[*at] == got[*at])
		(*at)++;
	return *at == wantSize;
}

// Writes the fixture's audio into a new file of the same type and compares the files
static void test_written_files_match_fixtures(void)
{
	for (size_t i = 0; i < sizeof(FIXTURES) / sizeof(FIXTURES[0]); i++)
	{
		const struct expect* e = &FIXTURES[i];
		char path[512], mine[512];
		AudioFileID f;
		CFURLRef url;
		unsigned char *want, *got;
		long wantSize, gotSize;
		unsigned char* audio = makeAudio(&e->format, e->frames);
		UInt32 packets = e->frames;
		OSStatus status;

		fixturePath(path, e->name);
		tempPath(mine, e->name);
		unlink(mine);
		url = urlFor(mine);

		status = AudioFileCreateWithURL(url, e->type, &e->format, 0, &f);
		if (status != noErr)
		{
			printf("FAIL create %s = %d\n", e->name, (int) status);
			g_failures++;
			CFRelease(url);
			free(audio);
			continue;
		}

		if (e->layout)
		{
			AudioChannelLayout layout;

			memset(&layout, 0, sizeof(layout));
			layout.mChannelLayoutTag = e->layout;
			CHECK_EQ(AudioFileSetProperty(f, kAudioFilePropertyChannelLayout, 12, &layout), noErr);
		}

		// A new file has a header and no audio
		{
			UInt64 count = 99;

			CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &count, 8), noErr);
			CHECK_EQ(count, 0);
		}

		// In two writes
		packets = 300;
		CHECK_EQ(AudioFileWritePackets(f, false, 300 * e->format.mBytesPerFrame, NULL, 0, &packets, audio), noErr);
		CHECK_EQ(packets, 300);
		packets = e->frames - 300;
		CHECK_EQ(AudioFileWritePackets(f, false, packets * e->format.mBytesPerFrame, NULL, 300, &packets, audio + 300 * e->format.mBytesPerFrame), noErr);
		CHECK_EQ(packets, e->frames - 300);
		checkProperties(f, e, e->name);
		CHECK_EQ(AudioFileClose(f), noErr);

		want = readFile(path, &wantSize);
		got = readFile(mine, &gotSize);
		{
			long at;

			if (!sameFile(e, want, wantSize, got, gotSize, &at))
			{
				printf("FAIL %s differs from the file made on macOS: sizes %ld and %ld, first difference at %ld\n", e->name, gotSize, wantSize, at);
				g_failures++;
			}
		}

		// And it reads back
		CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
		checkProperties(f, e, e->name);
		checkAudio(f, e);
		AudioFileClose(f);

		free(want);
		free(got);
		free(audio);
		unlink(mine);
		CFRelease(url);
	}
}

static void test_unsupported_formats(void)
{
	char path[512];
	AudioFileID f = NULL;
	CFURLRef url;

	tempPath(path, "unsupported");
	url = urlFor(path);

	// What the containers cannot hold
	AudioStreamBasicDescription wave16be = fmt(44100, 2, 16, 0, 1, 1);
	AudioStreamBasicDescription waveU16 = fmt(44100, 2, 16, 0, 0, 0);
	AudioStreamBasicDescription aiffLE = fmt(44100, 2, 16, 0, 1, 0);
	AudioStreamBasicDescription aiffFloat = fmt(44100, 2, 32, 1, 0, 1);
	AudioStreamBasicDescription aifcLE24 = fmt(44100, 2, 24, 0, 1, 0);
	AudioStreamBasicDescription bits12 = fmt(44100, 2, 16, 0, 1, 0);
	AudioStreamBasicDescription nonInterleaved = fmt(44100, 2, 16, 0, 1, 0);

	bits12.mBitsPerChannel = 12;
	nonInterleaved.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
	nonInterleaved.mBytesPerFrame = nonInterleaved.mBytesPerPacket = 2;

	unlink(path);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileWAVEType, &wave16be, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileWAVEType, &waveU16, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileAIFFType, &aiffLE, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileAIFFType, &aiffFloat, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileAIFCType, &aifcLE24, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileCAFType, &bits12, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileCAFType, &nonInterleaved, 0, &f), kAudioFileUnsupportedDataFormatError);
	CHECK(access(path, F_OK) != 0);
	CFRelease(url);
}

static void test_writing_rules(void)
{
	char path[512];
	AudioFileID f;
	CFURLRef url;
	AudioStreamBasicDescription d = fmt(44100, 2, 16, 0, 1, 0);
	short audio[20];
	UInt32 packets, bytes;
	short back[20];

	for (int i = 0; i < 20; i++)
		audio[i] = (short) (i + 1);

	tempPath(path, "rules.wav");
	unlink(path);
	url = urlFor(path);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileWAVEType, &d, 0, &f), noErr);

	packets = 5;
	CHECK_EQ(AudioFileWritePackets(f, false, 20, NULL, 0, &packets, audio), noErr);

	// There can be no gap
	packets = 1;
	CHECK_EQ(AudioFileWritePackets(f, false, 4, NULL, 9, &packets, audio), kAudioFileInvalidPacketOffsetError);

	// Packets in the file are overwritten in place, and the file does not grow
	packets = 1;
	audio[0] = 77;
	audio[1] = 78;
	CHECK_EQ(AudioFileWritePackets(f, false, 4, NULL, 2, &packets, audio), noErr);
	bytes = 20;
	CHECK_EQ(AudioFileReadBytes(f, false, 0, &bytes, back), noErr);
	CHECK(back[4] == 77 && back[5] == 78 && back[6] == 7);

	{
		UInt64 count;

		CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &count, 8), noErr);
		CHECK_EQ(count, 5);
	}

	// Writing bytes extends the audio like packets do
	bytes = 8;
	CHECK_EQ(AudioFileWriteBytes(f, false, 20, &bytes, audio + 2), noErr);
	{
		UInt64 count;

		CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &count, 8), noErr);
		CHECK_EQ(count, 7);
	}
	AudioFileClose(f);

	// The sizes in the header were updated
	{
		long size;
		unsigned char* file = readFile(path, &size);

		CHECK_EQ(size, 4096 + 28);
		CHECK_EQ(le32(file + 4), size - 8);
		CHECK_EQ(le32(file + 4096 - 4), 28);
		free(file);
	}

	// Without the page alignment the audio follows the header
	unlink(path);
	CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileWAVEType, &d, kAudioFileFlags_DontPageAlignAudioData, &f), noErr);
	packets = 5;
	CHECK_EQ(AudioFileWritePackets(f, false, 20, NULL, 0, &packets, audio), noErr);
	{
		SInt64 offset;

		CHECK_EQ(getProp(f, kAudioFilePropertyDataOffset, &offset, 8), noErr);
		CHECK_EQ(offset, 44);
	}
	AudioFileClose(f);
	{
		long size;
		unsigned char* file = readFile(path, &size);

		CHECK_EQ(size, 44 + 20);
		CHECK(memcmp(file, "RIFF", 4) == 0 && le32(file + 4) == 56 && memcmp(file + 8, "WAVEfmt ", 8) == 0 && le32(file + 16) == 16);
		CHECK(memcmp(file + 36, "data", 4) == 0 && le32(file + 40) == 20);
		CHECK(memcmp(file + 44, audio, 20) == 0);
		free(file);
	}

	// An existing file is not replaced
	{
		AudioFileID again;

		CHECK(AudioFileCreateWithURL(url, kAudioFileWAVEType, &d, 0, &again) != noErr);
		CHECK_EQ(AudioFileCreateWithURL(url, kAudioFileWAVEType, &d, kAudioFileFlags_EraseFile, &again), noErr);
		AudioFileClose(again);
	}
	unlink(path);
	CFRelease(url);
}

static void test_channel_layouts(void)
{
	static const struct { const char* name; AudioFileTypeID type; int big; } FILES[] = {
		{ "layout.wav", kAudioFileWAVEType, 0 }, { "layout.aiff", kAudioFileAIFFType, 1 }, { "layout.caf", kAudioFileCAFType, 0 },
	};

	for (size_t i = 0; i < 3; i++)
	{
		char path[512];
		AudioFileID f;
		CFURLRef url;
		AudioStreamBasicDescription d = fmt(48000, 6, 16, 0, 1, FILES[i].big);
		AudioChannelLayout layout, got;
		short frame[6] = { 1, 2, 3, 4, 5, 6 };
		UInt32 packets = 1, size = sizeof(got);

		tempPath(path, FILES[i].name);
		unlink(path);
		url = urlFor(path);
		CHECK_EQ(AudioFileCreateWithURL(url, FILES[i].type, &d, 0, &f), noErr);

		memset(&layout, 0, sizeof(layout));
		layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_5_1_A;
		CHECK_EQ(AudioFileSetProperty(f, kAudioFilePropertyChannelLayout, sizeof(layout), &layout), noErr);
		CHECK_EQ(AudioFileWritePackets(f, false, 12, NULL, 0, &packets, frame), noErr);
		AudioFileClose(f);

		CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
		memset(&got, 0, sizeof(got));
		CHECK_EQ(AudioFileGetPropertyInfo(f, kAudioFilePropertyChannelLayout, &size, NULL), noErr);
		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyChannelLayout, &size, &got), noErr);
		CHECK_EQ(got.mChannelLayoutTag, kAudioChannelLayoutTag_MPEG_5_1_A);
		AudioFileClose(f);

		if (FILES[i].type == kAudioFileWAVEType)
		{
			// The speaker mask of WAVE_FORMAT_EXTENSIBLE
			long fileSize;
			unsigned char* file = readFile(path, &fileSize);

			CHECK_EQ(le32(file + 16), 40);
			CHECK(file[20] == 0xfe && file[21] == 0xff);
			CHECK_EQ(le32(file + 20 + 20), 0x60f);
			CHECK_EQ(file[20 + 24], 1);
			free(file);
		}

		unlink(path);
		CFRelease(url);
	}
}

static void put32be(unsigned char* p, UInt32 v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

// Files built by hand from the specifications, for things the tools do not write
static void test_handmade_files(void)
{
	char path[512];
	AudioFileID f;
	CFURLRef url;
	UInt32 size = sizeof(AudioStreamBasicDescription);
	AudioStreamBasicDescription d;

	// AIFF-C with 'sowt': little-endian 16-bit samples
	tempPath(path, "sowt.aifc");
	{
		unsigned char file[12 + 12 + 8 + 22 + 8 + 8 + 8];
		unsigned char* p = file;
		const unsigned char rate44100[10] = { 0x40, 0x0e, 0xac, 0x44, 0, 0, 0, 0, 0, 0 };
		const short audio[4] = { 100, -100, 3000, -3000 };

		memcpy(p, "FORM", 4); put32be(p + 4, sizeof(file) - 8); memcpy(p + 8, "AIFC", 4); p += 12;
		memcpy(p, "FVER", 4); put32be(p + 4, 4); put32be(p + 8, 0xA2805140); p += 12;
		memcpy(p, "COMM", 4); put32be(p + 4, 22); p += 8;
		p[0] = 0; p[1] = 2; put32be(p + 2, 2); p[6] = 0; p[7] = 16; memcpy(p + 8, rate44100, 10); memcpy(p + 18, "sowt", 4); p += 22;
		memcpy(p, "SSND", 4); put32be(p + 4, 8 + 8); put32be(p + 8, 0); put32be(p + 12, 0); memcpy(p + 16, audio, 8);

		writeFile(path, file, sizeof(file));
		url = urlFor(path);
		CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyDataFormat, &size, &d), noErr);
		CHECK_EQ(d.mFormatFlags, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked);
		CHECK(d.mBitsPerChannel == 16 && d.mChannelsPerFrame == 2 && d.mSampleRate == 44100);
		{
			short got[4];
			UInt32 bytes = 8, packets = 2;

			CHECK_EQ(AudioFileReadPacketData(f, false, &bytes, NULL, 0, &packets, got), noErr);
			CHECK_EQ(packets, 2);
			CHECK(memcmp(got, audio, 8) == 0);
		}
		AudioFileClose(f);
		CFRelease(url);
		unlink(path);
	}

	// A WAVE file with another chunk before 'fmt ', an odd-sized chunk and a data size that is larger than the file
	{
		unsigned char file[12 + 8 + 3 + 1 + 8 + 16 + 8 + 8];
		unsigned char* p = file;
		const short audio[4] = { 5, 6, 7, 8 };

		memcpy(p, "RIFF", 4); p[4] = 0xff; p[5] = 0xff; p[6] = 0xff; p[7] = 0xff; memcpy(p + 8, "WAVE", 4); p += 12;
		memcpy(p, "LIST", 4); p[4] = 3; p[5] = p[6] = p[7] = 0; p[8] = 'a'; p[9] = 'b'; p[10] = 'c'; p[11] = 0; p += 12;
		memcpy(p, "fmt ", 4); p[4] = 16; p[5] = p[6] = p[7] = 0;
		p[8] = 1; p[9] = 0; p[10] = 1; p[11] = 0; p[12] = 0x40; p[13] = 0x1f; p[14] = 0; p[15] = 0; p[16] = 0x80; p[17] = 0x3e; p[18] = 0; p[19] = 0;
		p[20] = 2; p[21] = 0; p[22] = 16; p[23] = 0; p += 24;
		memcpy(p, "data", 4); p[4] = 0xff; p[5] = 0xff; p[6] = 0xff; p[7] = 0x7f; memcpy(p + 8, audio, 8);

		writeFile(path, file, sizeof(file));
		url = urlFor(path);
		CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyDataFormat, &size, &d), noErr);
		CHECK(d.mSampleRate == 8000 && d.mChannelsPerFrame == 1 && d.mBitsPerChannel == 16);
		{
			UInt64 count;

			CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &count, 8), noErr);
			CHECK_EQ(count, 4);
		}
		AudioFileClose(f);
		CFRelease(url);
		unlink(path);
	}

	// A CAF file whose data chunk size is not known yet (-1) extends to the end of the file
	{
		unsigned char file[8 + 12 + 32 + 12 + 4 + 8];
		unsigned char* p = file;
		const short audio[4] = { 1, 2, 3, 4 };

		memcpy(p, "caff", 4); p[4] = 0; p[5] = 1; p[6] = p[7] = 0; p += 8;
		memcpy(p, "desc", 4); memset(p + 4, 0, 7); p[11] = 32; p += 12;
		{
			const double rate = 8000;
			unsigned long long bits;

			memcpy(&bits, &rate, 8);
			for (int i = 0; i < 8; i++)
				p[i] = (unsigned char) (bits >> (56 - 8 * i));
		}
		memcpy(p + 8, "lpcm", 4); put32be(p + 12, 2); put32be(p + 16, 2); put32be(p + 20, 1); put32be(p + 24, 1); put32be(p + 28, 16); p += 32;
		memcpy(p, "data", 4); memset(p + 4, 0xff, 8); p += 12;
		put32be(p, 1); memcpy(p + 4, audio, 8);

		writeFile(path, file, sizeof(file));
		url = urlFor(path);
		CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
		{
			UInt64 count;

			CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &count, 8), noErr);
			CHECK_EQ(count, 4);
		}
		AudioFileClose(f);
		CFRelease(url);
		unlink(path);
	}

	// Damaged files are not opened
	{
		const unsigned char riffOnly[12] = { 'R', 'I', 'F', 'F', 4, 0, 0, 0, 'W', 'A', 'V', 'E' };
		const unsigned char junk[64] = { 1, 2, 3 };

		writeFile(path, riffOnly, sizeof(riffOnly));
		url = urlFor(path);
		CHECK(AudioFileOpenURL(url, kAudioFileReadPermission, kAudioFileWAVEType, &f) != noErr);
		CFRelease(url);

		writeFile(path, junk, sizeof(junk));
		url = urlFor(path);
		CHECK(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f) != noErr);
		CFRelease(url);
		unlink(path);
	}
}

struct memfile
{
	unsigned char* data;
	size_t size, capacity;
};

static OSStatus memRead(void* ud, SInt64 pos, UInt32 request, void* buffer, UInt32* actual)
{
	struct memfile* m = ud;
	size_t n = (pos >= (SInt64) m->size) ? 0 : (size_t) (m->size - pos);

	if (n > request)
		n = request;
	memcpy(buffer, m->data + pos, n);
	*actual = (UInt32) n;
	return noErr;
}

static OSStatus memWrite(void* ud, SInt64 pos, UInt32 request, const void* buffer, UInt32* actual)
{
	struct memfile* m = ud;

	if ((size_t) pos + request > m->capacity)
		return kAudioFileUnspecifiedError;
	memcpy(m->data + pos, buffer, request);
	if ((size_t) pos + request > m->size)
		m->size = (size_t) pos + request;
	*actual = request;
	return noErr;
}

static SInt64 memGetSize(void* ud) { return (SInt64) ((struct memfile*) ud)->size; }

static OSStatus memSetSize(void* ud, SInt64 size)
{
	struct memfile* m = ud;

	if ((size_t) size > m->capacity)
		return kAudioFileUnspecifiedError;
	m->size = (size_t) size;
	return noErr;
}

static void test_callbacks(void)
{
	struct memfile m = { malloc(1 << 16), 0, 1 << 16 };
	AudioStreamBasicDescription d = fmt(44100, 2, 16, 0, 1, 1);
	AudioFileID f;
	unsigned char* audio = makeAudio(&d, 100);
	UInt32 packets = 100;
	long size;
	char path[512];
	unsigned char* fixture;

	CHECK_EQ(AudioFileInitializeWithCallbacks(&m, memRead, memWrite, memGetSize, memSetSize, kAudioFileAIFFType, &d, 0, &f), noErr);
	CHECK_EQ(AudioFileWritePackets(f, false, 400, NULL, 0, &packets, audio), noErr);
	AudioFileClose(f);

	CHECK_EQ(m.size, 4096 + 400 + 0);
	CHECK(memcmp(m.data, "FORM", 4) == 0 && memcmp(m.data + 8, "AIFF", 4) == 0);
	CHECK_EQ(be32(m.data + 4), m.size - 8);
	CHECK(memcmp(m.data + m.size - 400, audio, 400) == 0);

	// And read it back from memory
	CHECK_EQ(AudioFileOpenWithCallbacks(&m, memRead, NULL, memGetSize, NULL, 0, &f), noErr);
	{
		AudioStreamBasicDescription got;
		UInt32 sz = sizeof(got);
		UInt32 bytes = 400;
		unsigned char back[400];
		UInt32 type;

		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyDataFormat, &sz, &got), noErr);
		CHECK_EQ(got.mFormatFlags, d.mFormatFlags);
		sz = 4;
		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyFileFormat, &sz, &type), noErr);
		CHECK_EQ(type, kAudioFileAIFFType);
		CHECK_EQ(AudioFileReadBytes(f, false, 0, &bytes, back), noErr);
		CHECK(memcmp(back, audio, 400) == 0);
	}
	AudioFileClose(f);

	// A fixture read from memory
	fixturePath(path, "pcm16.wav");
	fixture = readFile(path, &size);
	free(m.data);
	m.data = fixture;
	m.size = (size_t) size;
	CHECK_EQ(AudioFileOpenWithCallbacks(&m, memRead, NULL, memGetSize, NULL, kAudioFileWAVEType, &f), noErr);
	{
		UInt64 count;

		CHECK_EQ(getProp(f, kAudioFilePropertyAudioDataPacketCount, &count, 8), noErr);
		CHECK_EQ(count, FRAMES);
	}
	AudioFileClose(f);
	free(fixture);
	free(audio);
}

static void test_type_detection(void)
{
	char path[512], copy[512];
	AudioFileID f;
	CFURLRef url;
	long size;
	unsigned char* data;
	UInt32 type, sz = 4;

	// The extension is only a hint, and the contents decide
	fixturePath(path, "pcm16.aiff");
	tempPath(copy, "looks-like.wav");
	data = readFile(path, &size);
	writeFile(copy, data, size);
	free(data);
	url = urlFor(copy);
	CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
	CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyFileFormat, &sz, &type), noErr);
	CHECK_EQ(type, kAudioFileAIFFType);
	AudioFileClose(f);
	CFRelease(url);
	unlink(copy);

	// The types are listed as readable and writable
	{
		UInt32 bytes = 0;
		UInt32* types;
		int wave = 0, aiff = 0, aifc = 0, caf = 0;

		CHECK_EQ(AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_WritableTypes, 0, NULL, &bytes), noErr);
		types = malloc(bytes);
		CHECK_EQ(AudioFileGetGlobalInfo(kAudioFileGlobalInfo_WritableTypes, 0, NULL, &bytes, types), noErr);
		for (UInt32 i = 0; i < bytes / 4; i++)
		{
			wave |= types[i] == kAudioFileWAVEType;
			aiff |= types[i] == kAudioFileAIFFType;
			aifc |= types[i] == kAudioFileAIFCType;
			caf |= types[i] == kAudioFileCAFType;
		}
		CHECK(wave && aiff && aifc && caf);
		free(types);
	}
}

static OSStatus readAll(ExtAudioFileRef ref, UInt32 channels, UInt32 bytesPerFrame, void* out, UInt32 frames, UInt32* got)
{
	AudioBufferList list;
	UInt32 n = frames;
	OSStatus status;

	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = channels;
	list.mBuffers[0].mData = out;
	list.mBuffers[0].mDataByteSize = frames * bytesPerFrame;
	status = ExtAudioFileRead(ref, &n, &list);
	*got = n;
	return status;
}

// Companded audio is not linear PCM and is left to libavformat and libavcodec, when the host has them
static void test_companded_fallback(void)
{
	char path[512];
	AudioFileID f;
	CFURLRef url;
	OSStatus status;

	fixturePath(path, "ulaw.wav");
	url = urlFor(path);
	status = AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f);

	if (status != noErr)
	{
		// Without FFmpeg the file is not of a type that can be read
		CHECK(status == kAudioFileUnsupportedFileTypeError || status == kAudioFileUnsupportedDataFormatError);
		printf("note: no FFmpeg, u-law WAVE not opened (%d)\n", (int) status);
	}
	else
	{
		AudioStreamBasicDescription d;
		UInt32 size = sizeof(d);
		ExtAudioFileRef ref;
		AudioStreamBasicDescription client = fmt(44100, 2, 32, 1, 0, 0);
		float samples[2 * FRAMES];
		UInt32 got;
		float peak = 0;

		CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyDataFormat, &size, &d), noErr);
		CHECK_EQ(d.mFormatID, kAudioFormatULaw);
		CHECK_EQ(d.mChannelsPerFrame, 2);
		AudioFileClose(f);

		CHECK_EQ(ExtAudioFileOpenURL(url, &ref), noErr);
		CHECK_EQ(ExtAudioFileSetProperty(ref, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client), noErr);
		CHECK_EQ(readAll(ref, 2, 8, samples, FRAMES, &got), noErr);
		CHECK_EQ(got, FRAMES);
		for (UInt32 i = 0; i < got * 2; i++)
			if (fabsf(samples[i]) > peak)
				peak = fabsf(samples[i]);
		CHECK(peak > 0.88 && peak < 0.95);
		ExtAudioFileDispose(ref);
	}
	CFRelease(url);
}

static void test_ext_audio_file(void)
{
	char path[512], mine[512];
	ExtAudioFileRef ref;
	CFURLRef url;
	AudioStreamBasicDescription client = fmt(44100, 2, 32, 1, 0, 0);
	float* samples = malloc(FRAMES * 8);
	UInt32 got;
	SInt64 length;
	UInt32 size = sizeof(length);

	// Reading, converted to float
	fixturePath(path, "pcm16.wav");
	url = urlFor(path);
	CHECK_EQ(ExtAudioFileOpenURL(url, &ref), noErr);
	CHECK_EQ(ExtAudioFileGetProperty(ref, kExtAudioFileProperty_FileLengthFrames, &size, &length), noErr);
	CHECK_EQ(length, FRAMES);
	CHECK_EQ(ExtAudioFileSetProperty(ref, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client), noErr);
	CHECK_EQ(readAll(ref, 2, 8, samples, FRAMES, &got), noErr);
	CHECK_EQ(got, FRAMES);
	{
		int wrong = 0;

		for (int i = 0; i < FRAMES; i++)
			for (int c = 0; c < 2; c++)
				if (samples[i * 2 + c] != value16(c, i) / 32768.0f)
					wrong++;
		CHECK_EQ(wrong, 0);
	}
	CHECK_EQ(readAll(ref, 2, 8, samples, 10, &got), noErr);
	CHECK_EQ(got, 0);

	// Seeking, and reading from there
	CHECK_EQ(ExtAudioFileSeek(ref, 500), noErr);
	CHECK_EQ(readAll(ref, 2, 8, samples, 4, &got), noErr);
	CHECK_EQ(got, 4);
	CHECK(samples[0] == value16(0, 500) / 32768.0f && samples[7] == value16(1, 503) / 32768.0f);
	ExtAudioFileDispose(ref);

	// Reading at another sample rate gives as many frames as the rates imply
	CHECK_EQ(ExtAudioFileOpenURL(url, &ref), noErr);
	client = fmt(22050, 2, 32, 1, 0, 0);
	CHECK_EQ(ExtAudioFileSetProperty(ref, kExtAudioFileProperty_ClientDataFormat, sizeof(client), &client), noErr);
	CHECK_EQ(readAll(ref, 2, 8, samples, FRAMES, &got), noErr);
	CHECK(got >= 499 && got <= 500);
	{
		// The ramp of the first channel keeps its shape: the output follows the input at half the rate
		double worst = 0;

		for (UInt32 i = 40; i + 40 < got; i++)
		{
			double want = (value16(0, 2 * i) + 0.0) / 32768.0;

			if (fabs(samples[i * 2] - want) > worst)
				worst = fabs(samples[i * 2] - want);
		}
		CHECK(worst < 0.002);
	}
	ExtAudioFileDispose(ref);

	// Non-interleaved 16-bit client
	CHECK_EQ(ExtAudioFileOpenURL(url, &ref), noErr);
	{
		AudioStreamBasicDescription planar = fmt(44100, 2, 16, 0, 1, 0);
		short left[FRAMES], right[FRAMES];
		struct { AudioBufferList header; AudioBuffer extra; } list;
		UInt32 n = FRAMES;

		planar.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		planar.mBytesPerFrame = planar.mBytesPerPacket = 2;
		CHECK_EQ(ExtAudioFileSetProperty(ref, kExtAudioFileProperty_ClientDataFormat, sizeof(planar), &planar), noErr);
		list.header.mNumberBuffers = 2;
		list.header.mBuffers[0].mNumberChannels = 1;
		list.header.mBuffers[0].mData = left;
		list.header.mBuffers[0].mDataByteSize = sizeof(left);
		list.header.mBuffers[1].mNumberChannels = 1;
		list.header.mBuffers[1].mData = right;
		list.header.mBuffers[1].mDataByteSize = sizeof(right);
		CHECK_EQ(ExtAudioFileRead(ref, &n, &list.header), noErr);
		CHECK_EQ(n, FRAMES);
		CHECK(left[0] == value16(0, 0) && left[999] == value16(0, 999) && right[0] == value16(1, 0) && right[999] == value16(1, 999));
	}
	ExtAudioFileDispose(ref);
	CFRelease(url);

	// Writing: the client's 16-bit audio is converted to the format of each file, and the files are the ones macOS makes
	{
		static const struct { const char* fixture; AudioFileTypeID type; AudioStreamBasicDescription format; } FILES[] = {
			{ "pcm24.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 } },
			{ "float32.wav", kAudioFileWAVEType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 } },
			{ "pcm24.aiff", kAudioFileAIFFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 } },
			{ "fl32.aifc", kAudioFileAIFCType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 } },
			{ "pcm24be.caf", kAudioFileCAFType, { 44100, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsPacked, 6, 1, 6, 2, 24, 0 } },
		};
		AudioStreamBasicDescription src = fmt(44100, 2, 16, 0, 1, 0);
		unsigned char* audio = makeAudio(&src, FRAMES);

		for (size_t i = 0; i < sizeof(FILES) / sizeof(FILES[0]); i++)
		{
			AudioBufferList list;
			long wantSize, gotSize;
			unsigned char *want, *file;

			tempPath(mine, FILES[i].fixture);
			unlink(mine);
			url = urlFor(mine);
			CHECK_EQ(ExtAudioFileCreateWithURL(url, FILES[i].type, &FILES[i].format, NULL, 0, &ref), noErr);
			CHECK_EQ(ExtAudioFileSetProperty(ref, kExtAudioFileProperty_ClientDataFormat, sizeof(src), &src), noErr);

			list.mNumberBuffers = 1;
			list.mBuffers[0].mNumberChannels = 2;
			list.mBuffers[0].mData = audio;
			list.mBuffers[0].mDataByteSize = 400 * 4;
			CHECK_EQ(ExtAudioFileWrite(ref, 400, &list), noErr);
			list.mBuffers[0].mData = audio + 400 * 4;
			list.mBuffers[0].mDataByteSize = 600 * 4;
			CHECK_EQ(ExtAudioFileWrite(ref, 600, &list), noErr);
			CHECK_EQ(ExtAudioFileDispose(ref), noErr);

			fixturePath(path, FILES[i].fixture);
			want = readFile(path, &wantSize);
			file = readFile(mine, &gotSize);
			{
				struct expect e = { FILES[i].fixture, FILES[i].type, FILES[i].format, FRAMES, 0 };
				long at;

				if (!sameFile(&e, want, wantSize, file, gotSize, &at))
				{
					printf("FAIL ExtAudioFile wrote %s differently from macOS (sizes %ld and %ld, first difference at %ld)\n", FILES[i].fixture, gotSize, wantSize, at);
					g_failures++;
				}
			}
			free(want);
			free(file);
			unlink(mine);
			CFRelease(url);
		}
		free(audio);
	}
	free(samples);
}

// The converter and the file are separate pieces: using both by hand gives what the audio is
struct convInput
{
	const void* data;
	UInt32 frames, bytesPerFrame, done;
};

static OSStatus convProc(AudioConverterRef c, UInt32* n, AudioBufferList* l, AudioStreamPacketDescription** pd, void* u)
{
	struct convInput* in = u;
	UInt32 give = in->frames - in->done;

	if (give > *n)
		give = *n;
	l->mBuffers[0].mData = (char*) in->data + (size_t) in->done * in->bytesPerFrame;
	l->mBuffers[0].mDataByteSize = give * in->bytesPerFrame;
	in->done += give;
	*n = give;
	return noErr;
}

static void test_file_with_converter(void)
{
	char path[512];
	AudioFileID f;
	CFURLRef url;
	AudioStreamBasicDescription src, dst = fmt(44100, 2, 32, 1, 0, 0);
	AudioConverterRef c;
	unsigned char* raw = malloc(FRAMES * 4);
	float* out = malloc(FRAMES * 8);
	UInt32 bytes = FRAMES * 4, packets = FRAMES, size = sizeof(src), n = FRAMES;
	AudioBufferList list;
	int wrong = 0;

	fixturePath(path, "pcm16.aiff");
	url = urlFor(path);
	CHECK_EQ(AudioFileOpenURL(url, kAudioFileReadPermission, 0, &f), noErr);
	CHECK_EQ(AudioFileGetProperty(f, kAudioFilePropertyDataFormat, &size, &src), noErr);
	CHECK_EQ(AudioFileReadPacketData(f, false, &bytes, NULL, 0, &packets, raw), noErr);
	AudioFileClose(f);
	CFRelease(url);

	// Big-endian 16-bit integers to native floats
	CHECK_EQ(AudioConverterNew(&src, &dst, &c), noErr);
	{
		struct convInput in = { raw, FRAMES, 4, 0 };

		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = 2;
		list.mBuffers[0].mData = out;
		list.mBuffers[0].mDataByteSize = FRAMES * 8;
		CHECK_EQ(AudioConverterFillComplexBuffer(c, convProc, &in, &n, &list, NULL), noErr);
	}
	CHECK_EQ(n, FRAMES);
	for (int i = 0; i < FRAMES; i++)
		for (int ch = 0; ch < 2; ch++)
			if (out[i * 2 + ch] != value16(ch, i) / 32768.0f)
				wrong++;
	CHECK_EQ(wrong, 0);
	AudioConverterDispose(c);
	free(raw);
	free(out);
}

int main(int argc, char** argv)
{
	char* made;

	if (argc < 2)
	{
		printf("usage: audiofile_pcm <fixtures directory>\n");
		return 2;
	}
	g_fixtures = argv[1];

	snprintf(g_dir, sizeof(g_dir), "/tmp/audiofile_pcm.XXXXXX");
	made = mkdtemp(g_dir);
	if (!made)
	{
		printf("cannot create a temporary directory\n");
		return 2;
	}

	test_fixtures();
	test_six_channels();
	test_written_files_match_fixtures();
	test_unsupported_formats();
	test_writing_rules();
	test_channel_layouts();
	test_handmade_files();
	test_callbacks();
	test_type_detection();
	test_ext_audio_file();
	test_companded_fallback();
	test_file_with_converter();

	rmdir(g_dir);

	if (g_failures)
	{
		printf("audiofile_pcm: %d failure(s)\n", g_failures);
		return 1;
	}

	printf("audiofile_pcm: OK\n");
	return 0;
}
