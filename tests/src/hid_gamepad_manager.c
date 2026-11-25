/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * A game controller as a game sees it through the IOHIDManager API: matching
 * of GenericDesktop Joystick, GamePad and MultiAxisController, device-added
 * and -removed callbacks, element enumeration with every IOHIDElementGet*
 * property, input value callback, IOHIDDeviceGetValue, an IOHIDQueue,
 * scaled values and time stamps.
 *
 * The controller is a virtual evdev device on the host (uinput) that a driver
 * script creates and removes in lockstep: the test prints "SYNC: <tag>" and
 * waits for the file <controldir>/go-<tag>.
 * Usage: tests/hid-gamepad/run.sh <this test binary> <xbox|ds4>, which makes
 * the controller (tests/hid-gamepad/vpad.c) and answers the SYNC lines.
 * The test itself takes <controldir> <xbox|ds4>.
 * Link with -framework IOKit -framework CoreFoundation.
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
static int is_xbox;
static int is_bluetooth; /* the Xbox One controller of the Bluetooth kind (xboxbt) */
/* The layout the controller publishes: an Xbox One controller (standard layout: A is button 1, the
 * hat is 1..8, sticks 16 bits, triggers Brake and Accelerator) or a DualShock 4 (Cross is button 2,
 * the hat is 0..7, sticks 8 bits, triggers Rx and Ry) */
static int btn_usage, hat_lo, hat_hi, hat_value_shift, nbuttons;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
	fflush(stdout);
}

static void spin(double seconds)
{
	CFAbsoluteTime end = CFAbsoluteTimeGetCurrent() + seconds;

	/* The run loop returns at once when it has no source to wait for; the time must still pass */
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false);
	while (CFAbsoluteTimeGetCurrent() < end)
		usleep(5000);
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

static int num_prop(IOHIDDeviceRef dev, CFStringRef key, int *out)
{
	CFTypeRef v = IOHIDDeviceGetProperty(dev, key);
	return v && CFGetTypeID(v) == CFNumberGetTypeID() && CFNumberGetValue(v, kCFNumberIntType, out);
}

static int string_prop(IOHIDDeviceRef dev, CFStringRef key, char *out, size_t size)
{
	CFTypeRef v = IOHIDDeviceGetProperty(dev, key);
	return v && CFGetTypeID(v) == CFStringGetTypeID() && CFStringGetCString(v, out, size, kCFStringEncodingUTF8);
}

static CFMutableArrayRef matching_array(void)
{
	static const int pairs[3][2] = { { kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick },
		{ kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad }, { kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController } };
	CFMutableArrayRef array = CFArrayCreateMutable(NULL, 3, &kCFTypeArrayCallBacks);
	for (int i = 0; i < 3; i++)
	{
		CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFNumberRef page = CFNumberCreate(NULL, kCFNumberIntType, &pairs[i][0]);
		CFNumberRef usage = CFNumberCreate(NULL, kCFNumberIntType, &pairs[i][1]);
		CFDictionarySetValue(d, CFSTR(kIOHIDDeviceUsagePageKey), page);
		CFDictionarySetValue(d, CFSTR(kIOHIDDeviceUsageKey), usage);
		CFArrayAppendValue(array, d);
		CFRelease(page); CFRelease(usage); CFRelease(d);
	}
	return array;
}

static IOHIDDeviceRef added_dev, removed_dev;
static int added_count, removed_count;
static void added_cb(void *ctx, IOReturn r, void *sender, IOHIDDeviceRef dev)
{
	added_dev = dev; added_count++;
}
static void removed_cb(void *ctx, IOReturn r, void *sender, IOHIDDeviceRef dev)
{
	removed_dev = dev; removed_count++;
}

struct seen { uint32_t page, usage; CFIndex value; double scaled; uint64_t stamp; int count; };
#define MAXSEEN 64
static struct seen cbvals[MAXSEEN];
static int cbcount, cb_bad_sender;
static IOHIDDeviceRef cb_device;
static void value_cb(void *ctx, IOReturn r, void *sender, IOHIDValueRef v)
{
	IOHIDElementRef e = IOHIDValueGetElement(v);
	if (cbcount < MAXSEEN)
	{
		cbvals[cbcount].page = IOHIDElementGetUsagePage(e);
		cbvals[cbcount].usage = IOHIDElementGetUsage(e);
		cbvals[cbcount].value = IOHIDValueGetIntegerValue(v);
		cbvals[cbcount].scaled = IOHIDValueGetScaledValue(v, kIOHIDValueScaleTypePhysical);
		cbvals[cbcount].stamp = IOHIDValueGetTimeStamp(v);
		cbcount++;
	}
	if (sender != cb_device)
		cb_bad_sender++;
}

static int find_cb(uint32_t page, uint32_t usage, CFIndex value)
{
	for (int i = 0; i < cbcount; i++)
		if (cbvals[i].page == page && cbvals[i].usage == usage && cbvals[i].value == value)
			return i;
	return -1;
}

static int wait_cb(uint32_t page, uint32_t usage, CFIndex value)
{
	for (int i = 0; i < 100; i++)
	{
		if (find_cb(page, usage, value) >= 0)
			return 1;
		spin(0.05);
	}
	return 0;
}

static IOHIDElementRef find_element(CFArrayRef elems, uint32_t page, uint32_t usage)
{
	for (CFIndex i = 0; i < CFArrayGetCount(elems); i++)
	{
		IOHIDElementRef e = (IOHIDElementRef) CFArrayGetValueAtIndex(elems, i);
		if (IOHIDElementGetUsagePage(e) == page && IOHIDElementGetUsage(e) == usage)
			return e;
	}
	return NULL;
}

static CFIndex current(IOHIDDeviceRef dev, IOHIDElementRef e)
{
	IOHIDValueRef v = NULL;
	if (!e || IOHIDDeviceGetValue(dev, e, &v) != kIOReturnSuccess || !v)
		return -99999;
	return IOHIDValueGetIntegerValue(v);
}

static int drain_queue(IOHIDQueueRef q, uint32_t page, uint32_t usage, CFIndex value, double timeout)
{
	int found = 0;
	IOHIDValueRef v;
	while ((v = IOHIDQueueCopyNextValueWithTimeout(q, timeout)) != NULL)
	{
		IOHIDElementRef e = IOHIDValueGetElement(v);
		if (IOHIDElementGetUsagePage(e) == page && IOHIDElementGetUsage(e) == usage && IOHIDValueGetIntegerValue(v) == value)
			found = 1;
		CFRelease(v);
		timeout = 0.0;
	}
	return found;
}

static void dump_element(IOHIDElementRef e)
{
	IOHIDElementRef parent = IOHIDElementGetParent(e);
	printf("ELEM cookie=%u type=%d page=0x%x usage=0x%x coll=%d parent=%u lmin=%ld lmax=%ld pmin=%ld pmax=%ld size=%u count=%u rid=%u unit=%u exp=%u "
		"virt=%d rel=%d wrap=%d array=%d nonlin=%d pref=%d null=%d\n",
		(unsigned) IOHIDElementGetCookie(e), (int) IOHIDElementGetType(e), IOHIDElementGetUsagePage(e), IOHIDElementGetUsage(e),
		IOHIDElementGetType(e) == kIOHIDElementTypeCollection ? (int) IOHIDElementGetCollectionType(e) : -1,
		parent ? (unsigned) IOHIDElementGetCookie(parent) : 0,
		(long) IOHIDElementGetLogicalMin(e), (long) IOHIDElementGetLogicalMax(e), (long) IOHIDElementGetPhysicalMin(e), (long) IOHIDElementGetPhysicalMax(e),
		IOHIDElementGetReportSize(e), IOHIDElementGetReportCount(e), IOHIDElementGetReportID(e), IOHIDElementGetUnit(e), IOHIDElementGetUnitExponent(e),
		IOHIDElementIsVirtual(e), IOHIDElementIsRelative(e), IOHIDElementIsWrapping(e), IOHIDElementIsArray(e), IOHIDElementIsNonLinear(e),
		IOHIDElementHasPreferredState(e), IOHIDElementHasNullState(e));
}

int main(int argc, char **argv)
{
	IOHIDManagerRef mgr;
	CFMutableArrayRef match;
	CFSetRef devs;

	if (argc < 3)
		return 2;
	ctl = argv[1];
	is_xbox = strncmp(argv[2], "xbox", 4) == 0;
	is_bluetooth = strcmp(argv[2], "xboxbt") == 0 || !is_xbox;
	btn_usage = is_xbox ? 1 : 2;      /* the key BTN_SOUTH */
	hat_lo = is_xbox ? 1 : 0;
	hat_hi = hat_lo + 7;
	hat_value_shift = is_xbox ? 0 : 1; /* the value of a direction numbered 1 (up) .. 8 is d - shift */
	nbuttons = is_xbox ? 15 : 14;
	setvbuf(stdout, NULL, _IOLBF, 0);

	/* No controller yet */
	mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	check("manager created", mgr != NULL);
	match = matching_array();
	IOHIDManagerSetDeviceMatchingMultiple(mgr, match);
	IOHIDManagerRegisterDeviceMatchingCallback(mgr, added_cb, NULL);
	IOHIDManagerRegisterDeviceRemovalCallback(mgr, removed_cb, NULL);
	IOHIDManagerScheduleWithRunLoop(mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	check("open succeeds without a controller", IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone) == kIOReturnSuccess);
	spin(0.3);
	devs = IOHIDManagerCopyDevices(mgr);
	check("no controller: CopyDevices is NULL", devs == NULL);
	if (devs)
		CFRelease(devs);
	check("no controller: no device-added callback", added_count == 0);

	/* Hot-plug */
	sync_point("plug");
	for (int i = 0; i < 200 && added_count == 0; i++)
		spin(0.05);
	check("device-added callback fires on plug", added_count == 1 && added_dev != NULL);
	if (!added_dev)
		return 1;
	IOHIDDeviceRef dev = added_dev;
	devs = IOHIDManagerCopyDevices(mgr);
	check("CopyDevices has the controller", devs && CFSetGetCount(devs) == 1 && CFSetContainsValue(devs, dev));
	if (devs)
		CFRelease(devs);

	/* Properties */
	{
		int v; char s[128];
		check("Product", string_prop(dev, CFSTR(kIOHIDProductKey), s, sizeof s) &&
		    strstr(s, !is_xbox ? "Wireless Controller" : is_bluetooth ? "Xbox Wireless Controller" : "Xbox One"));
		/* Manufacturer comes from the USB string descriptor; a virtual (uinput) controller has none, so it is not checked */
		check("Manufacturer is absent or a string", IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDManufacturerKey)) == NULL || string_prop(dev, CFSTR(kIOHIDManufacturerKey), s, sizeof s));
		check("CountryCode is 0", num_prop(dev, CFSTR(kIOHIDCountryCodeKey), &v) && v == 0);
		check("VendorID", num_prop(dev, CFSTR(kIOHIDVendorIDKey), &v) && v == (is_xbox ? 0x045e : 0x054c));
		check("ProductID", num_prop(dev, CFSTR(kIOHIDProductIDKey), &v) && v == (!is_xbox ? 0x09cc : is_bluetooth ? 0x02fd : 0x02ea));
		check("VersionNumber", num_prop(dev, CFSTR(kIOHIDVersionNumberKey), &v) && v == (!is_xbox ? 0x8111 : is_bluetooth ? 0x0903 : 0x0408));
		check("Transport", string_prop(dev, CFSTR(kIOHIDTransportKey), s, sizeof s) && !strcmp(s, is_bluetooth ? "Bluetooth" : "USB"));
		check("PrimaryUsagePage is GenericDesktop", num_prop(dev, CFSTR(kIOHIDPrimaryUsagePageKey), &v) && v == kHIDPage_GenericDesktop);
		check("PrimaryUsage is GamePad", num_prop(dev, CFSTR(kIOHIDPrimaryUsageKey), &v) && v == kHIDUsage_GD_GamePad);
		check("LocationID is a number", num_prop(dev, CFSTR(kIOHIDLocationIDKey), &v));
		check("MaxInputReportSize > 0", num_prop(dev, CFSTR(kIOHIDMaxInputReportSizeKey), &v) && v > 0);
		check("ConformsTo GamePad", IOHIDDeviceConformsTo(dev, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad));
		check("does not conform to Keyboard", !IOHIDDeviceConformsTo(dev, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard));
		check("unknown property is NULL", IOHIDDeviceGetProperty(dev, CFSTR("NoSuchProperty")) == NULL);
		check("GetService returns a service", IOHIDDeviceGetService(dev) != MACH_PORT_NULL);
	}

	/* Elements */
	CFArrayRef elems = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	check("CopyMatchingElements returns elements", elems && CFArrayGetCount(elems) > 0);
	if (!elems)
		return 1;
	for (CFIndex i = 0; i < CFArrayGetCount(elems); i++)
		dump_element((IOHIDElementRef) CFArrayGetValueAtIndex(elems, i));

	IOHIDElementRef btnA = find_element(elems, kHIDPage_Button, btn_usage);
	IOHIDElementRef lx = find_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_X);
	IOHIDElementRef hat = find_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch);
	uint32_t trig_page = is_xbox ? kHIDPage_Simulation : kHIDPage_GenericDesktop;
	uint32_t trig_usage = is_xbox ? kHIDUsage_Sim_Brake : kHIDUsage_GD_Ry; /* ABS_Z (LT) of the Xbox pad, ABS_RZ (R2) of the DS4 */
	IOHIDElementRef trig = find_element(elems, trig_page, trig_usage);
	check("button element", btnA != NULL);
	check("X axis element", lx != NULL);
	check("hat switch element", hat != NULL);
	check("trigger element", trig != NULL);
	if (!btnA || !lx || !hat || !trig)
		return 1;

	check("button: type Input_Button", IOHIDElementGetType(btnA) == kIOHIDElementTypeInput_Button);
	check("button: logical 0..1", IOHIDElementGetLogicalMin(btnA) == 0 && IOHIDElementGetLogicalMax(btnA) == 1);
	check("button: report size 1", IOHIDElementGetReportSize(btnA) == 1);
	check("axis: type Input_Misc", IOHIDElementGetType(lx) == kIOHIDElementTypeInput_Misc);
	check("axis: logical range is unsigned, from 0", is_xbox ? (IOHIDElementGetLogicalMin(lx) == 0 && IOHIDElementGetLogicalMax(lx) == 65535)
		: (IOHIDElementGetLogicalMin(lx) == 0 && IOHIDElementGetLogicalMax(lx) == 255));
	check("axis: physical range follows logical", IOHIDElementGetPhysicalMin(lx) == IOHIDElementGetLogicalMin(lx)
		&& IOHIDElementGetPhysicalMax(lx) == IOHIDElementGetLogicalMax(lx));
	check("axis: report size", IOHIDElementGetReportSize(lx) == (is_xbox ? 16 : 8));
	check("axis: not relative, not array, not virtual", !IOHIDElementIsRelative(lx) && !IOHIDElementIsArray(lx) && !IOHIDElementIsVirtual(lx));
	check("axis: no null state", !IOHIDElementHasNullState(lx));
	check("hat: logical range 8 directions", IOHIDElementGetLogicalMin(hat) == hat_lo && IOHIDElementGetLogicalMax(hat) == hat_hi);
	check("hat: physical 0..315 degrees", IOHIDElementGetPhysicalMin(hat) == 0 && IOHIDElementGetPhysicalMax(hat) == 315);
	check("hat: has null state", IOHIDElementHasNullState(hat));
	check("hat: report size 4", IOHIDElementGetReportSize(hat) == 4);
	check("trigger: logical range", IOHIDElementGetLogicalMin(trig) == 0 && IOHIDElementGetLogicalMax(trig) == (is_xbox ? 1023 : 255));
	check("trigger: report size", IOHIDElementGetReportSize(trig) == (is_xbox ? 10 : 8));
	check("right stick is Z and Rz", find_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_Z) && find_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_Rz));
	if (is_xbox)
		check("second trigger is Accelerator", find_element(elems, kHIDPage_Simulation, kHIDUsage_Sim_Accelerator) != NULL);
	else
		check("second trigger is Rx", find_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_Rx) != NULL);
	/* Button usages are contiguous, 1 .. N, in the order of the report; a button the controller lacks is still there */
	{
		int contiguous = 1;
		for (int u = 1; u <= nbuttons; u++)
		{
			IOHIDElementRef b = find_element(elems, kHIDPage_Button, u);
			contiguous = contiguous && b && IOHIDElementGetType(b) == kIOHIDElementTypeInput_Button;
		}
		check("button usages are 1 .. N, all input buttons", contiguous);
		check("no button above N", find_element(elems, kHIDPage_Button, nbuttons + 1) == NULL);
	}
	{
		IOHIDElementRef parent = IOHIDElementGetParent(btnA);
		check("button parent is the application collection", parent && IOHIDElementGetType(parent) == kIOHIDElementTypeCollection
			&& IOHIDElementGetCollectionType(parent) == kIOHIDElementCollectionTypeApplication
			&& IOHIDElementGetUsagePage(parent) == kHIDPage_GenericDesktop && IOHIDElementGetUsage(parent) == kHIDUsage_GD_GamePad);
		check("collection has no parent", parent && IOHIDElementGetParent(parent) == NULL);
		check("device of an element", IOHIDElementGetDevice(btnA) == dev);
		check("cookies are unique", IOHIDElementGetCookie(btnA) != IOHIDElementGetCookie(lx) && IOHIDElementGetCookie(lx) != IOHIDElementGetCookie(hat));
	}
	{
		CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		int page = kHIDPage_Button;
		CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &page);
		CFDictionarySetValue(d, CFSTR(kIOHIDElementUsagePageKey), n);
		CFArrayRef buttons = IOHIDDeviceCopyMatchingElements(dev, d, kIOHIDOptionsTypeNone);
		check("matching elements by usage page: buttons", buttons && CFArrayGetCount(buttons) == nbuttons);
		if (buttons)
			CFRelease(buttons);
		CFRelease(n); CFRelease(d);
	}

	/* Input value callback and queue */
	cb_device = dev;
	IOHIDDeviceRegisterInputValueCallback(dev, value_cb, NULL);
	IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	IOHIDQueueRef queue = IOHIDQueueCreate(kCFAllocatorDefault, dev, 64, kIOHIDOptionsTypeNone);
	check("queue created", queue != NULL);
	for (CFIndex i = 0; i < CFArrayGetCount(elems); i++)
	{
		IOHIDElementRef e = (IOHIDElementRef) CFArrayGetValueAtIndex(elems, i);
		if (IOHIDElementGetType(e) != kIOHIDElementTypeCollection)
			IOHIDQueueAddElement(queue, e);
	}
	IOHIDQueueStart(queue);

	sync_point("rest");
	spin(0.3);
	check("at rest: button is 0", current(dev, btnA) == 0);
	check("at rest: hat is outside its range (null state)", current(dev, hat) < hat_lo || current(dev, hat) > hat_hi);
	check("GetValue on a value that never changed returns a value", current(dev, lx) != -99999);

	cbcount = 0;
	sync_point("press");
	check("button press: callback", wait_cb(kHIDPage_Button, btn_usage, 1));
	check("button press: GetValue", current(dev, btnA) == 1);
	check("button press: queue", drain_queue(queue, kHIDPage_Button, btn_usage, 1, 1.0));
	{
		int i = find_cb(kHIDPage_Button, btn_usage, 1);
		check("callback sender is the device", cb_bad_sender == 0);
		check("time stamp is set", i >= 0 && cbvals[i].stamp != 0);
	}
	sync_point("release");
	check("button release: callback", wait_cb(kHIDPage_Button, btn_usage, 0));
	check("button release: queue", drain_queue(queue, kHIDPage_Button, btn_usage, 0, 1.0));

	sync_point("stick");
	{
		CFIndex hi = is_xbox ? 65535 : 255;
		check("stick full right: callback", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, hi));
		check("stick full right: GetValue", current(dev, lx) == hi);
		check("stick full right: queue", drain_queue(queue, kHIDPage_GenericDesktop, kHIDUsage_GD_X, hi, 1.0));
		IOHIDValueRef v = NULL;
		IOHIDDeviceGetValue(dev, lx, &v);
		check("scaled value (physical) at maximum is 1.0 or the physical maximum",
			v && (IOHIDValueGetScaledValue(v, kIOHIDValueScaleTypePhysical) == (double) hi));
		check("scaled value (calibrated) is within -1..1 or equals the range",
			v && IOHIDValueGetScaledValue(v, kIOHIDValueScaleTypeCalibrated) >= -1.0);
		check("value length is the report size in bytes", v && IOHIDValueGetLength(v) == (is_xbox ? 2 : 1));
		check("value element is the axis", v && IOHIDValueGetElement(v) == lx);
	}
	sync_point("stick-low");
	{
		CFIndex lo = 0;
		check("stick full left: callback", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, lo));
		check("stick full left: GetValue", current(dev, lx) == lo);
	}
	sync_point("trigger");
	{
		CFIndex hi = is_xbox ? 1023 : 255;
		check("trigger: callback", wait_cb(trig_page, trig_usage, hi));
		check("trigger: GetValue", current(dev, trig) == hi);
	}
	/* direction d: 1 up, 2 up-right, 3 right, ... 7 left, 8 up-left */
	cbcount = 0;
	sync_point("hat-up");
	check("hat up (callback)", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, 1 - hat_value_shift));
	check("hat up: GetValue", current(dev, hat) == 1 - hat_value_shift);
	check("hat up: queue", drain_queue(queue, kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, 1 - hat_value_shift, 1.0));
	sync_point("hat-upright");
	check("hat up-right", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, 2 - hat_value_shift));
	sync_point("hat-left");
	check("hat left", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Hatswitch, 7 - hat_value_shift));
	sync_point("hat-center");
	spin(0.3);
	check("hat centred is outside its range (null state)", current(dev, hat) < hat_lo || current(dev, hat) > hat_hi);

	IOHIDQueueStop(queue);
	CFRelease(queue);

	/* A second manager started while the controller is present */
	{
		IOHIDManagerRef m2 = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		CFArrayRef a2 = matching_array();
		int saved = added_count;
		IOHIDManagerSetDeviceMatchingMultiple(m2, a2);
		IOHIDManagerRegisterDeviceMatchingCallback(m2, added_cb, NULL);
		IOHIDManagerScheduleWithRunLoop(m2, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		spin(0.5);
		check("existing device reported to a callback after Schedule (before Open)", added_count == saved + 1);
		check("Open of the second manager succeeds", IOHIDManagerOpen(m2, kIOHIDOptionsTypeNone) == kIOReturnSuccess);
		spin(0.3);
		CFSetRef d2 = IOHIDManagerCopyDevices(m2);
		check("second manager sees the device", d2 && CFSetGetCount(d2) == 1);
		if (d2)
			CFRelease(d2);
		IOHIDManagerClose(m2, kIOHIDOptionsTypeNone);
		IOHIDManagerUnscheduleFromRunLoop(m2, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		CFRelease(a2);
		CFRelease(m2);
	}

	/* Unplug */
	sync_point("unplug");
	for (int i = 0; i < 200 && removed_count == 0; i++)
		spin(0.05);
	check("device-removed callback fires on unplug", removed_count == 1 && removed_dev == dev);
	devs = IOHIDManagerCopyDevices(mgr);
	check("CopyDevices is NULL after unplug", devs == NULL);
	if (devs)
		CFRelease(devs);

	IOHIDManagerClose(mgr, kIOHIDOptionsTypeNone);
	CFRelease(mgr);
	CFRelease(match);
	printf("%s\n", failures ? "FAILED" : "OK");
	return failures != 0;
}
