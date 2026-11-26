/*
 * The HIServices text marker SPI that WebKit uses (declared by its
 * HIServicesSPI.h): a marker keeps a copy of its bytes, markers with the same
 * bytes are equal, and a range keeps the two markers it was made from.
 */
#include <HIServices/AXTextMarker.h>
#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(cond) do { \
	if (cond) printf("PASS: %s\n", #cond); else { printf("FAIL: %s\n", #cond); failures++; } \
} while (0)

int main(void)
{
	UInt8 bytes[] = { 1, 2, 3, 4, 5 };
	AXTextMarkerRef a = AXTextMarkerCreate(NULL, bytes, sizeof bytes);
	bytes[0] = 9;
	AXTextMarkerRef b = AXTextMarkerCreate(NULL, bytes, sizeof bytes);
	bytes[0] = 1;
	AXTextMarkerRef c = AXTextMarkerCreate(NULL, bytes, sizeof bytes);

	CHECK(a != NULL && CFGetTypeID(a) == AXTextMarkerGetTypeID());
	CHECK(AXTextMarkerGetLength(a) == 5);
	CHECK(AXTextMarkerGetBytePtr(a)[0] == 1 && AXTextMarkerGetBytePtr(a)[4] == 5);
	CHECK(CFEqual(a, c) && CFHash(a) == CFHash(c));
	CHECK(!CFEqual(a, b));
	CHECK(AXTextMarkerCreate(NULL, NULL, 3) == NULL);

	AXTextMarkerRangeRef range = AXTextMarkerRangeCreate(NULL, a, b);
	CHECK(range != NULL && CFGetTypeID(range) == AXTextMarkerRangeGetTypeID());
	AXTextMarkerRef start = AXTextMarkerRangeCopyStartMarker(range);
	AXTextMarkerRef end = AXTextMarkerRangeCopyEndMarker(range);
	CHECK(start == a && end == b);
	AXTextMarkerRangeRef same = AXTextMarkerRangeCreate(NULL, c, b);
	CHECK(CFEqual(range, same));
	CHECK(AXTextMarkerRangeCreate(NULL, a, NULL) == NULL);

	CHECK(_AXGetClientForCurrentRequestUntrusted() == kAXClientTypeNoActiveRequestFound);
	_AXSetClientIdentificationOverride(kAXClientTypeVoiceOver);
	CHECK(_AXGetClientForCurrentRequestUntrusted() == kAXClientTypeVoiceOver);
	CHECK(!_AXUIElementRequestServicedBySecondaryAXThread());

	CFRelease(start); CFRelease(end); CFRelease(range); CFRelease(same);
	CFRelease(a); CFRelease(b); CFRelease(c);
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
