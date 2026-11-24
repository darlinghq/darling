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

#ifndef AVFAUDIO_AVAUDIOBUFFER_H
#define AVFAUDIO_AVAUDIOBUFFER_H

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudioTypes.h>
#import <AVFAudio/AVAudioFormat.h>

NS_ASSUME_NONNULL_BEGIN

// Audio samples together with the AVAudioFormat that describes them. PCM samples are held by the subclass AVAudioPCMBuffer.
@interface AVAudioBuffer : NSObject
{
@protected
	AVAudioFormat* _format;
	AudioBufferList* _audioBufferList;
	AudioBufferList* _mutableAudioBufferList;
}

@property (nonatomic, readonly) AVAudioFormat* format;

// The AudioBufferList behind the buffer, for Core Audio and Audio Toolbox calls. Callers may change the samples but not
// the structure. mDataByteSize is the current frame length in bytes, for both properties; the mutable list is a separate
// list that is brought up to date on every access, see AVAudioBuffer.m.
@property (nonatomic, readonly) const AudioBufferList* audioBufferList;
@property (nonatomic, readonly) AudioBufferList* mutableAudioBufferList;

@end

NS_ASSUME_NONNULL_END

#endif
