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
#import <math.h>
#import <limits.h>

const QTTimeRange QTZeroTimeRange = { { 0, 1, 0 }, { 0, 1, 0 } };

static int64_t gcd64(int64_t a, int64_t b)
{
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

QTTime QTMakeTime(long long timeValue, long timeScale)
{
    QTTime t;
    t.timeValue = timeValue;
    t.timeScale = timeScale;
    t.flags = 0;
    return t;
}

QTTime QTMakeTimeScaled(QTTime time, long targetScale)
{
    if (time.flags & kQTTimeIsIndefinite) {
        QTTime t;
        t.timeValue = 0;
        t.timeScale = targetScale;
        t.flags = kQTTimeIsIndefinite;
        return t;
    }
    if (targetScale <= 0 || time.timeScale <= 0) {
        return QTMakeTime(0, targetScale);
    }
    if (time.timeScale == targetScale) {
        return time;
    }
    __int128 val = (__int128)time.timeValue * targetScale;
    val /= time.timeScale;
    QTTime t;
    t.timeValue = (long long)val;
    t.timeScale = targetScale;
    t.flags = time.flags;
    return t;
}

QTTime QTMakeTimeWithTimeInterval(NSTimeInterval timeInterval)
{
    if (isnan(timeInterval)) {
        return QTIndefiniteTime;
    }
    long scale = 600;
    long long val = (long long)round(timeInterval * scale);
    return QTMakeTime(val, scale);
}

QTTime QTMakeTimeWithTimeRecord(TimeRecord timeRecord)
{
    int64_t val = ((int64_t)timeRecord.value.hi << 32) | (uint32_t)timeRecord.value.lo;
    return QTMakeTime(val, timeRecord.scale);
}

BOOL QTGetTimeInterval(QTTime time, NSTimeInterval *timeIntervalPtr)
{
    if ((time.flags & kQTTimeIsIndefinite) || time.timeScale <= 0) {
        if (timeIntervalPtr)
            *timeIntervalPtr = 0.0;
        return NO;
    }
    if (timeIntervalPtr) {
        *timeIntervalPtr = (NSTimeInterval)time.timeValue / (NSTimeInterval)time.timeScale;
    }
    return YES;
}

TimeRecord QTGetTimeRecord(QTTime time)
{
    TimeRecord tr;
    tr.value.hi = (SInt32)(time.timeValue >> 32);
    tr.value.lo = (UInt32)(time.timeValue & 0xFFFFFFFF);
    tr.scale = (TimeScale)time.timeScale;
    tr.base = NULL;
    return tr;
}

BOOL QTTimeIsIndefinite(QTTime time)
{
    return (time.flags & kQTTimeIsIndefinite) != 0;
}

NSComparisonResult QTTimeCompare(QTTime time1, QTTime time2)
{
    BOOL ind1 = (time1.flags & kQTTimeIsIndefinite) != 0;
    BOOL ind2 = (time2.flags & kQTTimeIsIndefinite) != 0;
    if (ind1 && ind2) return NSOrderedSame;
    if (ind1) return NSOrderedDescending;
    if (ind2) return NSOrderedAscending;

    if (time1.timeScale <= 0 && time2.timeScale <= 0) return NSOrderedSame;
    if (time1.timeScale <= 0) return NSOrderedAscending;
    if (time2.timeScale <= 0) return NSOrderedDescending;

    __int128 p1 = (__int128)time1.timeValue * time2.timeScale;
    __int128 p2 = (__int128)time2.timeValue * time1.timeScale;
    if (p1 < p2) return NSOrderedAscending;
    if (p1 > p2) return NSOrderedDescending;
    return NSOrderedSame;
}

QTTime QTTimeIncrement(QTTime time, QTTime increment)
{
    if ((time.flags & kQTTimeIsIndefinite) || (increment.flags & kQTTimeIsIndefinite)) {
        return QTIndefiniteTime;
    }
    if (time.timeScale <= 0) return increment;
    if (increment.timeScale <= 0) return time;
    if (time.timeScale == increment.timeScale) {
        return QTMakeTime(time.timeValue + increment.timeValue, time.timeScale);
    }
    int64_t g = gcd64(time.timeScale, increment.timeScale);
    __int128 commonScale128 = ((__int128)time.timeScale * increment.timeScale) / g;
    if (commonScale128 > 0 && commonScale128 <= LONG_MAX) {
        long commonScale = (long)commonScale128;
        __int128 v1 = (__int128)time.timeValue * (commonScale / time.timeScale);
        __int128 v2 = (__int128)increment.timeValue * (commonScale / increment.timeScale);
        __int128 total = v1 + v2;
        return QTMakeTime((long long)total, commonScale);
    } else {
        QTTime scaledInc = QTMakeTimeScaled(increment, time.timeScale);
        return QTMakeTime(time.timeValue + scaledInc.timeValue, time.timeScale);
    }
}

QTTime QTTimeDecrement(QTTime time, QTTime decrement)
{
    if ((time.flags & kQTTimeIsIndefinite) || (decrement.flags & kQTTimeIsIndefinite)) {
        return QTIndefiniteTime;
    }
    if (time.timeScale <= 0) {
        return QTMakeTime(-decrement.timeValue, decrement.timeScale);
    }
    if (decrement.timeScale <= 0) return time;
    if (time.timeScale == decrement.timeScale) {
        return QTMakeTime(time.timeValue - decrement.timeValue, time.timeScale);
    }
    int64_t g = gcd64(time.timeScale, decrement.timeScale);
    __int128 commonScale128 = ((__int128)time.timeScale * decrement.timeScale) / g;
    if (commonScale128 > 0 && commonScale128 <= LONG_MAX) {
        long commonScale = (long)commonScale128;
        __int128 v1 = (__int128)time.timeValue * (commonScale / time.timeScale);
        __int128 v2 = (__int128)decrement.timeValue * (commonScale / decrement.timeScale);
        __int128 total = v1 - v2;
        return QTMakeTime((long long)total, commonScale);
    } else {
        QTTime scaledDec = QTMakeTimeScaled(decrement, time.timeScale);
        return QTMakeTime(time.timeValue - scaledDec.timeValue, time.timeScale);
    }
}

NSString *QTStringFromTime(QTTime time)
{
    if (time.flags & kQTTimeIsIndefinite) {
        return @"indefinite";
    }
    return [NSString stringWithFormat:@"%lld/%ld", time.timeValue, time.timeScale];
}

QTTime QTTimeFromString(NSString *string)
{
    if (!string || [string length] == 0) return QTZeroTime;
    if ([string caseInsensitiveCompare:@"indefinite"] == NSOrderedSame) return QTIndefiniteTime;

    long long val = 0;
    long scale = 1;
    if (sscanf([string UTF8String], "%lld/%ld", &val, &scale) == 2) {
        return QTMakeTime(val, scale > 0 ? scale : 1);
    }

    NSArray *parts = [string componentsSeparatedByString:@":"];
    if ([parts count] >= 2) {
        double totalSeconds = 0;
        if ([parts count] == 2) {
            totalSeconds = [[parts objectAtIndex:0] doubleValue] * 60.0 + [[parts objectAtIndex:1] doubleValue];
        } else if ([parts count] == 3) {
            totalSeconds = [[parts objectAtIndex:0] doubleValue] * 3600.0 + [[parts objectAtIndex:1] doubleValue] * 60.0 + [[parts objectAtIndex:2] doubleValue];
        } else if ([parts count] == 4) {
            totalSeconds = [[parts objectAtIndex:0] doubleValue] * 86400.0 + [[parts objectAtIndex:1] doubleValue] * 3600.0 + [[parts objectAtIndex:2] doubleValue] * 60.0 + [[parts objectAtIndex:3] doubleValue];
        }
        return QTMakeTimeWithTimeInterval(totalSeconds);
    }

    double d = [string doubleValue];
    return QTMakeTimeWithTimeInterval(d);
}

QTTimeRange QTMakeTimeRange(QTTime time, QTTime duration)
{
    QTTimeRange r;
    r.time = time;
    r.duration = duration;
    return r;
}

QTTime QTTimeRangeEnd(QTTimeRange range)
{
    return QTTimeIncrement(range.time, range.duration);
}

BOOL QTTimeInTimeRange(QTTime time, QTTimeRange range)
{
    if ((time.flags & kQTTimeIsIndefinite) || (range.time.flags & kQTTimeIsIndefinite) || (range.duration.flags & kQTTimeIsIndefinite)) {
        return NO;
    }
    if (QTTimeCompare(time, range.time) == NSOrderedAscending) {
        return NO;
    }
    QTTime end = QTTimeRangeEnd(range);
    if (range.duration.timeValue == 0) {
        return QTTimeCompare(time, range.time) == NSOrderedSame;
    }
    return QTTimeCompare(time, end) == NSOrderedAscending;
}

BOOL QTEqualTimeRanges(QTTimeRange range1, QTTimeRange range2)
{
    return (QTTimeCompare(range1.time, range2.time) == NSOrderedSame) &&
           (QTTimeCompare(range1.duration, range2.duration) == NSOrderedSame);
}

QTTimeRange QTIntersectionTimeRange(QTTimeRange range1, QTTimeRange range2)
{
    if ((range1.time.flags & kQTTimeIsIndefinite) || (range1.duration.flags & kQTTimeIsIndefinite) ||
        (range2.time.flags & kQTTimeIsIndefinite) || (range2.duration.flags & kQTTimeIsIndefinite)) {
        return QTZeroTimeRange;
    }
    QTTime start = (QTTimeCompare(range1.time, range2.time) == NSOrderedDescending) ? range1.time : range2.time;
    QTTime end1 = QTTimeRangeEnd(range1);
    QTTime end2 = QTTimeRangeEnd(range2);
    QTTime end = (QTTimeCompare(end1, end2) == NSOrderedAscending) ? end1 : end2;
    if (QTTimeCompare(start, end) == NSOrderedDescending) {
        return QTZeroTimeRange;
    }
    QTTime duration = QTTimeDecrement(end, start);
    return QTMakeTimeRange(start, duration);
}

QTTimeRange QTUnionTimeRange(QTTimeRange range1, QTTimeRange range2)
{
    if (range1.duration.timeValue == 0) return range2;
    if (range2.duration.timeValue == 0) return range1;
    QTTime start = (QTTimeCompare(range1.time, range2.time) == NSOrderedAscending) ? range1.time : range2.time;
    QTTime end1 = QTTimeRangeEnd(range1);
    QTTime end2 = QTTimeRangeEnd(range2);
    QTTime end = (QTTimeCompare(end1, end2) == NSOrderedDescending) ? end1 : end2;
    QTTime duration = QTTimeDecrement(end, start);
    return QTMakeTimeRange(start, duration);
}

NSString *QTStringFromTimeRange(QTTimeRange range)
{
    return [NSString stringWithFormat:@"{%@, %@}", QTStringFromTime(range.time), QTStringFromTime(range.duration)];
}

QTTimeRange QTTimeRangeFromString(NSString *string)
{
    if (!string || [string length] == 0) return QTZeroTimeRange;
    NSString *trimmed = [string stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"{} \t\r\n"]];
    NSArray *parts = [trimmed componentsSeparatedByString:@","];
    if ([parts count] >= 2) {
        QTTime t = QTTimeFromString([[parts objectAtIndex:0] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]);
        QTTime d = QTTimeFromString([[parts objectAtIndex:1] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]);
        return QTMakeTimeRange(t, d);
    }
    return QTZeroTimeRange;
}

QTTime QTTimeFromCMTime(CMTime time)
{
    if ((time.flags & kCMTimeFlags_Indefinite) || !(time.flags & kCMTimeFlags_Valid)) {
        return QTIndefiniteTime;
    }
    return QTMakeTime(time.value, (long)time.timescale);
}

CMTime CMTimeFromQTTime(QTTime time)
{
    if ((time.flags & kQTTimeIsIndefinite) || time.timeScale <= 0) {
        return kCMTimeIndefinite;
    }
    return CMTimeMake(time.timeValue, (int32_t)time.timeScale);
}

QTTimeRange QTTimeRangeFromCMTimeRange(CMTimeRange range)
{
    return QTMakeTimeRange(QTTimeFromCMTime(range.start), QTTimeFromCMTime(range.duration));
}

CMTimeRange CMTimeRangeFromQTTimeRange(QTTimeRange range)
{
    return CMTimeRangeMake(CMTimeFromQTTime(range.time), CMTimeFromQTTime(range.duration));
}
