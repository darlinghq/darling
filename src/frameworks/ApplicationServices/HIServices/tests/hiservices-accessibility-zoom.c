/*
 * HIServices accessibility names and UniversalAccess zoom (Apple's
 * HIServices headers): the error codes keep their public values, the
 * parameterized text attributes have the same names as AppKit's
 * NSAccessibility ones, and with no screen zoom UAZoomEnabled is false.
 */
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { \
	if (cond) printf("PASS: %s\n", #cond); else { printf("FAIL: %s\n", #cond); failures++; } \
} while (0)

int main(void)
{
	CHECK(kAXErrorSuccess == 0 && kAXErrorFailure == -25200 && kAXErrorNoValue == -25212);
	CHECK(kAXErrorParameterizedAttributeUnsupported == -25213 && kAXErrorNotEnoughPrecision == -25214);
	CHECK(CFEqual(kAXLineForIndexParameterizedAttribute, CFSTR("AXLineForIndex")));
	CHECK(CFEqual(kAXStringForRangeParameterizedAttribute, CFSTR("AXStringForRange")));
	CHECK(CFEqual(kAXMenuOpenedNotification, CFSTR("AXMenuOpened")));

	CHECK(!UAZoomEnabled());
	CGRect focus = CGRectMake(10, 20, 30, 40);
	CHECK(UAZoomChangeFocus(&focus, NULL, kUAZoomFocusTypeInsertionPoint) == noErr);
	CHECK(UAZoomChangeFocus(NULL, NULL, kUAZoomFocusTypeOther) == paramErr);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
