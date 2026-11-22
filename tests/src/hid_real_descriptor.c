/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * A HID device that has its own report descriptor, as a game sees it through
 * the IOHIDManager API: the ReportDescriptor property is the device's, and
 * the elements are those of that descriptor (cookies, usages, report IDs,
 * ranges, units, null state, vendor pages, input, output and feature items,
 * array items), with values that follow the reports of the device.
 *
 * The device is a virtual HID device on the host (uhid) that a driver script
 * creates and feeds reports in lockstep: the test prints "SYNC: <tag>" and
 * waits for the file <controldir>/go-<tag>.
 * Usage: tests/hid-gamepad/run-uhid.sh <this test binary> <f310|ids|kbd>
 * The test itself takes <controldir> <kind>.
 * Link with -framework IOKit -framework CoreFoundation.
 *
 * The descriptors:
 *  f310: the report descriptor of a USB game controller that has a hat
 *        switch with a range of 0..7 and the null state 8, a vendor-defined
 *        block of input bits and a vendor-defined output report;
 *  ids:  a game pad with two input reports, an output report and a feature
 *        report, each with a report ID, and a signed axis;
 *  kbd:  a keyboard with modifiers, an array of 6 keys and LEDs (the
 *        descriptor of the QEMU USB keyboard).
 * The elements the tests expect are derived from the HID specification (cookie
 * and element rules that IOHIDManager follows are checked on macOS 10.14 with
 * the keyboard).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static int failures;
static const char *ctl;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
	fflush(stdout);
}

static void spin(double seconds)
{
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false);
}

static void sync_point(const char *tag)
{
	char path[512];
	printf("SYNC: %s\n", tag);
	fflush(stdout);
	snprintf(path, sizeof path, "%s/go-%s", ctl, tag);
	for (int i = 0; i < 1200 && access(path, F_OK) != 0; i++)
		spin(0.05);
}

static const uint8_t f310_descriptor[] = {
	0x05, 0x01, 0x09, 0x04, 0xa1, 0x01, 0xa1, 0x02, 0x75, 0x08, 0x95, 0x04, 0x15, 0x00, 0x26, 0xff, 0x00, 0x35, 0x00, 0x46, 0xff, 0x00,
	0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35, 0x81, 0x02, 0x75, 0x04, 0x95, 0x01, 0x25, 0x07, 0x46, 0x3b, 0x01, 0x65, 0x14,
	0x09, 0x39, 0x81, 0x42, 0x65, 0x00, 0x75, 0x01, 0x95, 0x0c, 0x25, 0x01, 0x45, 0x01, 0x05, 0x09, 0x19, 0x01, 0x29, 0x0c, 0x81, 0x02,
	0x06, 0x00, 0xff, 0x75, 0x01, 0x95, 0x10, 0x25, 0x01, 0x45, 0x01, 0x09, 0x01, 0x81, 0x02, 0xc0,
	0xa1, 0x02, 0x75, 0x08, 0x95, 0x07, 0x46, 0xff, 0x00, 0x26, 0xff, 0x00, 0x09, 0x02, 0x91, 0x02, 0xc0, 0xc0,
};

static const uint8_t ids_descriptor[] = {
	0x05, 0x01, 0x09, 0x05, 0xa1, 0x01,
	0x85, 0x01, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x02, 0x81, 0x02,
	0x05, 0x09, 0x19, 0x01, 0x29, 0x08, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
	0x85, 0x02, 0x05, 0x01, 0x09, 0x32, 0x09, 0x35, 0x15, 0x00, 0x26, 0xff, 0x03, 0x75, 0x10, 0x95, 0x02, 0x81, 0x02,
	0x05, 0x09, 0x19, 0x09, 0x29, 0x10, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
	0x85, 0x03, 0x06, 0x00, 0xff, 0x09, 0x20, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95, 0x04, 0x91, 0x02,
	0x85, 0x04, 0x06, 0x00, 0xff, 0x09, 0x21, 0x75, 0x08, 0x95, 0x02, 0xb1, 0x02,
	0xc0,
};

static const uint8_t kbd_descriptor[] = {
	0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x75, 0x01, 0x95, 0x08, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, 0x25, 0x01, 0x81, 0x02,
	0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02,
	0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06, 0x75, 0x08, 0x15, 0x00, 0x25, 0xff, 0x05, 0x07, 0x19, 0x00, 0x29, 0xff, 0x81, 0x00, 0xc0,
};

static IOHIDDeviceRef dev;
static int added_count, removed_count;
static void added_cb(void *ctx, IOReturn r, void *sender, IOHIDDeviceRef d) { dev = d; added_count++; }
static void removed_cb(void *ctx, IOReturn r, void *sender, IOHIDDeviceRef d) { removed_count++; }

struct seen { uint32_t page, usage; CFIndex value; };
#define MAXSEEN 256
static struct seen cbvals[MAXSEEN];
static int cbcount;
static void value_cb(void *ctx, IOReturn r, void *sender, IOHIDValueRef v)
{
	IOHIDElementRef e = IOHIDValueGetElement(v);
	if (cbcount < MAXSEEN)
	{
		cbvals[cbcount].page = IOHIDElementGetUsagePage(e);
		cbvals[cbcount].usage = IOHIDElementGetUsage(e);
		cbvals[cbcount].value = IOHIDValueGetIntegerValue(v);
		cbcount++;
	}
}

static int wait_cb(uint32_t page, uint32_t usage, CFIndex value)
{
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < cbcount; j++)
			if (cbvals[j].page == page && cbvals[j].usage == usage && cbvals[j].value == value)
				return 1;
		spin(0.05);
	}
	return 0;
}

static uint8_t last_report[64];
static CFIndex last_report_length;
static uint32_t last_report_id;
static int report_count;
static void report_cb(void *ctx, IOReturn r, void *sender, IOHIDReportType type, uint32_t id, uint8_t *report, CFIndex length)
{
	last_report_id = id;
	last_report_length = length;
	memcpy(last_report, report, length < (CFIndex) sizeof last_report ? length : sizeof last_report);
	report_count++;
}

static CFArrayRef elements;

static IOHIDElementRef by_cookie(uint32_t cookie)
{
	for (CFIndex i = 0; i < CFArrayGetCount(elements); i++)
	{
		IOHIDElementRef e = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, i);
		if (IOHIDElementGetCookie(e) == cookie)
			return e;
	}
	return NULL;
}

static CFIndex value_of(uint32_t cookie)
{
	IOHIDValueRef v = NULL;
	IOHIDElementRef e = by_cookie(cookie);
	if (!e || IOHIDDeviceGetValue(dev, e, &v) != kIOReturnSuccess || !v)
		return -99999;
	return IOHIDValueGetIntegerValue(v);
}

static int num_prop(CFStringRef key, long *out)
{
	CFTypeRef v = IOHIDDeviceGetProperty(dev, key);
	long long n;
	if (!v || CFGetTypeID(v) != CFNumberGetTypeID() || !CFNumberGetValue(v, kCFNumberSInt64Type, &n))
		return 0;
	*out = (long) n;
	return 1;
}

static int check_prop(const char *what, CFStringRef key, long expected)
{
	long n;
	int ok = num_prop(key, &n) && n == expected;
	char text[200];
	snprintf(text, sizeof text, "%s (%ld)", what, num_prop(key, &n) ? n : -1L);
	check(text, ok);
	return ok;
}

static void check_descriptor(const uint8_t *expected, size_t length)
{
	CFTypeRef v = IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDReportDescriptorKey));
	check("ReportDescriptor is the device's own descriptor, byte for byte",
		v && CFGetTypeID(v) == CFDataGetTypeID() && CFDataGetLength(v) == (CFIndex) length && memcmp(CFDataGetBytePtr(v), expected, length) == 0);
}

struct expect
{
	uint32_t cookie, parent;
	int type;               /* IOHIDElementType */
	uint32_t page, usage;
	uint32_t rid, size, count;
	long lmin, lmax, pmin, pmax;
	uint32_t unit;
	int coll;               /* IOHIDElementCollectionType, or -1 */
};

static int element_matches(const struct expect *x)
{
	IOHIDElementRef e = by_cookie(x->cookie), parent;
	if (!e)
		return 0;
	parent = IOHIDElementGetParent(e);
	if ((parent ? IOHIDElementGetCookie(parent) : 0) != x->parent || (int) IOHIDElementGetType(e) != x->type ||
		IOHIDElementGetUsagePage(e) != x->page || IOHIDElementGetUsage(e) != x->usage)
		return 0;
	if (x->coll >= 0)
		return IOHIDElementGetCollectionType(e) == (IOHIDElementCollectionType) x->coll;
	return IOHIDElementGetReportID(e) == x->rid && IOHIDElementGetReportSize(e) == x->size && IOHIDElementGetReportCount(e) == x->count &&
		IOHIDElementGetLogicalMin(e) == x->lmin && IOHIDElementGetLogicalMax(e) == x->lmax &&
		IOHIDElementGetPhysicalMin(e) == x->pmin && IOHIDElementGetPhysicalMax(e) == x->pmax && IOHIDElementGetUnit(e) == x->unit;
}

static void check_elements(const char *what, const struct expect *list, int n)
{
	int bad = 0;
	for (int i = 0; i < n; i++)
	{
		if (!element_matches(&list[i]))
		{
			printf("  element cookie %u differs\n", list[i].cookie);
			bad++;
		}
	}
	check(what, bad == 0);
}

static IOHIDManagerRef start(int vid, int pid)
{
	IOHIDManagerRef mgr = IOHIDManagerCreate(NULL, kIOHIDOptionsTypeNone);
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFNumberRef v = CFNumberCreate(NULL, kCFNumberIntType, &vid), p = CFNumberCreate(NULL, kCFNumberIntType, &pid);
	CFDictionarySetValue(d, CFSTR(kIOHIDVendorIDKey), v);
	CFDictionarySetValue(d, CFSTR(kIOHIDProductIDKey), p);
	IOHIDManagerSetDeviceMatching(mgr, d);
	IOHIDManagerRegisterDeviceMatchingCallback(mgr, added_cb, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(mgr, removed_cb, NULL);
	IOHIDManagerScheduleWithRunLoop(mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	check("IOHIDManagerOpen", IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone) == kIOReturnSuccess);
	return mgr;
}

static void wait_added(void)
{
	for (int i = 0; i < 160 && !added_count; i++)
		spin(0.05);
	check("the device appears once", added_count == 1);
}

static void run_f310(void)
{
	static const struct expect list[] = {
		{ 1, 0, kIOHIDElementTypeCollection, 0x01, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, kIOHIDElementCollectionTypeApplication },
		{ 2, 1, kIOHIDElementTypeCollection, 0x01, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, kIOHIDElementCollectionTypeLogical },
		{ 3, 2, kIOHIDElementTypeInput_Misc, 0x01, 0x30, 0, 8, 1, 0, 255, 0, 255, 0, -1 },
		{ 4, 2, kIOHIDElementTypeInput_Misc, 0x01, 0x31, 0, 8, 1, 0, 255, 0, 255, 0, -1 },
		{ 5, 2, kIOHIDElementTypeInput_Misc, 0x01, 0x32, 0, 8, 1, 0, 255, 0, 255, 0, -1 },
		{ 6, 2, kIOHIDElementTypeInput_Misc, 0x01, 0x35, 0, 8, 1, 0, 255, 0, 255, 0, -1 },
		{ 7, 2, kIOHIDElementTypeInput_Misc, 0x01, 0x39, 0, 4, 1, 0, 7, 0, 315, 0x14, -1 },
		{ 8, 2, kIOHIDElementTypeInput_Button, 0x09, 0x01, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 19, 2, kIOHIDElementTypeInput_Button, 0x09, 0x0c, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 20, 2, kIOHIDElementTypeInput_Misc, 0xff00, 0x01, 0, 1, 16, 0, 1, 0, 1, 0, -1 },
		{ 35, 2, kIOHIDElementTypeInput_Misc, 0xff00, 0x01, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 36, 1, kIOHIDElementTypeCollection, 0xff00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, kIOHIDElementCollectionTypeLogical },
		{ 38, 36, kIOHIDElementTypeOutput, 0xff00, 0x02, 0, 8, 7, 0, 255, 0, 255, 0, -1 },
		{ 44, 36, kIOHIDElementTypeOutput, 0xff00, 0x02, 0, 8, 1, 0, 255, 0, 255, 0, -1 },
	};
	long n;

	check_descriptor(f310_descriptor, sizeof f310_descriptor);
	check_prop("PrimaryUsagePage is Generic Desktop", CFSTR(kIOHIDPrimaryUsagePageKey), kHIDPage_GenericDesktop);
	check_prop("PrimaryUsage is Joystick", CFSTR(kIOHIDPrimaryUsageKey), kHIDUsage_GD_Joystick);
	check_prop("MaxInputReportSize is the 8 bytes of the report", CFSTR(kIOHIDMaxInputReportSizeKey), 8);
	check_prop("MaxOutputReportSize is the 7 bytes of the output report", CFSTR(kIOHIDMaxOutputReportSizeKey), 7);
	check_prop("MaxFeatureReportSize is 0", CFSTR(kIOHIDMaxFeatureReportSizeKey), 0);
	check("exactly one DeviceUsagePair", CFArrayGetCount(IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDDeviceUsagePairsKey))) == 1);

	elements = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	check("43 elements (3 collections, 4 axes, hat, 12 buttons, 16 vendor bits, 7 output bytes)", elements && CFArrayGetCount(elements) == 43);
	check_elements("collections, axes, hat (0..7, null state, degrees), buttons, vendor bits and the output bytes", list, sizeof list / sizeof list[0]);
	check("the hat has a null state", IOHIDElementHasNullState(by_cookie(7)));

	IOHIDDeviceRegisterInputValueCallback(dev, value_cb, NULL);
	uint8_t buffer[64];
	IOHIDDeviceRegisterInputReportCallback(dev, buffer, sizeof buffer, report_cb, NULL);
	IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	/* A host node has not seen a report yet: the hat is in its null state, 8, which the device itself reports when idle */
	check("idle hat is the device's null value 8", value_of(7) == 8);

	cbcount = 0;
	sync_point("report");
	/* X=0x80 Y=0x20 Z=0xe0 Rz=0x40, hat 3 and buttons 1 and 3, buttons 5 and 12, vendor bits (not seen by the host) */
	check("hat 3 (callback)", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, 3));
	check("X, Y, Z, Rz are the device's values", value_of(3) == 0x80 && value_of(4) == 0x20 && value_of(5) == 0xe0 && value_of(6) == 0x40);
	check("hat reads 3", value_of(7) == 3);
	check("buttons 1, 3, 5 and 12 are down, the others up", value_of(8) == 1 && value_of(9) == 0 && value_of(10) == 1 && value_of(11) == 0 &&
		value_of(12) == 1 && value_of(13) == 0 && value_of(18) == 0 && value_of(19) == 1);
	check("the input report is the device's layout (8 bytes, no report ID)", report_count > 0 && last_report_length == 8 && last_report_id == 0);
	check("report bytes: axes, then hat in the low nibble and buttons 1 to 4 in the high nibble, then buttons 5 to 12",
		last_report_length == 8 && last_report[0] == 0x80 && last_report[1] == 0x20 && last_report[2] == 0xe0 && last_report[3] == 0x40 &&
		last_report[4] == 0x53 && last_report[5] == 0x81);
	check("vendor-defined bits are not fed by the host (0)", value_of(20) == 0 && value_of(35) == 0);

	for (int d = 0; d < 8; d++)
	{
		char tag[16];
		snprintf(tag, sizeof tag, "hat%d", d);
		cbcount = 0;
		sync_point(tag);
		char text[64];
		snprintf(text, sizeof text, "hat direction %d reads %d", d, d);
		check(text, wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, d) && value_of(7) == d);
	}
	cbcount = 0;
	sync_point("hat-center");
	check("hat released reads the null value 8", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, 8) && value_of(7) == 8);

	/* Output and feature reports of the device itself are not written by iokitd */
	uint8_t out[7] = {0};
	check("SetReport of the vendor output report is refused (unsupported)",
		IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 0, out, sizeof out) == kIOReturnUnsupported);
	CFIndex length = 16;
	uint8_t in[16];
	check("GetReport of a feature report is refused (unsupported)", IOHIDDeviceGetReport(dev, kIOHIDReportTypeFeature, 0, in, &length) == kIOReturnUnsupported);
	(void) n;
}

static void run_ids(void)
{
	static const struct expect list[] = {
		{ 1, 0, kIOHIDElementTypeCollection, 0x01, 0x05, 0, 0, 0, 0, 0, 0, 0, 0, kIOHIDElementCollectionTypeApplication },
		{ 2, 1, kIOHIDElementTypeInput_Misc, 0x01, 0x30, 1, 8, 1, -127, 127, -127, 127, 0, -1 },
		{ 3, 1, kIOHIDElementTypeInput_Misc, 0x01, 0x31, 1, 8, 1, -127, 127, -127, 127, 0, -1 },
		{ 4, 1, kIOHIDElementTypeInput_Button, 0x09, 0x01, 1, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 11, 1, kIOHIDElementTypeInput_Button, 0x09, 0x08, 1, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 12, 1, kIOHIDElementTypeInput_Misc, 0x01, 0x32, 2, 16, 1, 0, 1023, 0, 1023, 0, -1 },
		{ 13, 1, kIOHIDElementTypeInput_Misc, 0x01, 0x35, 2, 16, 1, 0, 1023, 0, 1023, 0, -1 },
		{ 14, 1, kIOHIDElementTypeInput_Button, 0x09, 0x09, 2, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 21, 1, kIOHIDElementTypeInput_Button, 0x09, 0x10, 2, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 24, 1, kIOHIDElementTypeOutput, 0xff00, 0x20, 3, 8, 4, 0, 255, 0, 255, 0, -1 },
		{ 28, 1, kIOHIDElementTypeFeature, 0xff00, 0x21, 4, 8, 2, 0, 255, 0, 255, 0, -1 },
	};

	check_descriptor(ids_descriptor, sizeof ids_descriptor);
	check_prop("MaxInputReportSize is the longest report with its report ID (6 bytes)", CFSTR(kIOHIDMaxInputReportSizeKey), 6);
	check_prop("MaxOutputReportSize counts the report ID (5 bytes)", CFSTR(kIOHIDMaxOutputReportSizeKey), 5);
	check_prop("MaxFeatureReportSize counts the report ID (3 bytes)", CFSTR(kIOHIDMaxFeatureReportSizeKey), 3);
	elements = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	check("elements: 1 collection, 2 axes, 16 buttons, 2 more axes, 4 output and 2 feature fields", elements && CFArrayGetCount(elements) == 27);
	check_elements("elements of the reports with their report IDs and ranges (signed axis, 16-bit axes)", list, sizeof list / sizeof list[0]);

	IOHIDDeviceRegisterInputValueCallback(dev, value_cb, NULL);
	uint8_t buffer[64];
	IOHIDDeviceRegisterInputReportCallback(dev, buffer, sizeof buffer, report_cb, NULL);
	IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	cbcount = 0;
	sync_point("report1");
	/* report 1: X = -16, Y = 32, buttons 1 and 3 */
	check("report 1: X is -16 (signed)", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, -16));
	check("report 1: Y is 32, buttons 1 and 3 down", value_of(3) == 32 && value_of(4) == 1 && value_of(5) == 0 && value_of(6) == 1);
	check("report 1 arrives with its ID first", report_count > 0 && last_report_id == 1 && last_report_length == 4 && last_report[0] == 1 &&
		last_report[1] == 0xf0 && last_report[2] == 0x20 && last_report[3] == 0x05);
	sync_point("report2");
	/* report 2: Z = 0x2ff, Rz = 0x100, buttons 9 and 16 */
	check("report 2: Z is 767", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Z, 0x2ff));
	check("report 2: Rz is 256, buttons 9 and 16 down", value_of(13) == 0x100 && value_of(14) == 1 && value_of(15) == 0 && value_of(21) == 1);
	check("report 1 is unchanged by report 2", value_of(2) == -16 && value_of(3) == 32);
	check("report 2 arrives with its ID first", last_report_id == 2 && last_report_length == 6 && last_report[0] == 2 &&
		last_report[1] == 0xff && last_report[2] == 0x02 && last_report[3] == 0x00 && last_report[4] == 0x01 && last_report[5] == 0x81);

	CFIndex length = 16;
	uint8_t in[16];
	check("GetReport(input, 2) returns the current report 2 with its ID", IOHIDDeviceGetReport(dev, kIOHIDReportTypeInput, 2, in, &length) == kIOReturnSuccess &&
		length == 6 && in[0] == 2 && in[5] == 0x81);
	length = 16;
	check("GetReport(input, 1) returns the current report 1 with its ID", IOHIDDeviceGetReport(dev, kIOHIDReportTypeInput, 1, in, &length) == kIOReturnSuccess &&
		length == 4 && in[0] == 1 && in[1] == 0xf0);
	uint8_t out[5] = { 3, 1, 2, 3, 4 };
	check("SetReport of the vendor output report is refused (unsupported)", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 3, out, sizeof out) == kIOReturnUnsupported);
	length = 16;
	check("GetReport of the feature report is refused (unsupported)", IOHIDDeviceGetReport(dev, kIOHIDReportTypeFeature, 4, in, &length) == kIOReturnUnsupported);
}

static int led_set(uint32_t cookie, CFIndex value)
{
	IOHIDValueRef v = IOHIDValueCreateWithIntegerValue(NULL, by_cookie(cookie), 0, value);
	IOReturn r = IOHIDDeviceSetValue(dev, by_cookie(cookie), v);
	CFRelease(v);
	return r == kIOReturnSuccess;
}

static void run_kbd(void)
{
	/* Cookies: 1 collection, 2 .. 9 modifiers, 10 .. 15 the six slots of the key array, 16 the report handler (not
	 * listed), 17 .. 272 the usages 0 .. 255 of the array, 273 .. 277 the LEDs (the outputs come after the inputs). */
	static const struct expect list[] = {
		{ 1, 0, kIOHIDElementTypeCollection, 0x01, 0x06, 0, 0, 0, 0, 0, 0, 0, 0, kIOHIDElementCollectionTypeApplication },
		{ 2, 1, kIOHIDElementTypeInput_Button, 0x07, 0xe0, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 9, 1, kIOHIDElementTypeInput_Button, 0x07, 0xe7, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 10, 1, kIOHIDElementTypeInput_Button, 0x07, 0xffffffff, 0, 8, 6, 0, -1, 0, -1, 0, -1 },
		{ 11, 1, kIOHIDElementTypeInput_Button, 0x07, 0xffffffff, 0, 8, 1, 0, -1, 0, -1, 0, -1 },
		{ 15, 1, kIOHIDElementTypeInput_Button, 0x07, 0xffffffff, 0, 8, 1, 0, -1, 0, -1, 0, -1 },
		{ 17, 1, kIOHIDElementTypeInput_Button, 0x07, 0x00, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 21, 1, kIOHIDElementTypeInput_Button, 0x07, 0x04, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 272, 1, kIOHIDElementTypeInput_Button, 0x07, 0xff, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 273, 1, kIOHIDElementTypeOutput, 0x08, 0x01, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
		{ 277, 1, kIOHIDElementTypeOutput, 0x08, 0x05, 0, 1, 1, 0, 1, 0, 1, 0, -1 },
	};

	check_descriptor(kbd_descriptor, sizeof kbd_descriptor);
	check_prop("PrimaryUsage is Keyboard", CFSTR(kIOHIDPrimaryUsageKey), kHIDUsage_GD_Keyboard);
	check_prop("MaxInputReportSize is 8", CFSTR(kIOHIDMaxInputReportSizeKey), 8);
	check_prop("MaxOutputReportSize is 1", CFSTR(kIOHIDMaxOutputReportSizeKey), 1);
	elements = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	check("elements: collection, 8 modifiers, 6 array slots, 256 usages, 5 LEDs", elements && CFArrayGetCount(elements) >= 276 && CFArrayGetCount(elements) <= 277);
	check_elements("modifiers, array slots (report count 6 in the first), array usages and LEDs (outputs after the inputs)", list, sizeof list / sizeof list[0]);
	check("array elements are marked as arrays and the modifiers are not", IOHIDElementIsArray(by_cookie(10)) && IOHIDElementIsArray(by_cookie(21)) && !IOHIDElementIsArray(by_cookie(2)));

	IOHIDDeviceRegisterInputValueCallback(dev, value_cb, NULL);
	uint8_t buffer[64];
	IOHIDDeviceRegisterInputReportCallback(dev, buffer, sizeof buffer, report_cb, NULL);
	IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	cbcount = 0;
	sync_point("key-a");
	check("key A down: the element of usage 4 reads 1 (callback)", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardA, 1));
	check("key A down: GetValue of the usage 4 element is 1, of usage 5 is 0, and the second slot is empty", value_of(21) == 1 && value_of(22) == 0 && value_of(11) == 0);
	check("input report: slot 0 is usage 4", last_report_length == 8 && last_report[0] == 0 && last_report[2] == 0x04);
	cbcount = 0;
	sync_point("key-a-up");
	check("key A up: the element of usage 4 reads 0", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardA, 0) && value_of(21) == 0);
	cbcount = 0;
	sync_point("shift");
	check("Left Shift down (modifier element 3, usage 0xe1)", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardLeftShift, 1) && value_of(3) == 1 && value_of(2) == 0);
	check("input report: modifier byte is 0x02", last_report_length == 8 && last_report[0] == 0x02);
	cbcount = 0;
	sync_point("shift-up");
	check("Left Shift up", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardLeftShift, 0) && value_of(3) == 0);

	/* The Caps Lock LED (usage 2, cookie 274) is written to the host's node */
	check("SetValue of the Caps Lock LED", led_set(274, 1));
	check("the LED output element reads back 1", value_of(274) == 1 && value_of(273) == 0);
	sync_point("led-check");
	uint8_t out[1] = { 0x01 };
	check("SetReport of the LED output report (Num Lock)", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 0, out, 1) == kIOReturnSuccess);
	sync_point("led-check2");
}

int main(int argc, char **argv)
{
	IOHIDManagerRef mgr;
	if (argc < 3)
		return 2;
	ctl = argv[1];

	int kbd = strcmp(argv[2], "kbd") == 0, ids = strcmp(argv[2], "ids") == 0;
	mgr = start(0x1234, kbd ? 0x5602 : ids ? 0x5601 : 0x5600);
	sync_point("plug");
	wait_added();
	if (!added_count)
		return 1;
	if (kbd)
		run_kbd();
	else if (ids)
		run_ids();
	else
		run_f310();
	sync_point("unplug");
	for (int i = 0; i < 100 && !removed_count; i++)
		spin(0.05);
	check("device removed", removed_count == 1);
	IOHIDManagerClose(mgr, kIOHIDOptionsTypeNone);
	printf("%s\n", failures ? "FAILED" : "all passed");
	return failures != 0;
}
