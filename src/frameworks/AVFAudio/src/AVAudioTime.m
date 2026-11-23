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

#import <AVFAudio/AVAudioTime.h>
#import <CoreAudio/HostTime.h>
#import "AVFAudioStub.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

// sampleTime is an integer position (AVAudioFramePosition) while an AudioTimeStamp keeps its sample time as
// a floating-point number.
// GUESS: the pages do not say how one becomes the other; it is truncated toward zero, saturates at the
// ends of the integer range, and NaN gives 0.
static AVAudioFramePosition framePosition(double sampleTime)
{
	if (sampleTime != sampleTime)
		return 0;
	if (sampleTime >= 0x1p63)
		return INT64_MAX;
	if (sampleTime <= -0x1p63)
		return INT64_MIN;
	return (AVAudioFramePosition) sampleTime;
}

@implementation AVAudioTime

- (instancetype)initWithAudioTimeStamp:(const AudioTimeStamp *)ts sampleRate:(double)sampleRate
{
	// GUESS: the page takes a non-optional pointer and does not cover NULL; it gives nil, as the other
	// initializers of this framework do for input they cannot use.
	if (ts == NULL)
	{
		[self release];
		return nil;
	}
	if ((self = [super init]))
	{
		_ats = *ts;
		// GUESS: a field whose validity flag is clear has no meaning; it reads as 0 (hostTime, sampleTime and
		// audioTimeStamp), so that two times with the same valid fields read the same. The pages do not say.
		if (!(ts->mFlags & kAudioTimeStampHostTimeValid))
			_ats.mHostTime = 0;
		if (!(ts->mFlags & kAudioTimeStampSampleTimeValid))
			_ats.mSampleTime = 0;
		// GUESS: the other fields of the time stamp (rate scalar, word clock, SMPTE time) are kept as they
		// are, so audioTimeStamp returns what the object was made from. The pages do not say.
		_sampleRate = sampleRate;
	}
	return self;
}

- (instancetype)initWithHostTime:(uint64_t)hostTime
{
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mHostTime = hostTime;
	ts.mFlags = kAudioTimeStampHostTimeValid;
	// GUESS: the page gives no sample rate, so the sample rate is 0 (sampleTime is not valid either).
	return [self initWithAudioTimeStamp:&ts sampleRate:0];
}

- (instancetype)initWithSampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate
{
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mSampleTime = (Float64) sampleTime;
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	return [self initWithAudioTimeStamp:&ts sampleRate:sampleRate];
}

- (instancetype)initWithHostTime:(uint64_t)hostTime sampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate
{
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mHostTime = hostTime;
	ts.mSampleTime = (Float64) sampleTime;
	ts.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampSampleTimeValid;
	return [self initWithAudioTimeStamp:&ts sampleRate:sampleRate];
}

+ (instancetype)timeWithAudioTimeStamp:(const AudioTimeStamp *)ts sampleRate:(double)sampleRate
{
	return [[[self alloc] initWithAudioTimeStamp:ts sampleRate:sampleRate] autorelease];
}

+ (instancetype)timeWithHostTime:(uint64_t)hostTime
{
	return [[[self alloc] initWithHostTime:hostTime] autorelease];
}

+ (instancetype)timeWithSampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate
{
	return [[[self alloc] initWithSampleTime:sampleTime atRate:sampleRate] autorelease];
}

+ (instancetype)timeWithHostTime:(uint64_t)hostTime sampleTime:(AVAudioFramePosition)sampleTime atRate:(double)sampleRate
{
	return [[[self alloc] initWithHostTime:hostTime sampleTime:sampleTime atRate:sampleRate] autorelease];
}

- (uint64_t)hostTime
{
	return _ats.mHostTime;
}

- (BOOL)isHostTimeValid
{
	return (_ats.mFlags & kAudioTimeStampHostTimeValid) != 0;
}

- (AVAudioFramePosition)sampleTime
{
	return framePosition(_ats.mSampleTime);
}

- (double)sampleRate
{
	return _sampleRate;
}

- (BOOL)isSampleTimeValid
{
	return (_ats.mFlags & kAudioTimeStampSampleTimeValid) != 0;
}

- (AudioTimeStamp)audioTimeStamp
{
	return _ats;
}

// The host time clock of this class is the one of CoreAudio's HostTime.h (mach_absolute_time, which the
// class page names as the host time); the conversions go through it.
+ (uint64_t)hostTimeForSeconds:(NSTimeInterval)seconds
{
	// GUESS: the page does not cover a negative or NaN number of seconds, which no host time can represent;
	// they give 0. A number of seconds beyond what 64 bits of nanoseconds hold is limited to UINT64_MAX
	// nanoseconds before the conversion; CoreAudio's conversion wraps for such a value when the timebase
	// has more denominator than numerator (it is exact for 1/1), which is not guarded here.
	// The nanoseconds round to the nearest, as the page does not say how a fraction of one is handled.
	if (!(seconds > 0))
		return 0;
	const double nanos = seconds * 1e9 + 0.5;
	return AudioConvertNanosToHostTime(nanos >= 0x1p64 ? UINT64_MAX : (UInt64) nanos);
}

+ (NSTimeInterval)secondsForHostTime:(uint64_t)hostTime
{
	return (double) AudioConvertHostTimeToNanos(hostTime) / 1e9;
}

- (AVAudioTime *)extrapolateTimeFromAnchor:(AVAudioTime *)anchorTime
{
	const BOOL hostValid = self.hostTimeValid;
	const BOOL sampleValid = self.sampleTimeValid;

	// The page requires valid host and sample times in the anchor, and a sample rate and at least one valid
	// time in the receiver. GUESS: "have a sample rate" is a rate above 0 (initWithHostTime: leaves it 0);
	// the sampleTimeValid page says the flag covers "the sample time and sample rate properties", so a
	// receiver that has only a valid host time can have a rate only from initWithAudioTimeStamp:sampleRate:.
	if (anchorTime == nil || !anchorTime.hostTimeValid || !anchorTime.sampleTimeValid)
		return nil;
	if (!(_sampleRate > 0) || (!hostValid && !sampleValid))
		return nil;

	// GUESS: the page and the header do not cover an anchor whose sample rate differs from the receiver's.
	// The anchor's sample time is converted to the receiver's rate before the two are combined, so the same
	// distance in seconds gives the same result at any rate (the alternative, refusing with nil, was
	// rejected). An anchor without a rate above 0 is taken to be counted at the receiver's rate.
	const double anchorRate = anchorTime.sampleRate;
	const double anchorFrames = (anchorRate > 0 && anchorRate != _sampleRate)
		? (double) anchorTime.sampleTime * _sampleRate / anchorRate
		: (double) anchorTime.sampleTime;

	// GUESS: a receiver with both times valid has nothing to fill in; the page covers only the case of
	// one valid time, so it gives an equal time. The result of an extrapolation that cannot be
	// represented in 64 bits is nil, which the page does not cover either.
	// GUESS: the result copies the receiver's time stamp and fills in only the host time or the sample
	// time; the header says fields the anchor provides are valid too, which would include its rate scalar,
	// word clock and SMPTE time, but says nothing more, so they are not taken from the anchor.
	AudioTimeStamp stamp = _ats;
	if (!hostValid)
	{
		const double seconds = ((double) self.sampleTime - anchorFrames) / _sampleRate;
		if (!isfinite(seconds))
			return nil;
		const uint64_t ticks = [AVAudioTime hostTimeForSeconds:fabs(seconds)];
		uint64_t host;
		const BOOL overflow = seconds < 0 ? __builtin_sub_overflow(anchorTime.hostTime, ticks, &host)
		                                  : __builtin_add_overflow(anchorTime.hostTime, ticks, &host);
		if (overflow)
			return nil;
		stamp.mHostTime = host;
		stamp.mFlags |= kAudioTimeStampHostTimeValid;
	}
	else if (!sampleValid)
	{
		const uint64_t host = _ats.mHostTime, anchorHost = anchorTime.hostTime;
		const double seconds = host >= anchorHost ? [AVAudioTime secondsForHostTime:host - anchorHost]
		                                          : -[AVAudioTime secondsForHostTime:anchorHost - host];
		// GUESS: the sample time rounds to the nearest frame (halves away from zero).
		const double position = round(anchorFrames + seconds * _sampleRate);
		if (!(fabs(position) < 0x1p62))
			return nil;
		stamp.mSampleTime = (Float64) (AVAudioFramePosition) position;
		stamp.mFlags |= kAudioTimeStampSampleTimeValid;
	}
	return [[[[self class] alloc] initWithAudioTimeStamp:&stamp sampleRate:_sampleRate] autorelease];
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
