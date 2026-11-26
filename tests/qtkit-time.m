// Copyright (C) 2026 Darling Developers
//
// QTTime, QTTimeRange and the QTKit utility functions. The expected values of everything that the
// documentation of QTKit does not spell out come from running these same calls against the real QTKit
// on macOS 10.14.4 (i386); this test passes there unchanged. Works for i386 and x86_64:
//   guest-cc.sh -a i386 -o qtkit-time tests/qtkit-time.m -framework Foundation -framework QTKit
// Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)
#define CHECK_TIME(t, value, scale, flagsValue, what) do { QTTime _t = (t); \
	CHECK(_t.timeValue == (value) && _t.timeScale == (scale) && _t.flags == (flagsValue), \
	      "%s: got %lld/%ld flags %ld", what, _t.timeValue, (long)_t.timeScale, (long)_t.flags); } while (0)
#define CHECK_STRING(actual, expected, what) do { NSString *_a = (actual); NSString *_e = (expected); \
	CHECK((_a == nil && _e == nil) || [_a isEqualToString:_e], "%s: got %s, expected %s", what, [[_a description] UTF8String], [[_e description] UTF8String]); } while (0)

int main(void)
{
	@autoreleasepool {
		QTTime zero = QTZeroTime, indefinite = QTIndefiniteTime;
		CHECK_TIME(zero, 0, 1, 0, "QTZeroTime");
		CHECK_TIME(indefinite, 0, 1, kQTTimeIsIndefinite, "QTIndefiniteTime");
		CHECK(QTTimeIsIndefinite(indefinite) && !QTTimeIsIndefinite(zero), "QTTimeIsIndefinite");

		// make time with a zero scale
		CHECK_TIME(QTMakeTime(1500, 600), 1500, 600, 0, "QTMakeTime");
		CHECK_TIME(QTMakeTime(7, 0), 0, 1, kQTTimeIsIndefinite, "QTMakeTime with a zero scale");

		// make time scaled
		CHECK_TIME(QTMakeTimeScaled(QTMakeTime(1500, 600), 30), 75, 30, 0, "scale 1500/600 to 30");
		CHECK_TIME(QTMakeTimeScaled(QTMakeTime(5, 3), 1), 1, 1, 0, "5/3 to 1 truncates");
		CHECK_TIME(QTMakeTimeScaled(QTMakeTime(-5, 3), 1), -1, 1, 0, "-5/3 to 1 truncates toward zero");
		CHECK_TIME(QTMakeTimeScaled(QTMakeTime(1, 3), 2), 0, 2, 0, "1/3 to 2");
		CHECK_TIME(QTMakeTimeScaled(indefinite, 30), 0, 1, kQTTimeIsIndefinite, "indefinite stays indefinite");
		CHECK_TIME(QTMakeTimeScaled(QTMakeTime(5, 2), 0), 0, 1, kQTTimeIsIndefinite, "a zero target scale is indefinite");

		// time interval
		CHECK_TIME(QTMakeTimeWithTimeInterval(2.5), 2500000, 1000000, 0, "interval 2.5");
		CHECK_TIME(QTMakeTimeWithTimeInterval(-1.5), -1500000, 1000000, 0, "interval -1.5");
		CHECK_TIME(QTMakeTimeWithTimeInterval(1.0 / 3), 333333, 1000000, 0, "interval 1/3 truncates");
		CHECK_TIME(QTMakeTimeWithTimeInterval(0.0000006), 0, 1000000, 0, "interval 6e-7 truncates");
		CHECK_TIME(QTMakeTimeWithTimeInterval(1e300), 0, 1, kQTTimeIsIndefinite, "interval too large");
		CHECK_TIME(QTMakeTimeWithTimeInterval(__builtin_inf()), 0, 1, kQTTimeIsIndefinite, "interval infinite");
		NSTimeInterval interval = -1;
		CHECK(QTGetTimeInterval(QTMakeTime(1500, 600), &interval) && interval == 2.5, "QTGetTimeInterval");
		CHECK(QTGetTimeInterval(QTMakeTime(-5, 2), &interval) && interval == -2.5, "QTGetTimeInterval of a negative time");
		interval = -1;
		CHECK(!QTGetTimeInterval(indefinite, &interval) && interval == 0, "QTGetTimeInterval of an indefinite time");

		// compare
		CHECK(QTTimeCompare(QTMakeTime(1500, 600), QTMakeTime(3, 1)) == NSOrderedAscending, "compare less");
		CHECK(QTTimeCompare(QTMakeTime(3, 1), QTMakeTime(1500, 600)) == NSOrderedDescending, "compare greater");
		CHECK(QTTimeCompare(QTMakeTime(1, 2), QTMakeTime(2, 4)) == NSOrderedSame, "compare equal at different scales");
		CHECK(QTTimeCompare(QTMakeTime(1, 3), QTMakeTime(333, 1000)) == NSOrderedDescending, "compare 1/3 and 333/1000");
		CHECK(QTTimeCompare(QTMakeTime(1, 1), indefinite) == NSOrderedAscending, "indefinite is greater than any time");
		CHECK(QTTimeCompare(indefinite, QTMakeTime(1, 1)) == NSOrderedDescending, "any time is less than indefinite");
		CHECK(QTTimeCompare(indefinite, indefinite) == NSOrderedSame, "indefinite equals indefinite");

		// increment and decrement
		CHECK_TIME(QTTimeIncrement(QTMakeTime(1500, 600), QTMakeTime(3, 1)), 3300, 600, 0, "1500/600 + 3/1");
		CHECK_TIME(QTTimeIncrement(QTMakeTime(1, 4), QTMakeTime(1, 6)), 5, 12, 0, "1/4 + 1/6 at the common scale");
		CHECK_TIME(QTTimeIncrement(QTMakeTime(1, 6), QTMakeTime(1, 6)), 2, 6, 0, "1/6 + 1/6 keeps the scale");
		CHECK_TIME(QTTimeIncrement(QTMakeTime(1, 6), QTMakeTime(1, 3)), 3, 6, 0, "1/6 + 1/3");
		CHECK_TIME(QTTimeIncrement(QTMakeTime(3, 6), QTMakeTime(1, 4)), 9, 12, 0, "3/6 + 1/4");
		CHECK_TIME(QTTimeIncrement(QTMakeTime(1500, 600), indefinite), 0, 1, kQTTimeIsIndefinite, "+ indefinite");
		CHECK_TIME(QTTimeDecrement(QTMakeTime(1500, 600), QTMakeTime(3, 1)), -300, 600, 0, "1500/600 - 3/1");
		CHECK_TIME(QTTimeDecrement(QTMakeTime(1, 2), QTMakeTime(3, 1)), -5, 2, 0, "1/2 - 3/1");
		CHECK_TIME(QTTimeDecrement(QTMakeTime(1, 2), indefinite), 0, 1, kQTTimeIsIndefinite, "- indefinite");
#if !__LP64__
		// increment with scales that overflow: the least common scale does not fit in a 32-bit long.
		CHECK_TIME(QTTimeIncrement(QTMakeTime(1, 1000003), QTMakeTime(1, 999983)), 2000, 1000000000, 0,
		           "increment with scales that overflow");
#endif

		// time strings
		CHECK_STRING(QTStringFromTime(QTMakeTime(1500, 600)), @"0:00:00:02.300/600", "string of 1500/600");
		CHECK_STRING(QTStringFromTime(QTMakeTime(3, 1)), @"0:00:00:03.00/1", "string of 3/1");
		CHECK_STRING(QTStringFromTime(zero), @"0:00:00:00.00/1", "string of zero");
		CHECK_STRING(QTStringFromTime(QTMakeTime(-90, 30)), @"-0:00:00:03.00/30", "string of a negative time");
		CHECK_STRING(QTStringFromTime(QTMakeTime(3600 * 600 + 5, 600)), @"0:01:00:00.05/600", "string with hours");
		CHECK_STRING(QTStringFromTime(QTMakeTime(7, 3)), @"0:00:00:02.01/3", "string of 7/3");
		CHECK_STRING(QTStringFromTime(QTMakeTime(90061 * 600, 600)), @"1:01:01:01.00/600", "string with days");
		CHECK_STRING(QTStringFromTime(indefinite), nil, "string of indefinite");

		// time from strings
		CHECK_TIME(QTTimeFromString(@"0:00:00:02.300/600"), 1500, 600, 0, "parse 0:00:00:02.300/600");
		CHECK_TIME(QTTimeFromString(@"0:00:00:03.00/1"), 3, 1, 0, "parse 0:00:00:03.00/1");
		CHECK_TIME(QTTimeFromString(@"-0:00:00:03.00/30"), -90, 30, 0, "parse a negative time");
		CHECK_TIME(QTTimeFromString(@"1:01:01:01.00/600"), 54036600, 600, 0, "parse with days");
		CHECK_TIME(QTTimeFromString(@"0:00:00:02.5/600"), 1205, 600, 0, "the fraction is a whole number of units");
		CHECK_TIME(QTTimeFromString(@"0:00:00:02.300/0"), 0, 1, kQTTimeIsIndefinite, "a zero scale is indefinite");
		NSString *others[] = { @"2.5", @"1500/600", @"0:00:00:02/600", @"0:00:00:02.300", @"indefinite", @"garbage", @"" };
		for (int i = 0; i < 7; i++)
			CHECK_TIME(QTTimeFromString(others[i]), 0, 1, 0, ([[NSString stringWithFormat:@"other format '%@' gives the zero time", others[i]] UTF8String]));
		QTTime roundTrip = QTTimeFromString(QTStringFromTime(QTMakeTime(123456, 600)));
		CHECK_TIME(roundTrip, 123456, 600, 0, "round trip");

		// ranges
		QTTimeRange r1 = QTMakeTimeRange(QTMakeTime(0, 600), QTMakeTime(600, 600));
		QTTimeRange r2 = QTMakeTimeRange(QTMakeTime(300, 600), QTMakeTime(600, 600));
		QTTimeRange r3 = QTMakeTimeRange(QTMakeTime(1200, 600), QTMakeTime(60, 600));
		CHECK_TIME(r1.time, 0, 600, 0, "range time");
		CHECK_TIME(QTTimeRangeEnd(r1), 600, 600, 0, "range end");
		CHECK_TIME(QTTimeRangeEnd(QTMakeTimeRange(QTMakeTime(1, 1), indefinite)), 0, 1, kQTTimeIsIndefinite, "end of a range of indefinite duration");
		CHECK(QTTimeInTimeRange(QTMakeTime(0, 600), r1) && QTTimeInTimeRange(QTMakeTime(599, 600), r1), "times in a range");
		CHECK(!QTTimeInTimeRange(QTMakeTime(600, 600), r1), "the end is not in the range");
		CHECK(!QTTimeInTimeRange(QTMakeTime(-1, 600), r1), "a time before the start is not in the range");
		CHECK(!QTTimeInTimeRange(QTMakeTime(1, 1), QTMakeTimeRange(QTMakeTime(1, 1), QTMakeTime(0, 1))), "no time is in a range of no duration");
		CHECK(!QTTimeInTimeRange(QTMakeTime(1, 1), QTMakeTimeRange(indefinite, QTMakeTime(1, 1))), "no time is in a range with an indefinite start");
		CHECK(QTEqualTimeRanges(r1, r1) && !QTEqualTimeRanges(r1, r2), "equal ranges");
		CHECK(QTEqualTimeRanges(QTMakeTimeRange(QTMakeTime(1, 2), QTMakeTime(1, 1)), QTMakeTimeRange(QTMakeTime(2, 4), QTMakeTime(2, 2))), "equal ranges at different scales");

		CHECK_STRING(QTStringFromTimeRange(r1), @"0:00:00:00.00/600~0:00:00:01.00/600", "time range strings");
		CHECK_STRING(QTStringFromTimeRange(QTMakeTimeRange(indefinite, zero)), @"(null)~0:00:00:00.00/1", "time range strings with an indefinite time");

		QTTimeRange u = QTUnionTimeRange(r1, r2);
		CHECK_STRING(QTStringFromTimeRange(u), @"0:00:00:00.00/600~0:00:00:01.300/600", "union of overlapping ranges");
		u = QTUnionTimeRange(r1, r3);
		CHECK_STRING(QTStringFromTimeRange(u), @"0:00:00:00.00/600~0:00:00:02.60/600", "union of disjoint ranges spans the gap");
		u = QTUnionTimeRange(QTMakeTimeRange(QTMakeTime(5, 1), QTMakeTime(0, 1)), QTMakeTimeRange(QTMakeTime(1, 1), QTMakeTime(2, 1)));
		CHECK_STRING(QTStringFromTimeRange(u), @"0:00:00:01.00/1~0:00:00:04.00/1", "union with a range of no duration");

		// range intersection
		QTTimeRange x = QTIntersectionTimeRange(r1, r2);
		CHECK_STRING(QTStringFromTimeRange(x), @"0:00:00:00.300/600~0:00:00:00.300/600", "range intersection");
		x = QTIntersectionTimeRange(r1, r3);
		CHECK_STRING(QTStringFromTimeRange(x), @"(null)~0:00:00:00.00/1", "intersection of disjoint ranges");
		x = QTIntersectionTimeRange(QTMakeTimeRange(QTMakeTime(0, 1), QTMakeTime(1, 1)), QTMakeTimeRange(QTMakeTime(1, 1), QTMakeTime(1, 1)));
		CHECK_STRING(QTStringFromTimeRange(x), @"0:00:00:01.00/1~0:00:00:00.00/1", "intersection of touching ranges");
		x = QTIntersectionTimeRange(QTMakeTimeRange(QTMakeTime(0, 1), QTMakeTime(1, 1)), QTMakeTimeRange(indefinite, QTMakeTime(1, 1)));
		CHECK_STRING(QTStringFromTimeRange(x), @"(null)~(null)", "intersection with an indefinite range");

		QTTimeRange parsed = QTTimeRangeFromString(@"0:00:00:02.300/600~0:00:00:01.00/600");
		CHECK_TIME(parsed.time, 1500, 600, 0, "parsed range time");
		CHECK_TIME(parsed.duration, 600, 600, 0, "parsed range duration");

		// NSValue
		NSValue *value = [NSValue valueWithQTTime:QTMakeTime(1500, 600)];
		// The values are made from an unnamed struct: "{?=qll}" on i386.
		typedef struct { long long timeValue; long timeScale; long flags; } UnnamedTime;
		CHECK(strcmp([value objCType], @encode(UnnamedTime)) == 0, "NSValue QTTime type %s", [value objCType]);
		CHECK_TIME([value QTTimeValue], 1500, 600, 0, "NSValue round trip");
		value = [NSValue valueWithQTTimeRange:r1];
		CHECK(QTEqualTimeRanges([value QTTimeRangeValue], r1), "NSValue range round trip");

		// OSType strings
		CHECK(QTOSTypeForString(@"vide") == 'vide', "OSType for vide");
		CHECK(QTOSTypeForString(@"abc") == 0x00616263, "OSType for a three character string is not padded");
		CHECK(QTOSTypeForString(@"abcde") == 'abcd', "OSType for a long string uses the first four characters");
		CHECK(QTOSTypeForString(@"") == 0, "OSType for the empty string");
		CHECK_STRING(QTStringForOSType('vide'), @"vide", "string for vide");
		CHECK_STRING(QTMediaTypeStringForOSType('soun'), @"soun", "media type string for soun");
		CHECK_STRING(QTMediaTypeStringForOSType('zzzz'), @"zzzz", "media type string for an unknown type");
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS: QTKit time functions\n");
	return 0;
}
