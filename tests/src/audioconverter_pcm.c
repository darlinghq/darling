// Copyright (C) 2026 Lubos Dolezel
//
// AudioConverter between linear PCM formats: sample formats, byte order, alignment,
// interleaving, channel counts and maps, and sample rates. Needs no codec library.
// The expected samples follow the AudioConverter documentation, and for what it leaves
// open, behaviour observed on macOS (marked as quirks, see the code in PCMConverter.cpp).
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long a_ = (a), b_ = (b); if (a_ != b_) { printf("FAIL %s:%d: %s = %lld, expected %lld\n", __FILE__, __LINE__, #a, a_, b_); g_failures++; } } while (0)

static AudioStreamBasicDescription fmt(double rate, int channels, int bits, int isFloat, int isSigned, int bigEndian, int packed)
{
	AudioStreamBasicDescription d;

	memset(&d, 0, sizeof(d));
	d.mSampleRate = rate;
	d.mFormatID = kAudioFormatLinearPCM;
	d.mFormatFlags = (isFloat ? kAudioFormatFlagIsFloat : (isSigned ? kAudioFormatFlagIsSignedInteger : 0))
		| (bigEndian ? kAudioFormatFlagIsBigEndian : 0) | (packed ? kAudioFormatFlagIsPacked : 0);
	d.mChannelsPerFrame = channels;
	d.mBitsPerChannel = bits;
	d.mFramesPerPacket = 1;
	d.mBytesPerFrame = d.mBytesPerPacket = channels * ((bits + 7) / 8);
	return d;
}

static AudioStreamBasicDescription s16(int ch) { return fmt(44100, ch, 16, 0, 1, 0, 1); }
static AudioStreamBasicDescription f32(int ch) { return fmt(44100, ch, 32, 1, 0, 0, 1); }

struct feed
{
	const void* data;
	UInt32 frames, bytesPerFrame, done, chunk;
	int calls;
	OSStatus failWith;
};

static OSStatus feedProc(AudioConverterRef c, UInt32* n, AudioBufferList* l, AudioStreamPacketDescription** pd, void* u)
{
	struct feed* f = u;
	UInt32 give = f->frames - f->done;

	f->calls++;
	if (f->failWith != noErr && f->calls > 1)
		return f->failWith;
	if (give > *n)
		give = *n;
	if (f->chunk && give > f->chunk)
		give = f->chunk;

	l->mBuffers[0].mData = (char*) f->data + (size_t) f->done * f->bytesPerFrame;
	l->mBuffers[0].mDataByteSize = give * f->bytesPerFrame;
	f->done += give;
	*n = give;
	return noErr;
}

// Converts all of 'in' and returns the number of output frames
static UInt32 convert(AudioStreamBasicDescription src, AudioStreamBasicDescription dst, const void* in, UInt32 inFrames, void* out, UInt32 outCapacity)
{
	AudioConverterRef c;
	OSStatus status = AudioConverterNew(&src, &dst, &c);

	if (status != noErr)
	{
		printf("FAIL AudioConverterNew = %d\n", (int) status);
		g_failures++;
		return 0;
	}

	struct feed f = { in, inFrames, src.mBytesPerFrame, 0, 0, 0, noErr };
	AudioBufferList list;
	UInt32 n = outCapacity;

	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = dst.mChannelsPerFrame;
	list.mBuffers[0].mData = out;
	list.mBuffers[0].mDataByteSize = outCapacity * dst.mBytesPerFrame;

	status = AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL);
	if (status != noErr)
	{
		printf("FAIL AudioConverterFillComplexBuffer = %d\n", (int) status);
		g_failures++;
	}
	CHECK_EQ(list.mBuffers[0].mDataByteSize, n * dst.mBytesPerFrame);
	AudioConverterDispose(c);
	return n;
}

static void expectBytes(const char* name, const void* got, const unsigned char* want, size_t size)
{
	if (memcmp(got, want, size) != 0)
	{
		const unsigned char* g = got;

		printf("FAIL %s: got", name);
		for (size_t i = 0; i < size; i++)
			printf(" %02x", g[i]);
		printf("\n  expected");
		for (size_t i = 0; i < size; i++)
			printf(" %02x", want[i]);
		printf("\n");
		g_failures++;
	}
}

#define EXPECT(name, got, ...) do { const unsigned char want_[] = { __VA_ARGS__ }; expectBytes(name, got, want_, sizeof(want_)); } while (0)

static void test_float_to_int(void)
{
	unsigned char out[128];
	const float x[] = { 0, 0.5f, 1.0f, -1.0f, 1.5f, -1.5f, 0.5f / 32768, 1.5f / 32768, 2.5f / 32768, -0.5f / 32768, -1.5f / 32768, 0.99999f, 0.3f, -0.3f };

	CHECK_EQ(convert(f32(1), s16(1), x, 14, out, 14), 14);
	// Full scale is 1.0 = 32768, so +1.0 is clipped; halves round up
	EXPECT("float to 16-bit", out, 0x00, 0x00, 0x00, 0x40, 0xff, 0x7f, 0x00, 0x80, 0xff, 0x7f, 0x00, 0x80, 0x01, 0x00, 0x02, 0x00,
		0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x7f, 0x66, 0x26, 0x9a, 0xd9);

	CHECK_EQ(convert(f32(1), fmt(44100, 1, 24, 0, 1, 0, 1), x, 6, out, 6), 6);
	EXPECT("float to 24-bit", out, 0, 0, 0, 0, 0, 0x40, 0xff, 0xff, 0x7f, 0, 0, 0x80, 0xff, 0xff, 0x7f, 0, 0, 0x80);

	CHECK_EQ(convert(f32(1), fmt(44100, 1, 16, 0, 1, 1, 1), x, 6, out, 6), 6);
	EXPECT("float to 16-bit big-endian", out, 0, 0, 0x40, 0, 0x7f, 0xff, 0x80, 0, 0x7f, 0xff, 0x80, 0);

	{
		const double d[] = { 0.5, 1.0, -1.0, 2.0, 1.5 / 32768, 0.5 / 32768 };

		CHECK_EQ(convert(fmt(44100, 1, 64, 1, 0, 0, 1), s16(1), d, 6, out, 6), 6);
		EXPECT("double to 16-bit", out, 0x00, 0x40, 0xff, 0x7f, 0x00, 0x80, 0xff, 0x7f, 0x02, 0x00, 0x01, 0x00);
	}
}

static void test_quirk_float_to_int32_clip(void)
{
	unsigned char out[64];
	const float x[] = { 0, 0.5f, 1.0f, -1.0f, 1.5f, -1.5f };
	const double d[] = { 0.5, 1.0, -1.0, 0.5 / 2147483648.0, 1.5 / 2147483648.0 };

	CHECK_EQ(convert(f32(1), fmt(44100, 1, 32, 0, 1, 0, 1), x, 6, out, 6), 6);
	// 1.0 clips at 0x7fffff80 instead of 0x7fffffff
	EXPECT("float to 32-bit", out, 0, 0, 0, 0, 0, 0, 0, 0x40, 0x80, 0xff, 0xff, 0x7f, 0, 0, 0, 0x80, 0x80, 0xff, 0xff, 0x7f, 0, 0, 0, 0x80);

	CHECK_EQ(convert(fmt(44100, 1, 64, 1, 0, 0, 1), fmt(44100, 1, 32, 0, 1, 0, 1), d, 5, out, 5), 5);
	EXPECT("double to 32-bit", out, 0, 0, 0, 0x40, 0x80, 0xff, 0xff, 0x7f, 0, 0, 0, 0x80, 1, 0, 0, 0, 2, 0, 0, 0);
}

static void test_quirk_float_nan_inf(void)
{
	unsigned char out[16];
	const float x[] = { NAN, INFINITY, -INFINITY };

	CHECK_EQ(convert(f32(1), s16(1), x, 3, out, 3), 3);
	EXPECT("float NaN and infinities", out, 0x00, 0x80, 0xff, 0x7f, 0x00, 0x80);
}

static void test_quirk_float_to_int8_rounding(void)
{
	unsigned char out[16];
	const float x[] = { 0.5f / 128, 1.5f / 128, 2.5f / 128, -0.5f / 128, -1.5f / 128, -2.5f / 128, 0.9f / 128, -0.9f / 128, 0.3f / 128, 126.5f / 128, 127.5f / 128 };

	CHECK_EQ(convert(f32(1), fmt(44100, 1, 8, 0, 1, 0, 1), x, 11, out, 11), 11);
	EXPECT("float to signed 8-bit", out, 1, 2, 3, 0, 0xfe, 0xfd, 1, 0xff, 0, 0x7f, 0x7f);

	CHECK_EQ(convert(f32(1), fmt(44100, 1, 8, 0, 0, 0, 1), x, 11, out, 11), 11);
	EXPECT("float to unsigned 8-bit", out, 0x81, 0x82, 0x83, 0x80, 0x7e, 0x7d, 0x81, 0x7f, 0x80, 0xff, 0xff);
}

static void test_int_to_float(void)
{
	unsigned char out[128];
	const short x[] = { 0, 1, -1, 32767, -32768, 12345, -12345, 255 };
	float f[8];
	int i;

	CHECK_EQ(convert(s16(1), f32(1), x, 8, f, 8), 8);
	for (i = 0; i < 8; i++)
		CHECK(f[i] == (float) x[i] / 32768.0f);

	{
		double d[4];

		CHECK_EQ(convert(s16(1), fmt(44100, 1, 64, 1, 0, 0, 1), x, 4, d, 4), 4);
		CHECK(d[0] == 0 && d[1] == 1.0 / 32768 && d[2] == -1.0 / 32768 && d[3] == 32767.0 / 32768);
	}

	{
		const int w[] = { 0, 0x7fffffff, (int) 0x80000000, 0x40000000 };
		float g[4];

		CHECK_EQ(convert(fmt(44100, 1, 32, 0, 1, 0, 1), f32(1), w, 4, g, 4), 4);
		CHECK(g[0] == 0 && g[1] == 1.0f && g[2] == -1.0f && g[3] == 0.5f);
	}

	{
		const unsigned char u[] = { 0, 1, 0x7f, 0x80, 0x81, 0xff };
		float g[6];

		CHECK_EQ(convert(fmt(44100, 1, 8, 0, 0, 0, 1), f32(1), u, 6, g, 6), 6);
		CHECK(g[0] == -1.0f && g[1] == -127.0f / 128 && g[2] == -1.0f / 128 && g[3] == 0 && g[4] == 1.0f / 128 && g[5] == 127.0f / 128);
	}

	{
		const unsigned char b[] = { 1, 2, 3, 0xff, 0xff, 0xff, 0x00, 0x00, 0x80, 0xff, 0xff, 0x7f };
		float g[4];

		CHECK_EQ(convert(fmt(44100, 1, 24, 0, 1, 0, 1), f32(1), b, 4, g, 4), 4);
		CHECK(g[0] == 197121.0f / 8388608 && g[1] == -1.0f / 8388608 && g[2] == -1.0f && g[3] == 8388607.0f / 8388608);
	}
	(void) out;
}

static void test_int_widening_and_narrowing(void)
{
	unsigned char out[128];
	const short x[] = { 0, 1, -1, 32767, -32768, 12345, -12345, 255 };

	CHECK_EQ(convert(s16(1), fmt(44100, 1, 32, 0, 1, 0, 1), x, 8, out, 8), 8);
	EXPECT("16 to 32-bit", out, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0xff, 0xff, 0, 0, 0xff, 0x7f, 0, 0, 0, 0x80, 0, 0, 0x39, 0x30, 0, 0, 0xc7, 0xcf, 0, 0, 0xff, 0);

	CHECK_EQ(convert(s16(1), fmt(44100, 1, 24, 0, 1, 0, 1), x, 8, out, 8), 8);
	EXPECT("16 to 24-bit", out, 0, 0, 0, 0, 1, 0, 0, 0xff, 0xff, 0, 0xff, 0x7f, 0, 0, 0x80, 0, 0x39, 0x30, 0, 0xc7, 0xcf, 0, 0xff, 0);

	{
		const int w[] = { 0, 0x00007fff, 0x00008000, 0x00018000, 0x7fffffff, (int) 0x80000000, 0x12345678, -0x00008000, -0x00018000, -0x00008001 };

		CHECK_EQ(convert(fmt(44100, 1, 32, 0, 1, 0, 1), s16(1), w, 10, out, 10), 10);
		EXPECT("32 to 16-bit", out, 0, 0, 0, 0, 1, 0, 2, 0, 0xff, 0x7f, 0, 0x80, 0x34, 0x12, 0, 0, 0xff, 0xff, 0xff, 0xff);
	}

	{
		const unsigned char b[] = { 1, 2, 3, 0xff, 0xff, 0xff, 0x00, 0x00, 0x80, 0xff, 0xff, 0x7f };

		CHECK_EQ(convert(fmt(44100, 1, 24, 0, 1, 0, 1), s16(1), b, 4, out, 4), 4);
		EXPECT("24 to 16-bit", out, 0x02, 0x03, 0x00, 0x00, 0x00, 0x80, 0xff, 0x7f);

		CHECK_EQ(convert(fmt(44100, 1, 24, 0, 1, 0, 1), fmt(44100, 1, 32, 0, 1, 0, 1), b, 4, out, 4), 4);
		EXPECT("24 to 32-bit", out, 0x00, 0x01, 0x02, 0x03, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x80, 0x00, 0xff, 0xff, 0x7f);
	}

	{
		const unsigned char u[] = { 0, 1, 0x7f, 0x80, 0x81, 0xff };

		CHECK_EQ(convert(fmt(44100, 1, 8, 0, 0, 0, 1), s16(1), u, 6, out, 6), 6);
		EXPECT("unsigned 8 to 16-bit", out, 0x00, 0x80, 0x00, 0x81, 0x00, 0xff, 0x00, 0x00, 0x00, 0x01, 0x00, 0x7f);
	}

	{
		const int w[] = { 0x7f00, 0x7fffffff };

		CHECK_EQ(convert(fmt(44100, 1, 32, 0, 1, 0, 1), fmt(44100, 1, 32, 0, 1, 1, 1), w, 2, out, 2), 2);
		EXPECT("32-bit byte swap", out, 0x00, 0x00, 0x7f, 0x00, 0x7f, 0xff, 0xff, 0xff);
	}
}

static void test_quirk_int_narrowing(void)
{
	unsigned char out[64];
	const short x[] = { 0, 1, -1, 32767, -32768, 12345, -12345, 255, 127, 128, 129, 256, 383, 384, -127, -128, -129, -255, -256, -383, -384 };

	// 16 to 8 bits drops the low byte
	CHECK_EQ(convert(s16(1), fmt(44100, 1, 8, 0, 1, 0, 1), x, 21, out, 21), 21);
	EXPECT("16 to signed 8-bit", out, 0x00, 0x00, 0xff, 0x7f, 0x80, 0x30, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe);

	CHECK_EQ(convert(s16(1), fmt(44100, 1, 8, 0, 0, 0, 1), x, 8, out, 8), 8);
	EXPECT("16 to unsigned 8-bit", out, 0x80, 0x80, 0x7f, 0xff, 0x00, 0xb0, 0x4f, 0x80);

	{
		// 32 to 8 bits goes through a float, so 0x017fffff is rounded up first
		const int w[] = { 0x7fffffff, 0x00800000, 0x00ffffff, 0x01000000, 0x017fffff, 0x01800000, -0x00800000, -0x00800001, -0x01800000, -0x01000000 };

		CHECK_EQ(convert(fmt(44100, 1, 32, 0, 1, 0, 1), fmt(44100, 1, 8, 0, 1, 0, 1), w, 10, out, 10), 10);
		EXPECT("32 to 8-bit", out, 0x7f, 0x01, 0x01, 0x01, 0x02, 0x02, 0x00, 0xff, 0xfe, 0xff);

		CHECK_EQ(convert(fmt(44100, 1, 32, 0, 1, 0, 1), fmt(44100, 1, 8, 0, 0, 0, 1), w, 3, out, 3), 3);
		EXPECT("32 to unsigned 8-bit", out, 0xff, 0x81, 0x81);
	}

	{
		// Other reductions round halves up
		const int w[] = { 0x7f, 0x80, 0x81, 0xff, 0x100, 0x17f, 0x180, -0x7f, -0x80, -0x81, -0x100, -0x17f, -0x180 };
		unsigned char b[39];
		int i;

		for (i = 0; i < 13; i++)
		{
			b[i * 3] = w[i] & 0xff;
			b[i * 3 + 1] = (w[i] >> 8) & 0xff;
			b[i * 3 + 2] = (w[i] >> 16) & 0xff;
		}

		CHECK_EQ(convert(fmt(44100, 1, 24, 0, 1, 0, 1), s16(1), b, 13, out, 13), 13);
		EXPECT("24 to 16-bit rounding", out, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
	}
}

static void test_alignment_and_packing(void)
{
	unsigned char out[64];
	AudioStreamBasicDescription a = fmt(44100, 1, 16, 0, 1, 0, 0);

	a.mBytesPerFrame = a.mBytesPerPacket = 4;
	a.mFormatFlags |= kAudioFormatFlagIsAlignedHigh;
	{
		const int hi[] = { 0x12340000, 0x7fff0000, (int) 0x80000000 };

		CHECK_EQ(convert(a, s16(1), hi, 3, out, 3), 3);
		EXPECT("16 bits aligned high in 32", out, 0x34, 0x12, 0xff, 0x7f, 0x00, 0x80);
	}

	a.mFormatFlags &= ~kAudioFormatFlagIsAlignedHigh;
	{
		const int lo[] = { 0x1234, -1, 0x7fff };
		const short x[] = { 0, 1, -1 };

		CHECK_EQ(convert(a, s16(1), lo, 3, out, 3), 3);
		EXPECT("16 bits aligned low in 32", out, 0x34, 0x12, 0xff, 0xff, 0xff, 0x7f);

		CHECK_EQ(convert(s16(1), a, x, 3, out, 3), 3);
		EXPECT("16 bits to aligned low in 32", out, 0, 0, 0, 0, 1, 0, 0, 0, 0xff, 0xff, 0, 0);
	}

	a = fmt(44100, 1, 24, 0, 1, 0, 0);
	a.mBytesPerFrame = a.mBytesPerPacket = 4;
	a.mFormatFlags |= kAudioFormatFlagIsAlignedHigh;
	{
		const int hi[] = { 0x12345600, 0x7fffff00 };
		const short x[] = { 0, 1, -1 };

		CHECK_EQ(convert(a, s16(1), hi, 2, out, 2), 2);
		EXPECT("24 bits aligned high in 32", out, 0x34, 0x12, 0xff, 0x7f);

		CHECK_EQ(convert(s16(1), a, x, 3, out, 3), 3);
		EXPECT("16 bits to 24 aligned high in 32", out, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0xff, 0xff);
	}
}

static void test_quirk_channel_count_changes(void)
{
	unsigned char out[64];
	const short st[] = { 1000, 2000, 3000, 4000, -1000, -3000 };

	// Stereo to mono keeps the left channel
	CHECK_EQ(convert(s16(2), s16(1), st, 3, out, 3), 3);
	EXPECT("stereo to mono", out, 0xe8, 0x03, 0xb8, 0x0b, 0x18, 0xfc);

	// Mono to stereo copies
	CHECK_EQ(convert(s16(1), s16(2), st, 3, out, 3), 3);
	EXPECT("mono to stereo", out, 0xe8, 0x03, 0xe8, 0x03, 0xd0, 0x07, 0xd0, 0x07, 0xb8, 0x0b, 0xb8, 0x0b);

	// More channels than the destination loses the last ones
	CHECK_EQ(convert(s16(3), s16(2), st, 2, out, 2), 2);
	EXPECT("three to two channels", out, 0xe8, 0x03, 0xd0, 0x07, 0xa0, 0x0f, 0x18, 0xfc);

	// Fewer channels leave the added ones silent, mono included except for stereo
	CHECK_EQ(convert(s16(2), s16(3), st, 2, out, 2), 2);
	EXPECT("two to three channels", out, 0xe8, 0x03, 0xd0, 0x07, 0x00, 0x00, 0xb8, 0x0b, 0xa0, 0x0f, 0x00, 0x00);

	CHECK_EQ(convert(s16(1), s16(3), st, 2, out, 2), 2);
	EXPECT("mono to three channels", out, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x07, 0x00, 0x00, 0x00, 0x00);

	CHECK_EQ(convert(s16(6), s16(1), st, 1, out, 1), 1);
	EXPECT("six channels to mono", out, 0xe8, 0x03);

	{
		const float f[] = { 0.5f, 0.25f, 1.0f, 1.0f };
		float g[4];

		CHECK_EQ(convert(f32(2), f32(1), f, 2, g, 2), 2);
		CHECK(g[0] == 0.5f && g[1] == 1.0f);
		CHECK_EQ(convert(f32(1), f32(2), f, 2, g, 2), 2);
		CHECK(g[0] == 0.5f && g[1] == 0.5f && g[2] == 0.25f && g[3] == 0.25f);
	}
}

struct planar
{
	const short* ch[2];
	UInt32 frames, done;
};

static OSStatus planarProc(AudioConverterRef c, UInt32* n, AudioBufferList* l, AudioStreamPacketDescription** pd, void* u)
{
	struct planar* p = u;
	UInt32 give = p->frames - p->done;

	if (give > *n)
		give = *n;
	CHECK_EQ(l->mNumberBuffers, 2);
	for (int i = 0; i < 2; i++)
	{
		l->mBuffers[i].mData = (void*) (p->ch[i] + p->done);
		l->mBuffers[i].mDataByteSize = give * 2;
		l->mBuffers[i].mNumberChannels = 1;
	}
	p->done += give;
	*n = give;
	return noErr;
}

static void test_non_interleaved(void)
{
	AudioStreamBasicDescription planarFmt = s16(2);
	AudioStreamBasicDescription interleaved = s16(2);
	AudioConverterRef c;
	const short left[] = { 1, 2, 3, 4 }, right[] = { -1, -2, -3, -4 };
	struct planar p = { { left, right }, 4, 0 };
	short out[8];
	AudioBufferList list;
	UInt32 n = 4;

	planarFmt.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
	planarFmt.mBytesPerFrame = planarFmt.mBytesPerPacket = 2;

	CHECK_EQ(AudioConverterNew(&planarFmt, &interleaved, &c), noErr);
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 2;
	list.mBuffers[0].mData = out;
	list.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterFillComplexBuffer(c, planarProc, &p, &n, &list, NULL), noErr);
	CHECK_EQ(n, 4);
	CHECK(out[0] == 1 && out[1] == -1 && out[2] == 2 && out[3] == -2 && out[6] == 4 && out[7] == -4);
	AudioConverterDispose(c);

	// And the other way, through the interleaved input
	{
		short l2[4] = { 0 }, r2[4] = { 0 };
		struct feed f = { out, 4, 4, 0, 0, 0, noErr };
		struct
		{
			AudioBufferList header;
			AudioBuffer extra;
		} planarOut;

		CHECK_EQ(AudioConverterNew(&interleaved, &planarFmt, &c), noErr);
		planarOut.header.mNumberBuffers = 2;
		planarOut.header.mBuffers[0].mNumberChannels = 1;
		planarOut.header.mBuffers[0].mData = l2;
		planarOut.header.mBuffers[0].mDataByteSize = sizeof(l2);
		planarOut.header.mBuffers[1].mNumberChannels = 1;
		planarOut.header.mBuffers[1].mData = r2;
		planarOut.header.mBuffers[1].mDataByteSize = sizeof(r2);
		n = 4;
		CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &planarOut.header, NULL), noErr);
		CHECK_EQ(n, 4);
		CHECK(l2[0] == 1 && l2[3] == 4 && r2[0] == -1 && r2[3] == -4);
		CHECK_EQ(planarOut.header.mBuffers[0].mDataByteSize, 8);
		AudioConverterDispose(c);
	}
}

static void test_channel_map(void)
{
	const short st[] = { 1000, 2000, 3000, 4000, -3000, -1000 };
	AudioStreamBasicDescription s = s16(2);
	AudioConverterRef c;
	short out[8];
	AudioBufferList list;
	struct feed f = { st, 3, 4, 0, 0, 0, noErr };
	UInt32 n = 3, size = 0;
	Boolean writable = 0;
	SInt32 swap[2] = { 1, 0 }, silence[2] = { -1, 0 }, got[2];

	CHECK_EQ(AudioConverterNew(&s, &s, &c), noErr);
	CHECK_EQ(AudioConverterGetPropertyInfo(c, kAudioConverterChannelMap, &size, &writable), noErr);
	CHECK_EQ(size, 8);
	CHECK(writable);

	size = sizeof(got);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterChannelMap, &size, got), noErr);
	CHECK(got[0] == 0 && got[1] == 1);

	CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterChannelMap, sizeof(swap), swap), noErr);
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 2;
	list.mBuffers[0].mData = out;
	list.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK(n == 3 && out[0] == 2000 && out[1] == 1000 && out[2] == 4000 && out[3] == 3000 && out[4] == -1000 && out[5] == -3000);

	// A negative entry is silence
	CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterChannelMap, sizeof(silence), silence), noErr);
	AudioConverterReset(c);
	f.done = 0;
	n = 3;
	list.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK(n == 3 && out[0] == 0 && out[1] == 1000 && out[2] == 0 && out[3] == 3000);

	// A map for fewer channels than the destination has is a bad size
	CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterChannelMap, 4, swap), kAudioConverterErr_BadPropertySizeError);
	AudioConverterDispose(c);

	// A mono destination takes the channel named by the map
	{
		AudioStreamBasicDescription m = s16(1);
		SInt32 right[1] = { 1 };

		CHECK_EQ(AudioConverterNew(&s, &m, &c), noErr);
		CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterChannelMap, sizeof(right), right), noErr);
		f.done = 0;
		n = 3;
		list.mBuffers[0].mDataByteSize = 6;
		CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
		CHECK(n == 3 && out[0] == 2000 && out[1] == 4000 && out[2] == -1000);
		AudioConverterDispose(c);
	}
}

static void test_semantics(void)
{
	const short x[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	AudioStreamBasicDescription s = s16(1), d = f32(1);
	AudioConverterRef c;
	float out[16];
	AudioBufferList list;
	UInt32 n;
	struct feed f = { x, 10, 2, 0, 3, 0, noErr };	// three frames per call

	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 1;
	list.mBuffers[0].mData = out;

	CHECK_EQ(AudioConverterNew(&s, &d, &c), noErr);

	// The input is asked for as often as needed
	n = 10;
	list.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK_EQ(n, 10);
	CHECK(f.calls >= 4);
	CHECK(out[0] == 1.0f / 32768 && out[9] == 10.0f / 32768);
	CHECK_EQ(list.mBuffers[0].mDataByteSize, 40);

	// At the end there is nothing left
	n = 10;
	list.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK_EQ(n, 0);
	CHECK_EQ(list.mBuffers[0].mDataByteSize, 0);

	// The number of packets asked for, and the size of the buffer, limit the output
	f.done = 0;
	n = 4;
	list.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK_EQ(n, 4);
	CHECK_EQ(list.mBuffers[0].mDataByteSize, 16);
	CHECK(out[3] == 4.0f / 32768);

	n = 10;
	list.mBuffers[0].mDataByteSize = 8;
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK_EQ(n, 2);
	CHECK(out[0] == 5.0f / 32768 && out[1] == 6.0f / 32768);

	n = 0;
	list.mBuffers[0].mDataByteSize = 8;
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
	CHECK_EQ(n, 0);

	// An error from the input is returned with whatever was converted before it
	{
		struct feed failing = { x, 10, 2, 0, 3, 0, 'nomo' };

		n = 10;
		list.mBuffers[0].mDataByteSize = sizeof(out);
		CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &failing, &n, &list, NULL), 'nomo');
		CHECK_EQ(n, 3);
		CHECK_EQ(list.mBuffers[0].mDataByteSize, 12);
	}

	n = 4;
	CHECK_EQ(AudioConverterFillComplexBuffer(c, NULL, &f, &n, &list, NULL), paramErr);
	AudioConverterDispose(c);
}

static void test_errors(void)
{
	AudioStreamBasicDescription ok = s16(1), bad;
	AudioConverterRef c = NULL;

	bad = ok;
	bad.mBitsPerChannel = 12;
	CHECK_EQ(AudioConverterNew(&bad, &ok, &c), kAudioConverterErr_FormatNotSupported);
	CHECK_EQ(AudioConverterNew(&ok, &bad, &c), kAudioConverterErr_FormatNotSupported);

	bad = f32(1);
	bad.mBitsPerChannel = 16;
	CHECK_EQ(AudioConverterNew(&bad, &ok, &c), kAudioConverterErr_FormatNotSupported);

	bad = ok;
	bad.mChannelsPerFrame = 0;
	CHECK_EQ(AudioConverterNew(&bad, &ok, &c), paramErr);

	bad = ok;
	bad.mBytesPerFrame = bad.mBytesPerPacket = 3;
	CHECK_EQ(AudioConverterNew(&bad, &ok, &c), paramErr);

	CHECK_EQ(AudioConverterNew(NULL, &ok, &c), paramErr);
	CHECK_EQ(AudioConverterNew(&ok, &ok, NULL), paramErr);

	// A sample rate of zero is not specified, and nothing is resampled
	bad = ok;
	bad.mSampleRate = 0;
	CHECK_EQ(AudioConverterNew(&bad, &ok, &c), noErr);
	AudioConverterDispose(c);
}

static void test_properties(void)
{
	AudioStreamBasicDescription s = s16(2), d = fmt(48000, 2, 32, 1, 0, 0, 1);
	AudioConverterRef c;
	UInt32 v, size;
	Boolean writable;
	AudioConverterPrimeInfo prime;
	AudioStreamBasicDescription g;

	CHECK_EQ(AudioConverterNew(&s, &d, &c), noErr);

	size = 4;
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterSampleRateConverterQuality, &size, &v), noErr);
	CHECK_EQ(v, kAudioConverterQuality_Medium);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterSampleRateConverterComplexity, &size, &v), noErr);
	CHECK_EQ(v, kAudioConverterSampleRateConverterComplexity_Normal);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterPrimeMethod, &size, &v), noErr);
	CHECK_EQ(v, kConverterPrimeMethod_Normal);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterPropertyMinimumInputBufferSize, &size, &v), noErr);
	CHECK_EQ(v, 4);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterPropertyMaximumInputPacketSize, &size, &v), noErr);
	CHECK_EQ(v, 4);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterPropertyMaximumOutputPacketSize, &size, &v), noErr);
	CHECK_EQ(v, 8);

	size = sizeof(prime);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterPrimeInfo, &size, &prime), noErr);
	CHECK(prime.leadingFrames == 0 && prime.trailingFrames == 0);

	size = sizeof(g);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterCurrentInputStreamDescription, &size, &g), noErr);
	CHECK(g.mSampleRate == 44100 && g.mBitsPerChannel == 16);
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterCurrentOutputStreamDescription, &size, &g), noErr);
	CHECK(g.mSampleRate == 48000 && g.mBitsPerChannel == 32);

	v = kAudioConverterQuality_Max;
	CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterSampleRateConverterQuality, 4, &v), noErr);
	size = 4;
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterSampleRateConverterQuality, &size, &v), noErr);
	CHECK_EQ(v, kAudioConverterQuality_Max);

	CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterSampleRateConverterQuality, 1, &v), kAudioConverterErr_BadPropertySizeError);
	CHECK_EQ(AudioConverterSetProperty(c, 'zzzz', 4, &v), kAudioConverterErr_PropertyNotSupported);
	CHECK_EQ(AudioConverterSetProperty(c, kAudioConverterEncodeBitRate, 4, &v), kAudioConverterErr_PropertyNotSupported);
	size = 4;
	CHECK_EQ(AudioConverterGetProperty(c, 'zzzz', &size, &v), kAudioConverterErr_PropertyNotSupported);

	CHECK_EQ(AudioConverterGetPropertyInfo(c, kAudioConverterPrimeInfo, &size, &writable), noErr);
	CHECK(size == 8 && !writable);

	// Buffer sizes with the rate change: 250 frames of 4 bytes become 272.1 frames of 8 bytes
	v = 1000;
	size = 4;
	CHECK_EQ(AudioConverterGetProperty(c, kAudioConverterPropertyCalculateOutputBufferSize, &size, &v), noErr);
	CHECK(v >= 2176 && v <= 2192);
	AudioConverterDispose(c);
}

static void test_convert_buffer(void)
{
	AudioConverterRef c;
	AudioStreamBasicDescription m = s16(1), st = s16(2);
	unsigned char in[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	short out[8];
	UInt32 size = 16;
	AudioBufferList inList, outList;

	CHECK_EQ(AudioConverterNew(&m, &st, &c), noErr);
	CHECK_EQ(AudioConverterConvertBuffer(c, 8, in, &size, out), noErr);
	CHECK_EQ(size, 16);
	CHECK(out[0] == 0x0201 && out[1] == 0x0201 && out[2] == 0x0403 && out[3] == 0x0403 && out[6] == 0x0807 && out[7] == 0x0807);

	size = 4;
	CHECK_EQ(AudioConverterConvertBuffer(c, 8, in, &size, out), kAudioConverterErr_InvalidInputSize);

	// A partial frame at the end is not converted
	size = 16;
	CHECK_EQ(AudioConverterConvertBuffer(c, 3, in, &size, out), noErr);
	CHECK_EQ(size, 4);

	inList.mNumberBuffers = 1;
	inList.mBuffers[0].mNumberChannels = 1;
	inList.mBuffers[0].mData = in;
	inList.mBuffers[0].mDataByteSize = 8;
	outList.mNumberBuffers = 1;
	outList.mBuffers[0].mNumberChannels = 2;
	outList.mBuffers[0].mData = out;
	outList.mBuffers[0].mDataByteSize = sizeof(out);
	CHECK_EQ(AudioConverterConvertComplexBuffer(c, 4, &inList, &outList), noErr);
	CHECK_EQ(outList.mBuffers[0].mDataByteSize, 16);
	CHECK(out[0] == 0x0201 && out[7] == 0x0807);
	AudioConverterDispose(c);

	// Not for conversions that change the rate
	{
		AudioStreamBasicDescription slow = fmt(22050, 1, 16, 0, 1, 0, 1);

		CHECK_EQ(AudioConverterNew(&m, &slow, &c), noErr);
		size = 16;
		CHECK_EQ(AudioConverterConvertBuffer(c, 8, in, &size, out), paramErr);
		AudioConverterDispose(c);
	}
}

// Total output frames of a stream of 'inFrames' frames, asking for 'ask' frames at a time
static UInt32 resampledLength(double from, double to, UInt32 inFrames, UInt32 ask, float* samples, UInt32 capacity, const float* input)
{
	AudioStreamBasicDescription s = fmt(from, 1, 32, 1, 0, 0, 1), d = fmt(to, 1, 32, 1, 0, 0, 1);
	AudioConverterRef c;
	struct feed f = { input, inFrames, 4, 0, 100, 0, noErr };
	AudioBufferList list;
	UInt32 total = 0;

	CHECK_EQ(AudioConverterNew(&s, &d, &c), noErr);
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 1;

	for (int rounds = 0; rounds < 100000; rounds++)
	{
		UInt32 n = ask;

		if (total + n > capacity)
			n = capacity - total;
		list.mBuffers[0].mData = samples + total;
		list.mBuffers[0].mDataByteSize = n * 4;
		CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &n, &list, NULL), noErr);
		if (n == 0)
			break;
		total += n;
	}

	AudioConverterDispose(c);
	return total;
}

static void test_sample_rate_conversion(void)
{
	static float input[48000], output[96000];
	UInt32 i, n;

	for (i = 0; i < 48000; i++)
		input[i] = 0.5f * sinf(2 * (float) M_PI * 1000 * i / 44100);

	// The length follows the rates exactly, however the output is asked for
	CHECK_EQ(resampledLength(22050, 44100, 100, 1000, output, 96000, input), 200);
	CHECK_EQ(resampledLength(44100, 22050, 100, 1000, output, 96000, input), 50);
	CHECK_EQ(resampledLength(44100, 48000, 441, 7, output, 96000, input), 480);
	CHECK_EQ(resampledLength(44100, 48000, 4410, 1000, output, 96000, input), 4800);
	CHECK_EQ(resampledLength(44100, 8000, 44100, 333, output, 96000, input), 8000);
	CHECK_EQ(resampledLength(8000, 44100, 800, 64, output, 96000, input), 4410);

	// A sine keeps its frequency and level, aligned in time with the input
	n = resampledLength(44100, 48000, 44100, 1024, output, 96000, input);
	CHECK_EQ(n, 48000);
	{
		double worst = 0;

		for (i = 100; i < n - 100; i++)
		{
			double want = 0.5 * sin(2 * M_PI * 1000 * (double) i / 48000);

			if (fabs(output[i] - want) > worst)
				worst = fabs(output[i] - want);
		}
		CHECK(worst < 0.01);
	}

	// A constant stays constant
	for (i = 0; i < 4000; i++)
		input[i] = 0.25f;
	n = resampledLength(44100, 22050, 4000, 4000, output, 96000, input);
	CHECK_EQ(n, 2000);
	for (i = 50; i < n - 50; i++)
	{
		if (fabs(output[i] - 0.25) > 0.001)
		{
			printf("FAIL constant at %u: %f\n", i, output[i]);
			g_failures++;
			break;
		}
	}

	// What the new rate cannot hold is not folded back: 6 kHz at 48 kHz, down to 8 kHz
	for (i = 0; i < 48000; i++)
		input[i] = 0.5f * sinf(2 * (float) M_PI * 6000 * i / 48000);
	n = resampledLength(48000, 8000, 48000, 1024, output, 96000, input);
	CHECK_EQ(n, 8000);
	{
		double peak = 0;

		for (i = 100; i < n - 100; i++)
			if (fabs(output[i]) > peak)
				peak = fabs(output[i]);
		CHECK(peak < 0.02);
	}

	// Stereo and integer samples through the rate conversion as well
	{
		short in[400], out[1000];
		AudioStreamBasicDescription s = fmt(22050, 2, 16, 0, 1, 0, 1), d = fmt(44100, 2, 16, 0, 1, 0, 1);
		AudioConverterRef c;
		struct feed f = { in, 200, 4, 0, 0, 0, noErr };
		AudioBufferList list;
		UInt32 count = 1000;

		for (i = 0; i < 200; i++)
		{
			in[i * 2] = 10000;
			in[i * 2 + 1] = -5000;
		}
		list.mNumberBuffers = 1;
		list.mBuffers[0].mNumberChannels = 2;
		list.mBuffers[0].mData = out;
		list.mBuffers[0].mDataByteSize = sizeof(out);
		CHECK_EQ(AudioConverterNew(&s, &d, &c), noErr);
		CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &count, &list, NULL), noErr);
		CHECK_EQ(count, 400);
		CHECK(abs(out[200] - 10000) <= 1 && abs(out[201] + 5000) <= 1);
		AudioConverterDispose(c);
	}
}

static void test_reset(void)
{
	static float input[200], output[400];
	AudioStreamBasicDescription s = fmt(22050, 1, 32, 1, 0, 0, 1), d = fmt(44100, 1, 32, 1, 0, 0, 1);
	AudioConverterRef c;
	AudioBufferList list;
	UInt32 first, second;
	float firstSamples[8];
	struct feed f = { input, 100, 4, 0, 0, 0, noErr };

	for (int i = 0; i < 200; i++)
		input[i] = (float) i / 200;

	CHECK_EQ(AudioConverterNew(&s, &d, &c), noErr);
	list.mNumberBuffers = 1;
	list.mBuffers[0].mNumberChannels = 1;
	list.mBuffers[0].mData = output;

	first = 8;
	list.mBuffers[0].mDataByteSize = 32;
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &first, &list, NULL), noErr);
	CHECK_EQ(first, 8);
	memcpy(firstSamples, output, sizeof(firstSamples));

	// After a reset the stream starts again
	CHECK_EQ(AudioConverterReset(c), noErr);
	f.done = 0;
	second = 8;
	list.mBuffers[0].mDataByteSize = 32;
	CHECK_EQ(AudioConverterFillComplexBuffer(c, feedProc, &f, &second, &list, NULL), noErr);
	CHECK_EQ(second, 8);
	CHECK(memcmp(firstSamples, output, sizeof(firstSamples)) == 0);
	AudioConverterDispose(c);
}

int main(void)
{
	test_float_to_int();
	test_quirk_float_to_int32_clip();
	test_quirk_float_nan_inf();
	test_quirk_float_to_int8_rounding();
	test_int_to_float();
	test_int_widening_and_narrowing();
	test_quirk_int_narrowing();
	test_alignment_and_packing();
	test_quirk_channel_count_changes();
	test_non_interleaved();
	test_channel_map();
	test_semantics();
	test_errors();
	test_properties();
	test_convert_buffer();
	test_sample_rate_conversion();
	test_reset();

	if (g_failures)
	{
		printf("audioconverter_pcm: %d failure(s)\n", g_failures);
		return 1;
	}

	printf("audioconverter_pcm: OK\n");
	return 0;
}
