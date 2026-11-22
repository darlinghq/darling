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

@implementation AVAudioBuffer

- (AVAudioFormat*)format
{
	return _format;
}

- (const AudioBufferList*)audioBufferList
{
	return _audioBufferList;
}

// GUESS: the audioBufferList page says the mDataByteSize fields express the frame length and the
// mutableAudioBufferList page says they express the frame capacity. They cannot both hold for one list, so both
// properties return the one list, with the frame length (the PCM buffer's frameLength page says changing
// frameLength updates mDataByteSize "and vice versa").
- (AudioBufferList*)mutableAudioBufferList
{
	return _audioBufferList;
}

- (void)dealloc
{
	[_format release];
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
