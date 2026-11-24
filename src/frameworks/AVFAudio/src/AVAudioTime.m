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
//
// QUIRK: the conversion truncates toward zero, and a NaN or a number outside the range of the integer gives the
// smallest integer. The pages do not say how one becomes the other. Evidence: black-box observation on macOS 26.7.3 and
// 10.14.4 (x86_64 and i386), tier 2: 3.7 reads 3, -3.7 reads -3, 0.5 and -0.5 read 0, and 1e30, -1e30, 9.3e18, infinity and NaN
// read INT64_MIN (the integer indefinite value of the x86 conversion; no arm64 observation).
// Test: tests/src/avfaudio_audio_time.m, testAudioTimeStamp.
static AVAudioFramePosition framePosition(double sampleTime)
{
	if (!(sampleTime > -0x1p63 && sampleTime < 0x1p63))
		return INT64_MIN;
	return (AVAudioFramePosition) sampleTime;
}

// A number of nanoseconds, as a double, to the 64-bit unsigned number the host clock functions take.
//
// QUIRK: the conversion truncates; a negative number wraps to the unsigned number of the same bits (-1 s is
// 2^64 - 10^9 ns); a NaN and minus infinity give 2^63; and a number from 2^64 up, and infinity, give 0. The pages do not cover
// these inputs. Evidence: black-box observation of +hostTimeForSeconds: on macOS 10.14.4 (x86_64 and i386) and
// 26.7.3 (x86_64), tier 2: 1.6e-9 s gives 1, -1 s gives 18446744072709551616, 1.8e10 s gives 1.8e19, 1.9e10 s, 1e300 s and
// infinity give 0, minus infinity and NaN give 9223372036854775808. 26.7.3 gives 2^63 instead of 0 from 2^64 up; the
// behaviour of 10.14.4 is followed. The same conversion is used for the host time of an extrapolation.
// Test: tests/src/avfaudio_audio_time.m, testHostTimeConversion.
static uint64_t nanosFromDouble(double nanos)
{
	if (nanos != nanos)
		return (uint64_t) 0x1p63;
	if (nanos < 0x1p63)
		return nanos >= -0x1p63 ? (uint64_t) (int64_t) nanos : (uint64_t) 0x1p63;
	const double rest = nanos - 0x1p63;
	return rest < 0x1p63 ? ((uint64_t) (int64_t) rest ^ (uint64_t) 0x1p63) : (uint64_t) 0;
}

@implementation AVAudioTime

- (instancetype)initWithAudioTimeStamp:(const AudioTimeStamp *)ts sampleRate:(double)sampleRate
{
	// The page takes a non-optional pointer and does not cover NULL, which crashes on macOS 26.7.3 and 10.14.4 (observed); it
	// gives nil here, as the other initializers of this framework do for input they cannot use.
	if (ts == NULL)
	{
		[self release];
		return nil;
	}
	if ((self = [super init]))
	{
		// The time stamp is kept as it is given, including the fields whose validity flag is clear and the fields
		// beyond the two times (rate scalar, word clock, SMPTE time): audioTimeStamp, hostTime and sampleTime read them
		// back. Observed on macOS 26.7.3 and 10.14.4 (a clear flag does not zero the field).
		_ats = *ts;
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
	// The page gives no sample rate, so the sample rate is 0, and the sample time is not valid (observed on macOS 26.7.3 and 10.14.4).
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
	return AudioConvertNanosToHostTime(nanosFromDouble(seconds * 1e9));
}

+ (NSTimeInterval)secondsForHostTime:(uint64_t)hostTime
{
	return (double) AudioConvertHostTimeToNanos(hostTime) / 1e9;
}

// The page requires valid host and sample times in the anchor, and a sample rate and at least one valid time in the
// receiver; the result has both. Observed on macOS 26.7.3 and 10.14.4 (black-box, tier 2), where the page is silent:
//  - a sample rate of 0 is no rate (nil); a negative rate works, as the division it is;
//  - the result is a copy of the receiver's time stamp (its other fields and flags too) with the missing time filled in;
//    a receiver with both times valid gives such a copy as it is;
//  - the sample time is counted from the anchor's sample time, in the receiver's sample rate: the anchor's own sample rate
//    is not used (an anchor with another rate or none gives the same result);
//  - a rate scalar scales the result: the receiver's if its time stamp has one (flag set), else the anchor's if it has
//    one, else 1; the host time is the anchor's plus (frames / rate) * scalar seconds and the sample time is the anchor's
//    plus seconds * rate / scalar frames, rounded to the nearest frame with halves to even. A scalar taken from the anchor
//    is stored in the result with its flag; the anchor's word clock and SMPTE time are not copied;
//  - the host time difference is the signed difference of the two host times (a host time above the anchor's by more than
//    2^63 counts as before it), and the host time is computed in nanoseconds as a double, so it wraps, as nanosFromDouble
//    says, instead of failing, when the time is before the start of the clock or beyond its end.
// Test: tests/src/avfaudio_audio_time.m, testExtrapolate*.
- (AVAudioTime *)extrapolateTimeFromAnchor:(AVAudioTime *)anchorTime
{
	if (anchorTime == nil || !anchorTime.hostTimeValid || !anchorTime.sampleTimeValid)
		return nil;
	const BOOL hostValid = (_ats.mFlags & kAudioTimeStampHostTimeValid) != 0;
	const BOOL sampleValid = (_ats.mFlags & kAudioTimeStampSampleTimeValid) != 0;
	if (_sampleRate == 0 || (!hostValid && !sampleValid))
		return nil;

	const AudioTimeStamp anchor = anchorTime.audioTimeStamp;
	AudioTimeStamp stamp = _ats;
	double scalar = 1.0;
	if (stamp.mFlags & kAudioTimeStampRateScalarValid)
		scalar = stamp.mRateScalar;
	else if (anchor.mFlags & kAudioTimeStampRateScalarValid)
	{
		scalar = anchor.mRateScalar;
		stamp.mRateScalar = scalar;
		stamp.mFlags |= kAudioTimeStampRateScalarValid;
	}

	if (!hostValid)
	{
		const double seconds = (stamp.mSampleTime - anchor.mSampleTime) / _sampleRate * scalar;
		const double nanos = (double) AudioConvertHostTimeToNanos(anchor.mHostTime) + seconds * 1e9;
		stamp.mHostTime = AudioConvertNanosToHostTime(nanosFromDouble(nanos));
		stamp.mFlags |= kAudioTimeStampHostTimeValid;
	}
	else if (!sampleValid)
	{
		const int64_t ticks = (int64_t) (stamp.mHostTime - anchor.mHostTime);
		const double nanos = ticks >= 0 ? (double) AudioConvertHostTimeToNanos((uint64_t) ticks)
		                                : -(double) AudioConvertHostTimeToNanos((uint64_t) 0 - (uint64_t) ticks);
		stamp.mSampleTime = nearbyint(anchor.mSampleTime + nanos / 1e9 * _sampleRate / scalar);
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
