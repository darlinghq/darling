//
// This file is part of Darling.
//
// Copyright (C) 2026 Lubos Dolezel
//
// Darling is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Darling is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Darling.  If not, see <http://www.gnu.org/licenses/>.

// Typecheck test: the Swift names the CoreMedia headers give the time and time range API, as the
// Swift CoreMedia overlay (CMTime.swift, CMTimeRange.swift) uses them. Not run, only compiled:
//   swiftc -frontend -typecheck tests/src/coremedia_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import CoreMedia

func timeCalls() -> Double {
    let a = CMTimeMake(value: 1, timescale: 3)
    let b = CMTimeMakeWithSeconds(0.25, preferredTimescale: 600)
    let e = CMTimeMakeWithEpoch(value: 1, timescale: 3, epoch: 2)
    let sum = CMTimeAdd(a, b)
    let diff = CMTimeSubtract(a, b)
    let product = CMTimeMultiply(a, multiplier: 3)
    let converted = CMTimeConvertScale(sum, timescale: 600, method: .roundHalfAwayFromZero)
    let defaulted = CMTimeConvertScale(sum, timescale: 600, method: .default)
    let order: Int32 = CMTimeCompare(a, b)
    let lo = CMTimeMinimum(a, b)
    let hi = CMTimeMaximum(a, b)
    let magnitude = CMTimeAbsoluteValue(diff)
    let zero: CMTime = .zero
    let flags: CMTimeFlags = [.valid, .hasBeenRounded]
    let mask: CMTimeFlags = .impliedValueFlagsMask
    let seconds: Float64 = CMTimeGetSeconds(CMTimeAdd(converted, defaulted))
    _ = (e, product, order, lo, hi, magnitude, zero, flags, mask, kCMTimeInvalid, kCMTimeIndefinite,
         kCMTimePositiveInfinity, kCMTimeNegativeInfinity, kCMTimeMaxTimescale)
    let methods: [CMTimeRoundingMethod] = [.roundHalfAwayFromZero, .roundTowardZero, .roundAwayFromZero,
                                           .quickTime, .roundTowardPositiveInfinity, .roundTowardNegativeInfinity]
    _ = methods
    return seconds
}

func rangeCalls() -> Bool {
    let start = CMTimeMake(value: 2, timescale: 1)
    let range = CMTimeRange(start: start, duration: CMTimeMake(value: 3, timescale: 1))
    let other = CMTimeRangeFromTimeToTime(start: start, end: CMTimeMake(value: 9, timescale: 1))
    let end: CMTime = CMTimeRangeGetEnd(range)
    let union = CMTimeRangeGetUnion(range, otherRange: other)
    let both = CMTimeRangeGetIntersection(range, otherRange: other)
    let made = CMTimeRangeMake(start: start, duration: end)
    let clamped = CMTimeClampToRange(CMTimeMake(value: 9, timescale: 1), range: range)
    _ = (union, both, made, clamped, kCMTimeRangeZero, kCMTimeRangeInvalid)
    return CMTimeRangeContainsTime(range, time: start)
        && CMTimeRangeContainsTimeRange(range, otherRange: range)
        && CMTimeRangeEqual(range, range)
}

func sampleTypes(_ buffer: CMSampleBuffer?, _ format: CMFormatDescription?) -> CMSampleTimingInfo {
    var info = kCMTimingInfoInvalid
    info.duration = CMTimeMake(value: 1, timescale: 30)
    info.presentationTimeStamp = .zero
    let count: CMItemCount = 1
    let index: CMItemIndex = 0
    _ = (buffer, format, count, index)
    return info
}
