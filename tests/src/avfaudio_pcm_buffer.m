// CFLAGS: -framework Foundation -framework AVFAudio -framework CoreAudio
// Copyright (C) 2026 Darling developers
//
// AVAudioPCMBuffer, as described on the AVFAudio pages for AVAudioBuffer and AVAudioPCMBuffer
// (https://developer.apple.com/documentation/avfaudio/avaudiopcmbuffer):
//  - init(pcmFormat:frameCapacity:) returns nil when the format has no bytes per frame, or when the byte
//    capacity does not fit an unsigned 32-bit integer;
//  - the buffer list has one buffer for an interleaved format and one per channel otherwise, and
//    mDataByteSize of each is the frame length in bytes (so frameLength and mDataByteSize change together);
//  - frameCapacity is the capacity in frames, stride is the number of interleaved channels (1 otherwise);
//  - floatChannelData, int16ChannelData and int32ChannelData return one pointer per channel for a format
//    with that sample type and nil for another, with the pointers to separate memory when the format is
//    not interleaved and into one buffer, spaced by stride samples, when it is;
//  - init(pcmFormat:bufferListNoCopy:deallocator:) returns nil for a zero-byte frame, no buffers, a NULL
//    data pointer, a zero or unequal byte size, or a buffer count that does not match the format, and
//    calls the deallocator when the buffer deallocates.
// Cases marked GUESS pin choices the pages leave open (see AVAudioPCMBuffer.m): a new buffer has frame
// length zero, and a frame length above the capacity raises NSInvalidArgumentException.
// Prints PASS, or the failures with a non-zero exit status.
#import <AVFAudio/AVFAudio.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static AVAudioFormat* makeFormat(AVAudioCommonFormat common, AVAudioChannelCount channels, BOOL interleaved)
{
	return [[[AVAudioFormat alloc] initWithCommonFormat:common sampleRate:44100 channels:channels interleaved:interleaved] autorelease];
}

static void testStandardFormat(void)
{
	AVAudioFormat* format = makeFormat(AVAudioPCMFormatFloat32, 2, NO);
	AVAudioPCMBuffer* buffer = [[[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:100] autorelease];
	CHECK(buffer != nil);
	CHECK([buffer isKindOfClass:[AVAudioBuffer class]]);
	CHECK([buffer.format isEqual:format]);
	CHECK(buffer.frameCapacity == 100);
	CHECK(buffer.stride == 1);

	const AudioBufferList* list = buffer.audioBufferList;
	CHECK(list != NULL);
	CHECK(list->mNumberBuffers == 2);
	// QUIRK test, mutable list is a separate, refreshed list (macOS 26.7.3 and 10.14.4): another list than
	// audioBufferList with equal contents, the same pointer on every access.
	CHECK(buffer.mutableAudioBufferList != NULL && buffer.mutableAudioBufferList != list);
	CHECK(buffer.mutableAudioBufferList == buffer.mutableAudioBufferList);
	CHECK(buffer.mutableAudioBufferList->mNumberBuffers == 2);
	CHECK(buffer.mutableAudioBufferList->mBuffers[0].mData == list->mBuffers[0].mData);
	CHECK(list->mBuffers[0].mNumberChannels == 1 && list->mBuffers[1].mNumberChannels == 1);
	CHECK(list->mBuffers[0].mData != NULL && list->mBuffers[1].mData != NULL);
	CHECK(list->mBuffers[0].mData != list->mBuffers[1].mData);

	// GUESS: the frame length starts at zero.
	CHECK(buffer.frameLength == 0);
	CHECK(list->mBuffers[0].mDataByteSize == 0);

	buffer.frameLength = 40;
	CHECK(buffer.frameLength == 40);
	CHECK(list->mBuffers[0].mDataByteSize == 40 * sizeof(float));
	CHECK(list->mBuffers[1].mDataByteSize == 40 * sizeof(float));

	// The mutable list follows the frame length, and a byte size written into it is not taken up (quirk).
	CHECK(buffer.mutableAudioBufferList->mBuffers[0].mDataByteSize == 40 * sizeof(float));
	buffer.mutableAudioBufferList->mBuffers[0].mDataByteSize = 10 * sizeof(float);
	buffer.mutableAudioBufferList->mBuffers[1].mDataByteSize = 10 * sizeof(float);
	CHECK(buffer.frameLength == 40);
	CHECK(buffer.audioBufferList->mBuffers[0].mDataByteSize == 40 * sizeof(float));
	CHECK(buffer.mutableAudioBufferList->mBuffers[0].mDataByteSize == 40 * sizeof(float));

	buffer.frameLength = 100;
	CHECK(buffer.frameLength == 100);

	float* const* channels = buffer.floatChannelData;
	CHECK(channels != NULL);
	CHECK(channels[0] == list->mBuffers[0].mData && channels[1] == list->mBuffers[1].mData);
	CHECK(buffer.int16ChannelData == NULL);
	CHECK(buffer.int32ChannelData == NULL);

	// Each channel holds frameCapacity samples of its own.
	for (int i = 0; i < 100; i++)
	{
		channels[0][i] = (float) i;
		channels[1][i] = (float) -i;
	}
	CHECK(channels[0][99] == 99.0f && channels[1][99] == -99.0f);

	// GUESS: a length above the capacity raises.
	BOOL raised = NO;
	@try { buffer.frameLength = 101; }
	@catch (NSException* e) { raised = [[e name] isEqualToString:NSInvalidArgumentException]; }
	CHECK(raised);
	CHECK(buffer.frameLength == 100);
}

static void testInterleavedInt16(void)
{
	AVAudioFormat* format = makeFormat(AVAudioPCMFormatInt16, 3, YES);
	AVAudioPCMBuffer* buffer = [[[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:8] autorelease];
	CHECK(buffer != nil);
	CHECK(buffer.stride == 3);

	const AudioBufferList* list = buffer.audioBufferList;
	CHECK(list->mNumberBuffers == 1);
	CHECK(list->mBuffers[0].mNumberChannels == 3);

	buffer.frameLength = 8;
	CHECK(list->mBuffers[0].mDataByteSize == 8 * 3 * sizeof(int16_t));

	int16_t* const* channels = buffer.int16ChannelData;
	CHECK(channels != NULL);
	CHECK(buffer.floatChannelData == NULL);
	CHECK(buffer.int32ChannelData == NULL);

	// The pointers are into the one interleaved buffer; channel c, frame f is channels[c][f * stride].
	CHECK((char*) channels[0] == (char*) list->mBuffers[0].mData);
	for (int f = 0; f < 8; f++)
		for (int c = 0; c < 3; c++)
			channels[c][f * buffer.stride] = (int16_t) (f * 10 + c);
	const int16_t* raw = list->mBuffers[0].mData;
	for (int i = 0; i < 8 * 3; i++)
		CHECK(raw[i] == (int16_t) ((i / 3) * 10 + i % 3));
}

static void testInt32AndOtherFormats(void)
{
	AVAudioPCMBuffer* buffer = [[[AVAudioPCMBuffer alloc] initWithPCMFormat:makeFormat(AVAudioPCMFormatInt32, 1, NO)
		frameCapacity:4] autorelease];
	CHECK(buffer.int32ChannelData != NULL);
	CHECK(buffer.floatChannelData == NULL && buffer.int16ChannelData == NULL);

	// A 64-bit float format is a PCM format but has no channel data accessor.
	buffer = [[[AVAudioPCMBuffer alloc] initWithPCMFormat:makeFormat(AVAudioPCMFormatFloat64, 2, NO) frameCapacity:4] autorelease];
	CHECK(buffer != nil);
	CHECK(buffer.floatChannelData == NULL && buffer.int16ChannelData == NULL && buffer.int32ChannelData == NULL);
	buffer.frameLength = 4;
	CHECK(buffer.audioBufferList->mBuffers[0].mDataByteSize == 4 * sizeof(double));
}

static void testCreationFailures(void)
{
	AVAudioFormat* format = makeFormat(AVAudioPCMFormatFloat32, 2, YES);

	// 8 bytes per frame: 536870912 frames is 4 GiB, which does not fit 32 bits.
	AVAudioPCMBuffer* tooBig = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:536870912];
	CHECK(tooBig == nil);
	[tooBig release];

	// A format with no bytes per frame.
	AudioStreamBasicDescription asbd = *format.streamDescription;
	asbd.mBytesPerFrame = asbd.mBytesPerPacket = 0;
	AVAudioFormat* zero = [[[AVAudioFormat alloc] initWithStreamDescription:&asbd] autorelease];
	CHECK(zero != nil);
	AVAudioPCMBuffer* noBytes = [[AVAudioPCMBuffer alloc] initWithPCMFormat:zero frameCapacity:10];
	CHECK(noBytes == nil);
	[noBytes release];

	// A capacity of zero is a valid buffer with room for no frames.
	AVAudioPCMBuffer* empty = [[[AVAudioPCMBuffer alloc] initWithPCMFormat:format frameCapacity:0] autorelease];
	CHECK(empty != nil && empty.frameCapacity == 0);
	empty.frameLength = 0;
	CHECK(empty.frameLength == 0);
}

static AudioBufferList* makeList(UInt32 count, UInt32 channelsPerBuffer, UInt32 bytes)
{
	AudioBufferList* list = calloc(1, offsetof(AudioBufferList, mBuffers) + count * sizeof(AudioBuffer));
	list->mNumberBuffers = count;
	for (UInt32 i = 0; i < count; i++)
	{
		list->mBuffers[i].mNumberChannels = channelsPerBuffer;
		list->mBuffers[i].mDataByteSize = bytes;
		list->mBuffers[i].mData = calloc(1, bytes ? bytes : 1);
	}
	return list;
}

static void freeList(AudioBufferList* list)
{
	for (UInt32 i = 0; i < list->mNumberBuffers; i++)
		free(list->mBuffers[i].mData);
	free(list);
}

static void testBufferListNoCopy(void)
{
	AVAudioFormat* format = makeFormat(AVAudioPCMFormatFloat32, 2, NO);

	AudioBufferList* list = makeList(2, 1, 64 * sizeof(float));
	__block int calls = 0;
	__block UInt32 seenBuffers = 0, seenSize0 = 0, seenSize1 = 0;
	__block void* seenData0 = NULL;
	__block void* seenData1 = NULL;
	void* data0 = list->mBuffers[0].mData;
	void* data1 = list->mBuffers[1].mData;

	@autoreleasepool
	{
		AVAudioPCMBuffer* buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format bufferListNoCopy:list
			deallocator:^(const AudioBufferList* given) {
				calls++;
				seenBuffers = given->mNumberBuffers;
				seenSize0 = given->mBuffers[0].mDataByteSize;
				seenSize1 = given->mBuffers[1].mDataByteSize;
				seenData0 = given->mBuffers[0].mData;
				seenData1 = given->mBuffers[1].mData;
			}];
		CHECK(buffer != nil);
		CHECK(buffer.frameCapacity == 64);
		// The samples are the caller's: the channel pointers are the buffers' data.
		CHECK(buffer.floatChannelData != NULL);
		CHECK(buffer.floatChannelData[0] == data0 && buffer.floatChannelData[1] == data1);
		CHECK(buffer.audioBufferList->mNumberBuffers == 2);
		CHECK(buffer.frameLength == 64);

		// Shortening the frame length changes the byte size the buffer reports, not the caller's list.
		buffer.frameLength = 16;
		CHECK(buffer.audioBufferList->mBuffers[0].mDataByteSize == 16 * sizeof(float));
		CHECK(list->mBuffers[0].mDataByteSize == 64 * sizeof(float));
		CHECK(calls == 0);
		[buffer release];
	}

	CHECK(calls == 1);
	CHECK(seenBuffers == 2);
	// QUIRK test, deallocator list carries the frame length (macOS 26.7.3): the list handed to the deallocator
	// has the buffer's current frame length in bytes, not the size originally given.
	CHECK(seenSize0 == 16 * sizeof(float) && seenSize1 == 16 * sizeof(float));
	CHECK(seenData0 == data0 && seenData1 == data1);
	freeList(list);

	// The deallocator is optional.
	list = makeList(2, 1, 32);
	AVAudioPCMBuffer* buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format bufferListNoCopy:list deallocator:nil];
	CHECK(buffer != nil);
	[buffer release];
	freeList(list);
}

// Whether the no-copy initializer accepts the list; an accepted buffer is released (it has no deallocator, so
// the list stays the caller's).
static BOOL accepts(AVAudioFormat* format, AudioBufferList* list)
{
	AVAudioPCMBuffer* buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:format bufferListNoCopy:list deallocator:nil];
	const BOOL accepted = buffer != nil;
	[buffer release];
	return accepted;
}

static void testBufferListNoCopyFailures(void)
{
	AVAudioFormat* format = makeFormat(AVAudioPCMFormatFloat32, 2, NO);
	AVAudioFormat* interleaved = makeFormat(AVAudioPCMFormatFloat32, 2, YES);

	// The buffer count must be the channel count for a format that is not interleaved, and 1 if it is.
	AudioBufferList* one = makeList(1, 1, 32);
	CHECK(!accepts(format, one));
	CHECK(accepts(interleaved, one));
	freeList(one);

	AudioBufferList* two = makeList(2, 1, 32);
	CHECK(!accepts(interleaved, two));

	// Unequal byte sizes.
	two->mBuffers[1].mDataByteSize = 16;
	CHECK(!accepts(format, two));

	// A zero byte size.
	two->mBuffers[0].mDataByteSize = 0;
	two->mBuffers[1].mDataByteSize = 0;
	CHECK(!accepts(format, two));

	// A NULL data pointer.
	two->mBuffers[0].mDataByteSize = 32;
	two->mBuffers[1].mDataByteSize = 32;
	void* kept = two->mBuffers[1].mData;
	two->mBuffers[1].mData = NULL;
	CHECK(!accepts(format, two));
	two->mBuffers[1].mData = kept;

	freeList(two);

	// No buffers at all, for either layout.
	AudioBufferList* none = makeList(0, 1, 32);
	CHECK(!accepts(format, none));
	CHECK(!accepts(interleaved, none));
	freeList(none);

	// A format with no bytes per frame.
	AudioStreamBasicDescription asbd = *format.streamDescription;
	asbd.mBytesPerFrame = asbd.mBytesPerPacket = 0;
	AVAudioFormat* zero = [[[AVAudioFormat alloc] initWithStreamDescription:&asbd] autorelease];
	AudioBufferList* list = makeList(2, 1, 32);
	CHECK(!accepts(zero, list));
	freeList(list);
}

int main(void)
{
	@autoreleasepool
	{
		testStandardFormat();
		testInterleavedInt16();
		testInt32AndOtherFormats();
		testCreationFailures();
		testBufferListNoCopy();
		testBufferListNoCopyFailures();
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
