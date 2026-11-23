/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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

#import <AVFAudio/AVAudioPCMBuffer.h>
#import "AVFAudioStub.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// A buffer list with room for count buffers, zero-filled.
static AudioBufferList* allocateBufferList(UInt32 count)
{
	const size_t size = offsetof(AudioBufferList, mBuffers) + (size_t) count * sizeof(AudioBuffer);
	AudioBufferList* list = calloc(1, size);
	if (list)
		list->mNumberBuffers = count;
	return list;
}

// The number of buffers in the list for the format: one for interleaved samples, else one per channel.
static UInt32 bufferCountForFormat(AVAudioFormat* format)
{
	return format.interleaved ? 1 : format.channelCount;
}

// Builds the per-channel pointers that floatChannelData and the other accessors return. The pages say each
// pointer in an interleaved buffer is "offset by 1 frame" while the data is spaced by stride samples, which
// only addresses the channels consistently if the offset is one sample, so one sample is used.
// Returns NULL, without error, for a format whose samples are none of float32, int16 or int32.
static void** createChannelPointers(AVAudioFormat* format, AudioBufferList* list)
{
	switch (format.commonFormat)
	{
		case AVAudioPCMFormatFloat32:
		case AVAudioPCMFormatInt16:
		case AVAudioPCMFormatInt32:
			break;
		default:
			return NULL;
	}

	const AudioStreamBasicDescription* asbd = format.streamDescription;
	void** pointers = calloc(asbd->mChannelsPerFrame, sizeof(void*));
	for (UInt32 i = 0; pointers && i < asbd->mChannelsPerFrame; i++)
	{
		if (format.interleaved)
			pointers[i] = (char*) list->mBuffers[0].mData + (size_t) i * (asbd->mBitsPerChannel / 8);
		else
			pointers[i] = list->mBuffers[i].mData;
	}
	return pointers;
}

@implementation AVAudioPCMBuffer

- (instancetype)initWithPCMFormat:(AVAudioFormat*)format frameCapacity:(AVAudioFrameCount)frameCapacity
{
	const AudioStreamBasicDescription* asbd = format.streamDescription;

	// The pages give two reasons for nil: no bytes per frame, and a byte capacity that does not fit 32 bits.
	// GUESS: a nil format fails the same way; the pages do not say.
	if (!asbd || asbd->mBytesPerFrame == 0 || (uint64_t) frameCapacity * asbd->mBytesPerFrame > UINT32_MAX)
	{
		[self release];
		return nil;
	}

	const UInt32 count = bufferCountForFormat(format);
	const UInt32 byteCapacity = frameCapacity * asbd->mBytesPerFrame;
	AudioBufferList* list = allocateBufferList(count);
	BOOL allocated = list != NULL;
	for (UInt32 i = 0; allocated && i < count; i++)
	{
		// Zero-filled, and never NULL, even for a capacity of zero.
		list->mBuffers[i].mData = calloc(1, byteCapacity ? byteCapacity : 1);
		list->mBuffers[i].mNumberChannels = format.interleaved ? asbd->mChannelsPerFrame : 1;
		allocated = list->mBuffers[i].mData != NULL;
	}

	if (allocated && (self = [super init]))
	{
		_format = [format retain];
		_audioBufferList = list;
		_frameCapacity = frameCapacity;
		_ownsSamples = YES;
		_channelData = createChannelPointers(format, list);
		// GUESS: the pages say the frame length has no useful value on creation, so it starts at zero.
		// Its bytes are the buffers' mDataByteSize, which start at zero to match.
		return self;
	}

	if (list)
	{
		for (UInt32 i = 0; i < count; i++)
			free(list->mBuffers[i].mData);
		free(list);
	}
	[self release];
	return nil;
}

- (instancetype)initWithPCMFormat:(AVAudioFormat*)format
	bufferListNoCopy:(const AudioBufferList*)bufferList
	deallocator:(void (^)(const AudioBufferList*))deallocator
{
	const AudioStreamBasicDescription* asbd = format.streamDescription;

	// The reasons for nil are those on the page. GUESS: a nil format or a NULL list fail the same way.
	BOOL valid = asbd && bufferList && asbd->mBytesPerFrame != 0 && bufferList->mNumberBuffers != 0
		&& bufferList->mNumberBuffers == bufferCountForFormat(format);
	for (UInt32 i = 0; valid && i < bufferList->mNumberBuffers; i++)
		valid = bufferList->mBuffers[i].mData != NULL && bufferList->mBuffers[i].mDataByteSize != 0
			&& bufferList->mBuffers[i].mDataByteSize == bufferList->mBuffers[0].mDataByteSize;

	AudioBufferList* working = NULL;
	if (valid)
	{
		const size_t size = offsetof(AudioBufferList, mBuffers) + (size_t) bufferList->mNumberBuffers * sizeof(AudioBuffer);
		working = malloc(size);
		valid = working != NULL;
		if (valid)
			memcpy(working, bufferList, size);
	}

	if (valid && (self = [super init]))
	{
		_format = [format retain];
		// The buffer works on a copy of the list: setting the frame length changes the copy's mDataByteSize,
		// never the caller's list.
		_audioBufferList = working;
		_deallocator = [deallocator copy];
		// GUESS: the capacity is the byte size divided by the bytes per frame, truncated, and the frame length
		// starts equal to it because mDataByteSize is left as given. The pages do not say either.
		_frameCapacity = bufferList->mBuffers[0].mDataByteSize / asbd->mBytesPerFrame;
		_channelData = createChannelPointers(format, working);
		return self;
	}

	free(working);
	[self release];
	return nil;
}

- (void)dealloc
{
	if (_ownsSamples)
	{
		for (UInt32 i = 0; i < _audioBufferList->mNumberBuffers; i++)
			free(_audioBufferList->mBuffers[i].mData);
	}
	// GUESS: without a deallocator the caller's samples are left alone; the page does not say what happens.
	else if (_deallocator)
	{
		// QUIRK: the deallocator gets a list that is not the one the caller passed in, and whose mDataByteSize is
		// the current frame length in bytes, not the size originally given. Observed on macOS 26.7.3 (x86_64):
		// a 64-byte buffer whose frameLength was set to 3 frames of 4 bytes hands the deallocator 12 bytes, a
		// buffer whose frameLength was untouched hands it 64, and the data pointers are the caller's (tier 2,
		// black-box probe mac-probes/cristim/batch7a/avpcm2.m; macOS 10.14.4 has no such initializer).
		// Test: tests/src/avfaudio_pcm_buffer.m, "deallocator list carries the frame length (quirk)".
		_deallocator(_audioBufferList);
	}
	[_deallocator release];
	free(_audioBufferList);
	free(_channelData);
	[super dealloc];
}

- (AVAudioFrameCount)frameCapacity
{
	return _frameCapacity;
}

- (AVAudioFrameCount)frameLength
{
	if (!_audioBufferList)
		return 0;
	return _audioBufferList->mBuffers[0].mDataByteSize / _format.streamDescription->mBytesPerFrame;
}

// GUESS: the pages require a length no larger than the capacity but do not say what happens otherwise.
// Raising an exception makes the mistake visible instead of recording a length that overruns the memory.
- (void)setFrameLength:(AVAudioFrameCount)frameLength
{
	if (frameLength > _frameCapacity)
		[NSException raise:NSInvalidArgumentException
			format:@"frameLength %u is larger than frameCapacity %u", (unsigned) frameLength, (unsigned) _frameCapacity];

	if (!_audioBufferList)
		return;

	const UInt32 byteLength = frameLength * _format.streamDescription->mBytesPerFrame;
	for (UInt32 i = 0; i < _audioBufferList->mNumberBuffers; i++)
		_audioBufferList->mBuffers[i].mDataByteSize = byteLength;
}

- (NSUInteger)stride
{
	return _format.interleaved ? _format.channelCount : 1;
}

- (float* const*)floatChannelData
{
	return _format.commonFormat == AVAudioPCMFormatFloat32 ? (float* const*) _channelData : NULL;
}

// GUESS: int16ChannelData and int32ChannelData are keyed on the common format, like floatChannelData. The pages
// say "2-byte integer samples" and "4-byte integer samples", which this takes to mean the common formats Int16
// and Int32 (signed, packed, native endian).
- (int16_t* const*)int16ChannelData
{
	return _format.commonFormat == AVAudioPCMFormatInt16 ? (int16_t* const*) _channelData : NULL;
}

- (int32_t* const*)int32ChannelData
{
	return _format.commonFormat == AVAudioPCMFormatInt32 ? (int32_t* const*) _channelData : NULL;
}

// Methods of the real class that are not implemented yet still have to be accepted.
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return AVAudioStubSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
