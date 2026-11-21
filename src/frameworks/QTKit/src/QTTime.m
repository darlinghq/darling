/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#import <QTKit/QTTime.h>
#import <QTKit/QTTimeRange.h>
#import "QTCMTime.h"
#import <limits.h>
#import <math.h>
#import <stdio.h>
#import <string.h>

// Observed behavior of QTKit's time functions that the documentation does not cover is marked
// QUIRK: below. The evidence for all of them is black-box observation of the real framework on
// macOS 10.14.4 (i386) through the public functions (tier 2), pinned by tests/qtkit-time.m, which
// runs unchanged against the real QTKit.

#define QT_TIME_SCALE_FALLBACK 1000000000L

static QTTime QTInvalidTime(void)
{
    return QTIndefiniteTime;
}

// value * multiplier / divisor, truncated toward zero. NO if the result does not fit. Exact while the
// product fits in 64 bits; beyond that long double's 64-bit mantissa is as close as the platform
// gets without 128-bit integers (i386 has none).
static BOOL QTMultiplyDivide(long long value, long long multiplier, long long divisor, long long *result)
{
    long long product;
    if (!__builtin_mul_overflow(value, multiplier, &product)) {
        *result = product / divisor;
        return YES;
    }
    long double approximate = (long double)value * (long double)multiplier / (long double)divisor;
    if (approximate >= 9223372036854775807.0L || approximate <= -9223372036854775807.0L)
        return NO;
    *result = (long long)approximate;
    return YES;
}

static BOOL QTTimeIsInvalid(QTTime time)
{
    return (time.flags & kQTTimeIsIndefinite) != 0 || time.timeScale <= 0;
}

QTTime QTMakeTime(long long timeValue, long timeScale)
{
    // QUIRK: a time scale of 0 gives the indefinite time. Test: qtkit-time "make time with a zero scale".
    if (timeScale == 0)
        return QTIndefiniteTime;
    QTTime time;
    time.timeValue = timeValue;
    time.timeScale = timeScale;
    time.flags = 0;
    return time;
}

QTTime QTMakeTimeScaled(QTTime time, long timeScale)
{
    // QUIRK: an indefinite time, or a target scale that is not positive, gives the indefinite time;
    // the value is truncated toward zero, not rounded. Test: qtkit-time "make time scaled".
    if (QTTimeIsInvalid(time) || timeScale <= 0)
        return QTIndefiniteTime;
    long long value;
    if (!QTMultiplyDivide(time.timeValue, timeScale, time.timeScale, &value))
        return QTIndefiniteTime;
    return QTMakeTime(value, timeScale);
}

QTTime QTMakeTimeWithTimeInterval(NSTimeInterval timeInterval)
{
    // QUIRK: the result has a time scale of 1000000, the fraction beyond that is truncated, and a
    // value that does not fit gives the indefinite time. Test: qtkit-time "time interval".
    const long scale = 1000000;
    double scaled = timeInterval * (double)scale;
    if (isnan(scaled) || scaled >= 9223372036854775808.0 || scaled <= -9223372036854775808.0)
        return QTIndefiniteTime;
    return QTMakeTime((long long)scaled, scale);
}

QTTime QTMakeTimeWithTimeRecord(TimeRecord timeRecord)
{
    long long value = ((long long)timeRecord.value.hi << 32) | (long long)(unsigned int)timeRecord.value.lo;
    return QTMakeTime(value, (long)timeRecord.scale);
}

BOOL QTGetTimeInterval(QTTime time, NSTimeInterval *timeIntervalPtr)
{
    if (QTTimeIsInvalid(time)) {
        if (timeIntervalPtr)
            *timeIntervalPtr = 0.0;
        return NO;
    }
    if (timeIntervalPtr)
        *timeIntervalPtr = (NSTimeInterval)time.timeValue / (NSTimeInterval)time.timeScale;
    return YES;
}

TimeRecord QTGetTimeRecord(QTTime time)
{
    TimeRecord record;
    memset(&record, 0, sizeof(record));
    record.value.hi = (SInt32)(time.timeValue >> 32);
    record.value.lo = (UInt32)(time.timeValue & 0xFFFFFFFF);
    record.scale = (TimeScale)time.timeScale;
    record.base = NULL;
    return record;
}

BOOL QTTimeIsIndefinite(QTTime time)
{
    return (time.flags & kQTTimeIsIndefinite) != 0;
}

NSComparisonResult QTTimeCompare(QTTime time, QTTime otherTime)
{
    // The indefinite time compares greater than every other time, and equal to itself.
    BOOL indefinite = QTTimeIsIndefinite(time);
    BOOL otherIndefinite = QTTimeIsIndefinite(otherTime);
    if (indefinite || otherIndefinite) {
        if (indefinite && otherIndefinite)
            return NSOrderedSame;
        return indefinite ? NSOrderedDescending : NSOrderedAscending;
    }
    if (time.timeScale <= 0 || otherTime.timeScale <= 0) {
        if (time.timeScale <= 0 && otherTime.timeScale <= 0)
            return NSOrderedSame;
        return time.timeScale <= 0 ? NSOrderedAscending : NSOrderedDescending;
    }
    long long left, right;
    if (__builtin_mul_overflow(time.timeValue, otherTime.timeScale, &left) ||
        __builtin_mul_overflow(otherTime.timeValue, time.timeScale, &right)) {
        long double wideLeft = (long double)time.timeValue * otherTime.timeScale;
        long double wideRight = (long double)otherTime.timeValue * time.timeScale;
        if (wideLeft < wideRight)
            return NSOrderedAscending;
        return wideLeft > wideRight ? NSOrderedDescending : NSOrderedSame;
    }
    if (left < right)
        return NSOrderedAscending;
    return left > right ? NSOrderedDescending : NSOrderedSame;
}

static int64_t QTGreatestCommonDivisor(int64_t a, int64_t b)
{
    while (b != 0) {
        int64_t rest = a % b;
        a = b;
        b = rest;
    }
    return a;
}

// time + sign * other, at the least common time scale of the two, which is the scale of the
// result as long as it is the same for both.
static QTTime QTTimeCombine(QTTime time, QTTime other, int sign)
{
    if (QTTimeIsInvalid(time) || QTTimeIsInvalid(other))
        return QTInvalidTime();
    if (time.timeScale == other.timeScale)
        return QTMakeTime(time.timeValue + sign * other.timeValue, time.timeScale);

    int64_t divisor = QTGreatestCommonDivisor(time.timeScale, other.timeScale);
    long long common;
    if (!__builtin_mul_overflow((long long)(time.timeScale / divisor), (long long)other.timeScale, &common) && common <= LONG_MAX) {
        long scale = (long)common;
        long long first, second, total;
        if (__builtin_mul_overflow(time.timeValue, (long long)(scale / time.timeScale), &first) ||
            __builtin_mul_overflow(other.timeValue, (long long)(scale / other.timeScale), &second) ||
            (sign > 0 ? __builtin_add_overflow(first, second, &total) : __builtin_sub_overflow(first, second, &total)))
            return QTInvalidTime();
        return QTMakeTime(total, scale);
    }

    // QUIRK: when the least common scale does not fit in a long, the result has a scale of
    // 1000000000 and each operand is rounded to it. Observed on i386 (32-bit long) as 1/1000003 +
    // 1/999983 = 2000/1000000000. Test: qtkit-time "increment with scales that overflow" (i386 only).
    double scale = (double)QT_TIME_SCALE_FALLBACK;
    double first = round((double)time.timeValue * scale / (double)time.timeScale);
    double second = round((double)other.timeValue * scale / (double)other.timeScale);
    return QTMakeTime((long long)(first + sign * second), QT_TIME_SCALE_FALLBACK);
}

QTTime QTTimeIncrement(QTTime time, QTTime increment)
{
    return QTTimeCombine(time, increment, 1);
}

QTTime QTTimeDecrement(QTTime time, QTTime decrement)
{
    return QTTimeCombine(time, decrement, -1);
}

NSString *QTStringFromTime(QTTime time)
{
    // QUIRK: the format is days:hours:minutes:seconds.fraction/scale, with the fraction as a
    // whole number of time scale units written with at least two digits, and a minus sign before a
    // negative time. The indefinite time has no string (nil). Test: qtkit-time "time strings".
    if (QTTimeIsInvalid(time))
        return nil;
    BOOL negative = time.timeValue < 0;
    unsigned long long magnitude = negative ? 0ULL - (unsigned long long)time.timeValue : (unsigned long long)time.timeValue;
    unsigned long long seconds = magnitude / (unsigned long long)time.timeScale;
    unsigned long long fraction = magnitude % (unsigned long long)time.timeScale;
    return [NSString stringWithFormat:@"%@%llu:%02llu:%02llu:%02llu.%02llu/%ld",
        negative ? @"-" : @"",
        seconds / 86400, seconds / 3600 % 24, seconds / 60 % 60, seconds % 60, fraction, time.timeScale];
}

QTTime QTTimeFromString(NSString *string)
{
    // QUIRK: only the format of QTStringFromTime is understood: anything else gives the zero time,
    // and a time scale of 0 gives the indefinite time. Test: qtkit-time "time from strings".
    if (!string)
        return QTZeroTime;
    const char *text = [string UTF8String];
    if (!text)
        return QTZeroTime;

    BOOL negative = *text == '-';
    if (negative)
        text++;
    unsigned long long days, hours, minutes, seconds, fraction;
    long scale;
    int consumed = 0;
    if (sscanf(text, "%llu:%llu:%llu:%llu.%llu/%ld%n", &days, &hours, &minutes, &seconds, &fraction, &scale, &consumed) != 6 ||
        text[consumed] != '\0')
        return QTZeroTime;
    if (scale == 0)
        return QTIndefiniteTime;
    if (scale < 0)
        return QTZeroTime;

    unsigned long long wholeSeconds = days * 86400 + hours * 3600 + minutes * 60 + seconds;
    unsigned long long total;
    if (__builtin_mul_overflow(wholeSeconds, (unsigned long long)scale, &total) ||
        __builtin_add_overflow(total, fraction, &total) || total > (unsigned long long)LLONG_MAX)
        return QTZeroTime;
    return QTMakeTime(negative ? -(long long)total : (long long)total, scale);
}

QTTimeRange QTMakeTimeRange(QTTime time, QTTime duration)
{
    QTTimeRange range;
    range.time = time;
    range.duration = duration;
    return range;
}

QTTime QTTimeRangeEnd(QTTimeRange range)
{
    return QTTimeIncrement(range.time, range.duration);
}

static QTTimeRange QTIndefiniteTimeRange(void)
{
    return QTMakeTimeRange(QTIndefiniteTime, QTIndefiniteTime);
}

BOOL QTTimeInTimeRange(QTTime time, QTTimeRange range)
{
    // The range includes its start and not its end, so a range of no duration has no times in it.
    QTTime end = QTTimeRangeEnd(range);
    if (QTTimeIsInvalid(time) || QTTimeIsInvalid(range.time) || QTTimeIsInvalid(end))
        return NO;
    return QTTimeCompare(time, range.time) != NSOrderedAscending && QTTimeCompare(time, end) == NSOrderedAscending;
}

BOOL QTEqualTimeRanges(QTTimeRange range1, QTTimeRange range2)
{
    return QTTimeCompare(range1.time, range2.time) == NSOrderedSame &&
           QTTimeCompare(range1.duration, range2.duration) == NSOrderedSame;
}

QTTimeRange QTIntersectionTimeRange(QTTimeRange range1, QTTimeRange range2)
{
    QTTime end1 = QTTimeRangeEnd(range1);
    QTTime end2 = QTTimeRangeEnd(range2);
    if (QTTimeIsInvalid(range1.time) || QTTimeIsInvalid(range2.time) || QTTimeIsInvalid(end1) || QTTimeIsInvalid(end2))
        return QTIndefiniteTimeRange();

    QTTime start = QTTimeCompare(range1.time, range2.time) == NSOrderedDescending ? range1.time : range2.time;
    QTTime end = QTTimeCompare(end1, end2) == NSOrderedAscending ? end1 : end2;
    // QUIRK: ranges that do not overlap give an indefinite start and no duration. Ranges that only
    // touch give a range of no duration at the touching point. Test: qtkit-time "range intersection".
    if (QTTimeCompare(start, end) == NSOrderedDescending)
        return QTMakeTimeRange(QTIndefiniteTime, QTZeroTime);
    return QTMakeTimeRange(start, QTTimeDecrement(end, start));
}

QTTimeRange QTUnionTimeRange(QTTimeRange range1, QTTimeRange range2)
{
    // The smallest range that holds both, including a range of no duration.
    QTTime end1 = QTTimeRangeEnd(range1);
    QTTime end2 = QTTimeRangeEnd(range2);
    if (QTTimeIsInvalid(range1.time) || QTTimeIsInvalid(range2.time) || QTTimeIsInvalid(end1) || QTTimeIsInvalid(end2))
        return QTIndefiniteTimeRange();

    QTTime start = QTTimeCompare(range1.time, range2.time) == NSOrderedAscending ? range1.time : range2.time;
    QTTime end = QTTimeCompare(end1, end2) == NSOrderedDescending ? end1 : end2;
    return QTMakeTimeRange(start, QTTimeDecrement(end, start));
}

NSString *QTStringFromTimeRange(QTTimeRange range)
{
    // QUIRK: the format is the strings of the start and the duration joined by "~"; an indefinite
    // time shows as "(null)". Test: qtkit-time "time range strings".
    return [NSString stringWithFormat:@"%@~%@", QTStringFromTime(range.time), QTStringFromTime(range.duration)];
}

QTTimeRange QTTimeRangeFromString(NSString *string)
{
    if (!string)
        return QTMakeTimeRange(QTZeroTime, QTZeroTime);
    NSRange separator = [string rangeOfString:@"~"];
    if (separator.location != NSNotFound) {
        return QTMakeTimeRange(QTTimeFromString([string substringToIndex:separator.location]),
                               QTTimeFromString([string substringFromIndex:NSMaxRange(separator)]));
    }
    // Without a duration, the range is empty at the time the string gives.
    QTTime time = QTTimeFromString(string);
    return QTMakeTimeRange(time, QTMakeTime(0, time.timeScale));
}
