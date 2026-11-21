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

#ifndef AVFAUDIO_AVAUDIOFORMAT_H
#define AVFAUDIO_AVAUDIOFORMAT_H

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudioTypes.h>
#import <AVFAudio/AVAudioTypes.h>

@class AVAudioChannelLayout;

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, AVAudioCommonFormat) {
	AVAudioOtherFormat = 0,
	AVAudioPCMFormatFloat32 = 1,
	AVAudioPCMFormatFloat64 = 2,
	AVAudioPCMFormatInt16 = 3,
	AVAudioPCMFormatInt32 = 4
};

// Describes the format of audio samples: an AudioStreamBasicDescription with a few conveniences.
@interface AVAudioFormat : NSObject <NSSecureCoding, NSCopying>
{
@private
	AudioStreamBasicDescription _asbd;
}

- (instancetype)initWithStreamDescription:(const AudioStreamBasicDescription *)asbd;
- (instancetype)initWithStreamDescription:(const AudioStreamBasicDescription *)asbd channelLayout:(nullable AVAudioChannelLayout *)layout;
- (nullable instancetype)initStandardFormatWithSampleRate:(double)sampleRate channels:(AVAudioChannelCount)channels;
- (instancetype)initWithCommonFormat:(AVAudioCommonFormat)format sampleRate:(double)sampleRate channels:(AVAudioChannelCount)channels interleaved:(BOOL)interleaved;
- (nullable instancetype)initWithSettings:(NSDictionary<NSString *, id> *)settings;

@property (nonatomic, readonly, getter=isStandard) BOOL standard;
@property (nonatomic, readonly) AVAudioCommonFormat commonFormat;
@property (nonatomic, readonly) AVAudioChannelCount channelCount;
@property (nonatomic, readonly) double sampleRate;
@property (nonatomic, readonly, getter=isInterleaved) BOOL interleaved;
@property (nonatomic, readonly) const AudioStreamBasicDescription *streamDescription;
@property (nonatomic, readonly, nullable) AVAudioChannelLayout *channelLayout;
@property (nonatomic, readonly) NSDictionary<NSString *, id> *settings;

@end

NS_ASSUME_NONNULL_END

#endif
