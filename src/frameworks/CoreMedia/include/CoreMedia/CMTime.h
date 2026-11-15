/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#ifndef _CMTIME_H_
#define _CMTIME_H_

#include <CoreMedia/CMBase.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// The structures of this header are 4-byte aligned, so that a CMTime is 24
// bytes and keeps the layout applications were built with.
#pragma pack(push, 4)

typedef int64_t CMTimeValue;
typedef int32_t CMTimeScale;
typedef int64_t CMTimeEpoch;

typedef CF_OPTIONS(uint32_t, CMTimeFlags) {
	kCMTimeFlags_Valid = (1UL << 0),
	kCMTimeFlags_HasBeenRounded = (1UL << 1),
	kCMTimeFlags_PositiveInfinity = (1UL << 2),
	kCMTimeFlags_NegativeInfinity = (1UL << 3),
	kCMTimeFlags_Indefinite = (1UL << 4),
	kCMTimeFlags_ImpliedValueFlagsMask = kCMTimeFlags_PositiveInfinity | kCMTimeFlags_NegativeInfinity | kCMTimeFlags_Indefinite
};

typedef CF_ENUM(uint32_t, CMTimeRoundingMethod) {
	kCMTimeRoundingMethod_RoundHalfAwayFromZero = 1,
	kCMTimeRoundingMethod_RoundTowardZero = 2,
	kCMTimeRoundingMethod_RoundAwayFromZero = 3,
	kCMTimeRoundingMethod_QuickTime = 4,
	kCMTimeRoundingMethod_RoundTowardPositiveInfinity = 5,
	kCMTimeRoundingMethod_RoundTowardNegativeInfinity = 6,
	kCMTimeRoundingMethod_Default = kCMTimeRoundingMethod_RoundHalfAwayFromZero
};

// A rational time: value / timescale seconds, in the given epoch.
typedef struct
{
	CMTimeValue     value;
	CMTimeScale     timescale;
	CMTimeFlags     flags;
	CMTimeEpoch     epoch;
}
CMTime;

#pragma pack(pop)

extern const CMTime kCMTimeInvalid;
extern const CMTime kCMTimeIndefinite;
extern const CMTime kCMTimePositiveInfinity;
extern const CMTime kCMTimeNegativeInfinity;
extern const CMTime kCMTimeZero CM_SWIFT_NAME(CMTime.zero);

// Largest timescale a time can have.
#define kCMTimeMaxTimescale 0x7fffffffL

// A timescale that is not positive gives kCMTimeInvalid.
CMTime CMTimeMake(int64_t value, int32_t timescale) CM_SWIFT_NAME(CMTimeMake(value:timescale:));
CMTime CMTimeMakeWithEpoch(int64_t value, int32_t timescale, int64_t epoch) CM_SWIFT_NAME(CMTimeMakeWithEpoch(value:timescale:epoch:));
CMTime CMTimeMakeWithSeconds(Float64 seconds, int32_t preferredTimescale) CM_SWIFT_NAME(CMTimeMakeWithSeconds(_:preferredTimescale:));

// NaN for an invalid or indefinite time.
Float64 CMTimeGetSeconds(CMTime time) CM_SWIFT_NAME(CMTimeGetSeconds(_:));

CMTime CMTimeAdd(CMTime addend1, CMTime addend2) CM_SWIFT_NAME(CMTimeAdd(_:_:));
CMTime CMTimeSubtract(CMTime minuend, CMTime subtrahend) CM_SWIFT_NAME(CMTimeSubtract(_:_:));
CMTime CMTimeMultiply(CMTime time, int32_t multiplier) CM_SWIFT_NAME(CMTimeMultiply(_:multiplier:));
CMTime CMTimeConvertScale(CMTime time, int32_t newTimescale, CMTimeRoundingMethod method) CM_SWIFT_NAME(CMTimeConvertScale(_:timescale:method:));

// Orders invalid times above everything else; see CMTime.c for the full order.
int32_t CMTimeCompare(CMTime time1, CMTime time2) CM_SWIFT_NAME(CMTimeCompare(_:_:));
CMTime CMTimeMinimum(CMTime time1, CMTime time2) CM_SWIFT_NAME(CMTimeMinimum(_:_:));
CMTime CMTimeMaximum(CMTime time1, CMTime time2) CM_SWIFT_NAME(CMTimeMaximum(_:_:));
CMTime CMTimeAbsoluteValue(CMTime time) CM_SWIFT_NAME(CMTimeAbsoluteValue(_:));

#ifdef __cplusplus
}
#endif

#endif
