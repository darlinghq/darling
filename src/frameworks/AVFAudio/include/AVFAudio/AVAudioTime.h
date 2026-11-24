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

#ifndef AVFAUDIO_AVAUDIOTIME_H
#define AVFAUDIO_AVAUDIOTIME_H

#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudioTypes.h>
#import <AVFAudio/AVAudioTypes.h>

NS_ASSUME_NONNULL_BEGIN

// A moment in time as a host time, as a position in an audio stream, or as both. Instances are immutable.
@interface AVAudioTime : NSObject
{
@private
	AudioTimeStamp _ats;
	double _sampleRate;
	void *_reserved;
}

- (instancetype)initWithAudioTimeStamp:(const AudioTimeStamp *)ts sampleRate:(double)sampleRate;
- (instancetype)initWithHostTime:(uint64_t)hostTime;
- (instancetype)initWithSampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate;
- (instancetype)initWithHostTime:(uint64_t)hostTime sampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate;

+ (instancetype)timeWithAudioTimeStamp:(const AudioTimeStamp *)ts sampleRate:(double)sampleRate;
+ (instancetype)timeWithHostTime:(uint64_t)hostTime;
+ (instancetype)timeWithSampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate;
+ (instancetype)timeWithHostTime:(uint64_t)hostTime sampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate;

- (nullable AVAudioTime *)extrapolateTimeFromAnchor:(AVAudioTime *)anchorTime;

+ (uint64_t)hostTimeForSeconds:(NSTimeInterval)seconds;
+ (NSTimeInterval)secondsForHostTime:(uint64_t)hostTime;

@property (nonatomic, readonly) uint64_t hostTime;
@property (nonatomic, readonly, getter=isHostTimeValid) BOOL hostTimeValid;
@property (nonatomic, readonly) AVAudioFramePosition sampleTime;
@property (nonatomic, readonly) double sampleRate;
@property (nonatomic, readonly, getter=isSampleTimeValid) BOOL sampleTimeValid;
@property (nonatomic, readonly) AudioTimeStamp audioTimeStamp;

@end

NS_ASSUME_NONNULL_END

#endif
