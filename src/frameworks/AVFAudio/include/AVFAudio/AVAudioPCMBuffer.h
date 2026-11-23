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

#ifndef AVFAUDIO_AVAUDIOPCMBUFFER_H
#define AVFAUDIO_AVAUDIOPCMBUFFER_H

#import <Foundation/Foundation.h>
#import <AVFAudio/AVAudioBuffer.h>
#import <AVFAudio/AVAudioTypes.h>

NS_ASSUME_NONNULL_BEGIN

// An audio buffer for PCM formats.
@interface AVAudioPCMBuffer : AVAudioBuffer
{
@private
	AVAudioFrameCount _frameCapacity;
	void** _channelData;
	void (^_deallocator)(const AudioBufferList*);
	BOOL _ownsSamples;
}

- (nullable instancetype)initWithPCMFormat:(AVAudioFormat*)format frameCapacity:(AVAudioFrameCount)frameCapacity;
- (nullable instancetype)initWithPCMFormat:(AVAudioFormat*)format
	bufferListNoCopy:(const AudioBufferList*)bufferList
	deallocator:(void (^_Nullable)(const AudioBufferList*))deallocator;

@property (nonatomic, readonly) AVAudioFrameCount frameCapacity;
@property (nonatomic) AVAudioFrameCount frameLength;

// The number of interleaved channels: 1 for a format that is not interleaved.
@property (nonatomic, readonly) NSUInteger stride;

// One pointer per channel to that channel's samples, or NULL if the format's samples are of another type.
@property (nonatomic, readonly) float* _Nonnull const* _Nullable floatChannelData;
@property (nonatomic, readonly) int16_t* _Nonnull const* _Nullable int16ChannelData;
@property (nonatomic, readonly) int32_t* _Nonnull const* _Nullable int32ChannelData;

@end

NS_ASSUME_NONNULL_END

#endif
