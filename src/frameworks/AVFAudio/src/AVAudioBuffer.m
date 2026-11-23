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

#import <AVFAudio/AVAudioBuffer.h>
#import "AVFAudioStub.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

@implementation AVAudioBuffer

- (AVAudioFormat*)format
{
	return _format;
}

- (const AudioBufferList*)audioBufferList
{
	return _audioBufferList;
}

// mutableAudioBufferList is a list of its own, not the audioBufferList one: it is refreshed from the buffer's list on
// every access, and what a caller writes into it (mDataByteSize included) does not reach the buffer. The frame length
// changes only through the PCM buffer's frameLength.
//
// QUIRK: the two properties return different lists whose contents are equal, and a write to the mutable list's
// mDataByteSize does not change the frame length (the page for the property says the fields express the capacity,
// the page for audioBufferList says the frame length; neither says the lists differ). Observed on macOS 26.7.3 and
// 10.14.4 (x86_64) after frameLength changes: both lists show the frame length in bytes, the pointers differ and stay
// the same between calls, and a changed byte size is overwritten at the next access (tier 2, black-box probe
// mac-probes/cristim/batch7a/avpcm2.m).
// Test: tests/src/avfaudio_pcm_buffer.m, "mutable list is a separate, refreshed list (quirk)".
- (AudioBufferList*)mutableAudioBufferList
{
	if (!_audioBufferList)
		return NULL;
	const size_t size = offsetof(AudioBufferList, mBuffers) + (size_t) _audioBufferList->mNumberBuffers * sizeof(AudioBuffer);
	if (!_mutableAudioBufferList)
		_mutableAudioBufferList = malloc(size);
	if (_mutableAudioBufferList)
		memcpy(_mutableAudioBufferList, _audioBufferList, size);
	return _mutableAudioBufferList;
}

- (void)dealloc
{
	[_format release];
	free(_mutableAudioBufferList);
	[super dealloc];
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
