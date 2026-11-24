/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Keyboards, mice and output reports as an application sees them through the
 * IOHIDManager API.
 *
 * Mode "kbd": a keyboard is matched as Keyboard (and not as Mouse); its
 * properties, the key and modifier elements and the LED output elements; key
 * values (callback, GetValue, input report callback, report bytes); LEDs
 * through IOHIDDeviceSetValue and IOHIDDeviceSetReport (synchronous and with
 * a callback), GetReport of the output report and the errors of bad reports.
 * The completion callbacks of IOHIDDeviceSetReportWithCallback and
 * IOHIDDeviceGetReportWithCallback (once, on the run loop or the dispatch queue
 * the device is scheduled with, with the result, the report and its length;
 * none for a call that fails) and of the asynchronous report calls of the
 * IOHIDDeviceInterface plug-in interface.
 * Mode "mouse": matched as Mouse and as Pointer; buttons, X, Y, wheel and AC
 * Pan elements (relative) and their values.
 * Modes "xboxff", "ds4ff" and "ds4usbff": a game controller with force feedback has the
 * output report of its layout (Xbox One report 3, DualShock 4 report 17 on Bluetooth and 5 on USB):
 * SetReport and SetValue on its output elements make the host upload and
 * play a rumble effect, and a report that enables no motor stops it.
 *
 * The device is a virtual evdev device on the host (uinput) that a driver
 * script creates and removes in lockstep: the test prints "SYNC: <tag>" and
 * waits for the file <controldir>/go-<tag>. What the host sees of the device
 * (LED events, force-feedback uploads and plays) is in <controldir>/vpad.log
 * after the tag "log".
 * Usage: tests/hid-gamepad/run.sh <this test binary> <kbd|mouse|xboxff|ds4ff|ds4usbff>.
 * Link with -framework IOKit -framework CoreFoundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <dispatch/dispatch.h>
#include <pthread.h>

static int failures;
static const char *ctl;

static void expect(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
	fflush(stdout);
}

/* Runs the run loop for the time, also when it has no sources (the call then returns at once) */
static void spin(double seconds)
{
	CFAbsoluteTime end = CFAbsoluteTimeGetCurrent() + seconds;

	do
	{
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);
		usleep(5000);
	}
	while (CFAbsoluteTimeGetCurrent() < end);
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

static IOHIDManagerRef make_manager(int page, int usage)
{
	IOHIDManagerRef mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFNumberRef p = CFNumberCreate(NULL, kCFNumberIntType, &page);
	CFNumberRef u = CFNumberCreate(NULL, kCFNumberIntType, &usage);

	CFDictionarySetValue(d, CFSTR(kIOHIDDeviceUsagePageKey), p);
	CFDictionarySetValue(d, CFSTR(kIOHIDDeviceUsageKey), u);
	IOHIDManagerSetDeviceMatching(mgr, d);
	CFRelease(p);
	CFRelease(u);
	CFRelease(d);
	IOHIDManagerScheduleWithRunLoop(mgr, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	return mgr;
}

static IOHIDDeviceRef first_device(IOHIDManagerRef mgr)
{
	CFSetRef devs = IOHIDManagerCopyDevices(mgr);
	IOHIDDeviceRef dev = NULL;

	if (devs && CFSetGetCount(devs) == 1)
	{
		IOHIDDeviceRef one;
		CFSetGetValues(devs, (const void **) &one);
		dev = one;
	}
	if (devs)
		CFRelease(devs);
	return dev;
}

static int device_count(IOHIDManagerRef mgr)
{
	CFSetRef devs = IOHIDManagerCopyDevices(mgr);
	int n = devs ? (int) CFSetGetCount(devs) : 0;

	if (devs)
		CFRelease(devs);
	return n;
}

/* The elements with a usage, in cookie order */
static IOHIDElementRef nth_element(CFArrayRef elems, uint32_t page, uint32_t usage, int n)
{
	for (CFIndex i = 0; i < CFArrayGetCount(elems); i++)
	{
		IOHIDElementRef e = (IOHIDElementRef) CFArrayGetValueAtIndex(elems, i);
		if (IOHIDElementGetUsagePage(e) == page && IOHIDElementGetUsage(e) == usage && n-- == 0)
			return e;
	}
	return NULL;
}

static int count_page(CFArrayRef elems, uint32_t page, IOHIDElementType type)
{
	int n = 0;

	for (CFIndex i = 0; i < CFArrayGetCount(elems); i++)
	{
		IOHIDElementRef e = (IOHIDElementRef) CFArrayGetValueAtIndex(elems, i);
		n += IOHIDElementGetUsagePage(e) == page && IOHIDElementGetType(e) == type;
	}
	return n;
}

struct seen { uint32_t page, usage; CFIndex value; };
#define MAXSEEN 128
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

static CFIndex current(IOHIDDeviceRef dev, IOHIDElementRef e)
{
	IOHIDValueRef v = NULL;
	if (!e || IOHIDDeviceGetValue(dev, e, &v) != kIOReturnSuccess || !v)
		return -99999;
	return IOHIDValueGetIntegerValue(v);
}

static uint8_t last_report[64];
static CFIndex last_report_length;
static uint32_t last_report_id = 99;
static int report_cb_count;
static void report_cb(void *ctx, IOReturn r, void *sender, IOHIDReportType type, uint32_t id, uint8_t *report, CFIndex length)
{
	last_report_length = length < (CFIndex) sizeof last_report ? length : (CFIndex) sizeof last_report;
	memcpy(last_report, report, last_report_length);
	last_report_id = id;
	report_cb_count++;
}

/* What the completion callback of a report call with a callback got */
static int async_done;
static IOReturn async_result = -1;
static void *async_ctx, *async_sender;
static IOHIDReportType async_type;
static uint32_t async_id;
static uint8_t *async_report;
static CFIndex async_length;
static void async_cb(void *ctx, IOReturn r, void *sender, IOHIDReportType type, uint32_t id, uint8_t *report, CFIndex length)
{
	async_result = r;
	async_ctx = ctx;
	async_sender = sender;
	async_type = type;
	async_id = id;
	async_report = report;
	async_length = length;
	async_done++;
}

static void async_reset(void)
{
	async_done = 0;
	async_result = -1;
	async_ctx = async_sender = NULL;
	async_report = NULL;
	async_length = -1;
}

/* Waits for the first completion, then for a while to see that no second one follows */
static int async_once(void)
{
	for (int i = 0; i < 100 && async_done == 0; i++)
		spin(0.05);
	spin(0.3);
	return async_done == 1;
}

/* The host's view of the device: the lines of vpad.log */
static char logtext[16384];
static void read_log(void)
{
	static int serial;
	char path[512], tag[32];
	FILE *f;
	size_t n;

	snprintf(tag, sizeof tag, "log%d", ++serial);   /* the driver answers log* by copying the host's view */
	sync_point(tag);
	snprintf(path, sizeof path, "%s/vpad.log", ctl);
	logtext[0] = 0;
	f = fopen(path, "r");
	if (!f)
		return;
	n = fread(logtext, 1, sizeof logtext - 1, f);
	logtext[n] = 0;
	fclose(f);
}

/* The state of the five LEDs after the last LED events in the log; -1 if there are none */
static int led_state(void)
{
	int state[5] = { -1, -1, -1, -1, -1 };
	const char *p = logtext;

	while ((p = strstr(p, "led ")) != NULL)
	{
		unsigned code; int value;
		if (sscanf(p, "led %u %d", &code, &value) == 2 && code < 5)
			state[code] = value;
		p += 4;
	}
	if (state[0] < 0)
		return -1;
	return state[0] | state[1] << 1 | state[2] << 2 | state[3] << 3 | state[4] << 4;
}


/* The host's view after the device had time to take what was sent: reads the log until the LEDs are as expected */
static int led_state_is(int expected)
{
	for (int i = 0; i < 5; i++)
	{
		read_log();
		if (led_state() == expected)
			return 1;
	}
	return 0;
}

/* The asynchronous report calls of the IOHIDDeviceInterface plug-in interface: the callback gets the
 * target, the result, the refcon, the sender and the length */
static int old_done;
static IOReturn old_result = -1;
static void *old_target, *old_refcon;
static uint32_t old_size;
static void old_cb(void *target, IOReturn result, void *refcon, void *sender, uint32_t size)
{
	old_result = result;
	old_target = target;
	old_refcon = refcon;
	old_size = size;
	old_done++;
}

static int old_once(void)
{
	for (int i = 0; i < 100 && old_done == 0; i++)
		spin(0.05);
	spin(0.3);
	return old_done == 1;
}

static void test_obsolete_interface(IOHIDDeviceRef dev)
{
	IOCFPlugInInterface **plug = NULL;
	IOHIDDeviceInterface122 **hid = NULL;
	SInt32 score;
	static int ta, tb;

	IOReturn r = IOCreatePlugInInterfaceForService(IOHIDDeviceGetService(dev), kIOHIDDeviceUserClientTypeID,
		kIOCFPlugInInterfaceID, &plug, &score);
	expect("the IOHIDDeviceInterface plug-in loads", r == kIOReturnSuccess && plug);
	if (!plug)
		return;
	HRESULT hr = (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID122), (LPVOID *) &hid);
	IODestroyPlugInInterface(plug);
	expect("the IOHIDDeviceInterface122 is available", hr == S_OK && hid);
	if (!hid)
		return;
	expect("the old interface opens", (*hid)->open(hid, 0) == kIOReturnSuccess);

	CFRunLoopSourceRef src = NULL;
	expect("the old interface has an async event source", (*hid)->createAsyncEventSource(hid, &src) == kIOReturnSuccess && src);
	if (src)
		CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopDefaultMode);

	uint8_t leds = 0x01;
	old_done = 0;
	r = (*hid)->setReport(hid, kIOHIDReportTypeOutput, 0, &leds, 1, 5000, old_cb, &ta, &tb);
	expect("IOHIDDeviceInterface setReport with a callback is accepted", r == kIOReturnSuccess);
	expect("IOHIDDeviceInterface setReport: the callback is called once", old_once());
	expect("IOHIDDeviceInterface setReport: success, target, refcon and length 1",
		old_result == kIOReturnSuccess && old_target == &ta && old_refcon == &tb && old_size == 1);
	expect("host got Num Lock from the old interface", led_state_is(0x01));

	uint8_t got[4] = { 0xff };
	uint32_t size = sizeof got;
	old_done = 0;
	r = (*hid)->getReport(hid, kIOHIDReportTypeOutput, 0, got, &size, 5000, old_cb, &ta, &tb);
	expect("IOHIDDeviceInterface getReport with a callback is accepted", r == kIOReturnSuccess);
	expect("IOHIDDeviceInterface getReport: the callback is called once", old_once());
	expect("IOHIDDeviceInterface getReport: success, target, refcon and length 1, the LED state in the buffer",
		old_result == kIOReturnSuccess && old_target == &ta && old_refcon == &tb && old_size == 1 && got[0] == 0x01);

	uint8_t bad[2] = { 1, 1 };
	old_done = 0;
	r = (*hid)->setReport(hid, kIOHIDReportTypeOutput, 0, bad, 2, 5000, old_cb, &ta, &tb);
	spin(0.5);
	expect("IOHIDDeviceInterface setReport of a bad report fails without a callback", r == kIOReturnBadArgument && old_done == 0);

	if (src)
	{
		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), src, kCFRunLoopDefaultMode);
		CFRelease(src);
	}
	(*hid)->close(hid);
	(*hid)->Release(hid);
}

/* The completion of a device scheduled with a dispatch queue comes on that queue */
static const char queue_key;
static int queue_done;
static void queue_cb(void *ctx, IOReturn r, void *sender, IOHIDReportType type, uint32_t id, uint8_t *report, CFIndex length)
{
	async_cb(ctx, r, sender, type, id, report, length);
	queue_done = dispatch_get_specific(&queue_key) == &queue_key && !pthread_main_np();
}

static void test_dispatch_queue(void)
{
	dispatch_queue_t q = dispatch_queue_create("hid-test-queue", DISPATCH_QUEUE_SERIAL);
	dispatch_queue_set_specific(q, &queue_key, (void *) &queue_key, NULL);

	IOHIDManagerRef mgr = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
	int page = kHIDPage_GenericDesktop, usage = kHIDUsage_GD_Keyboard;
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFNumberRef p = CFNumberCreate(NULL, kCFNumberIntType, &page), u = CFNumberCreate(NULL, kCFNumberIntType, &usage);
	CFDictionarySetValue(d, CFSTR(kIOHIDDeviceUsagePageKey), p);
	CFDictionarySetValue(d, CFSTR(kIOHIDDeviceUsageKey), u);
	IOHIDManagerSetDeviceMatching(mgr, d);
	CFRelease(p);
	CFRelease(u);
	CFRelease(d);
	IOHIDManagerSetDispatchQueue(mgr, q);
	IOHIDManagerActivate(mgr);
	expect("the dispatch queue manager opens", IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone) == kIOReturnSuccess);
	for (int i = 0; i < 100 && device_count(mgr) == 0; i++)
		usleep(50000);
	IOHIDDeviceRef dev = first_device(mgr);
	expect("the dispatch queue manager has the keyboard", dev != NULL);
	if (dev)
	{
		static uint8_t leds = 0x03;
		static int token;
		async_reset();
		queue_done = 0;
		IOReturn r = IOHIDDeviceSetReportWithCallback(dev, kIOHIDReportTypeOutput, 0, &leds, 1, 5.0, queue_cb, &token);
		expect("SetReportWithCallback on a dispatch queue device is accepted", r == kIOReturnSuccess);
		for (int i = 0; i < 100 && async_done == 0; i++)
			usleep(50000);
		usleep(300000);
		expect("the callback is called once, on the queue", async_done == 1 && queue_done);
		expect("the queue callback got success, the context and length 1", async_result == kIOReturnSuccess && async_ctx == &token && async_length == 1);
		expect("host got Num Lock and Caps Lock from the queue report", led_state_is(0x03));
	}
	IOHIDManagerCancel(mgr);
}


static void test_keyboard(void)
{
	IOHIDManagerRef kbd = make_manager(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
	IOHIDManagerRef mouse = make_manager(kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse);
	int v;
	char s[128];

	IOHIDManagerRegisterInputValueCallback(kbd, value_cb, NULL);
	expect("keyboard manager opens", IOHIDManagerOpen(kbd, kIOHIDOptionsTypeNone) == kIOReturnSuccess);
	expect("mouse manager opens", IOHIDManagerOpen(mouse, kIOHIDOptionsTypeNone) == kIOReturnSuccess);
	sync_point("plug");
	for (int i = 0; i < 200 && device_count(kbd) == 0; i++)
		spin(0.05);
	spin(0.3);
	expect("keyboard is matched as Keyboard", device_count(kbd) == 1);
	expect("keyboard is not matched as Mouse", device_count(mouse) == 0);

	IOHIDDeviceRef dev = first_device(kbd);
	if (!dev)
		return;

	expect("Product", string_prop(dev, CFSTR(kIOHIDProductKey), s, sizeof s) && !strcmp(s, "Vpad Keyboard"));
	expect("VendorID", num_prop(dev, CFSTR(kIOHIDVendorIDKey), &v) && v == 0x046d);
	expect("ProductID", num_prop(dev, CFSTR(kIOHIDProductIDKey), &v) && v == 0xc31c);
	expect("Transport", string_prop(dev, CFSTR(kIOHIDTransportKey), s, sizeof s) && !strcmp(s, "USB"));
	expect("PrimaryUsagePage", num_prop(dev, CFSTR(kIOHIDPrimaryUsagePageKey), &v) && v == kHIDPage_GenericDesktop);
	expect("PrimaryUsage is Keyboard", num_prop(dev, CFSTR(kIOHIDPrimaryUsageKey), &v) && v == kHIDUsage_GD_Keyboard);
	expect("MaxInputReportSize is 15", num_prop(dev, CFSTR(kIOHIDMaxInputReportSizeKey), &v) && v == 15);
	expect("MaxOutputReportSize is 1", num_prop(dev, CFSTR(kIOHIDMaxOutputReportSizeKey), &v) && v == 1);
	expect("ConformsTo Keyboard", IOHIDDeviceConformsTo(dev, kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard));
	expect("does not conform to Mouse", !IOHIDDeviceConformsTo(dev, kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse));
	expect("has a ReportDescriptor", IOHIDDeviceGetProperty(dev, CFSTR(kIOHIDReportDescriptorKey)) != NULL);

	CFArrayRef elems = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	expect("elements", elems && CFArrayGetCount(elems) > 0);
	if (!elems)
		return;
	IOHIDElementRef keyA = nth_element(elems, kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardA, 0);
	IOHIDElementRef shift = nth_element(elems, kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardLeftShift, 0);
	IOHIDElementRef caps = nth_element(elems, kHIDPage_LEDs, kHIDUsage_LED_CapsLock, 0);
	IOHIDElementRef num = nth_element(elems, kHIDPage_LEDs, kHIDUsage_LED_NumLock, 0);
	expect("key element A", keyA && IOHIDElementGetType(keyA) == kIOHIDElementTypeInput_Button
		&& IOHIDElementGetLogicalMin(keyA) == 0 && IOHIDElementGetLogicalMax(keyA) == 1 && IOHIDElementGetReportSize(keyA) == 1);
	expect("modifier element Left Shift", shift && IOHIDElementGetType(shift) == kIOHIDElementTypeInput_Button);
	expect("112 key and 8 modifier button elements", count_page(elems, kHIDPage_KeyboardOrKeypad, kIOHIDElementTypeInput_Button) == 120);
	expect("LED element Caps Lock is an output element", caps && IOHIDElementGetType(caps) == kIOHIDElementTypeOutput
		&& IOHIDElementGetReportID(caps) == 0 && IOHIDElementGetLogicalMax(caps) == 1);
	expect("five LED output elements", count_page(elems, kHIDPage_LEDs, kIOHIDElementTypeOutput) == 5);
	IOHIDElementRef app = IOHIDElementGetParent(keyA);
	expect("elements are in the Application collection of the Keyboard", app && IOHIDElementGetType(app) == kIOHIDElementTypeCollection
		&& IOHIDElementGetCollectionType(app) == kIOHIDElementCollectionTypeApplication
		&& IOHIDElementGetUsagePage(app) == kHIDPage_GenericDesktop && IOHIDElementGetUsage(app) == kHIDUsage_GD_Keyboard);

	/* Key values */
	uint8_t buffer[64];
	IOHIDDeviceRegisterInputReportCallback(dev, buffer, sizeof buffer, report_cb, NULL);
	cbcount = 0;
	sync_point("key-a");
	expect("key A pressed (callback)", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardA, 1));
	expect("key A pressed (GetValue)", current(dev, keyA) == 1);
	expect("input report callback", report_cb_count > 0 && last_report_length == 15 && last_report_id == 0);
	expect("input report: key A is bit 0 of byte 1", last_report_length == 15 && last_report[0] == 0 && last_report[1] == 0x01);
	sync_point("key-a-up");
	expect("key A released (callback)", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardA, 0));
	expect("key A released (GetValue)", current(dev, keyA) == 0);
	sync_point("shift");
	expect("Left Shift pressed (callback)", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardLeftShift, 1));
	expect("input report: Left Shift is bit 1 of byte 0", last_report_length == 15 && last_report[0] == 0x02);
	sync_point("shift-up");
	expect("Left Shift released", wait_cb(kHIDPage_KeyboardOrKeypad, kHIDUsage_KeyboardLeftShift, 0));

	/* LEDs through SetValue */
	IOReturn r;
	IOHIDValueRef on = IOHIDValueCreateWithIntegerValue(NULL, caps, 0, 1);
	r = IOHIDDeviceSetValue(dev, caps, on);
	expect("SetValue on the Caps Lock LED", r == kIOReturnSuccess);
	CFRelease(on);
	expect("host got the Caps Lock LED", led_state_is(0x02));
	on = IOHIDValueCreateWithIntegerValue(NULL, num, 0, 1);
	expect("SetValue on the Num Lock LED", IOHIDDeviceSetValue(dev, num, on) == kIOReturnSuccess);
	CFRelease(on);
	expect("host got Num Lock and Caps Lock (the other LED stays)", led_state_is(0x03));

	/* LEDs through SetReport / GetReport */
	uint8_t leds = 0x05;
	expect("SetReport Output 0: Num Lock and Scroll Lock", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 0, &leds, 1) == kIOReturnSuccess);
	expect("host got Num Lock and Scroll Lock", led_state_is(0x05));
	uint8_t got[4] = { 0xff };
	CFIndex length = sizeof got;
	expect("GetReport Output 0 is what was sent", IOHIDDeviceGetReport(dev, kIOHIDReportTypeOutput, 0, got, &length) == kIOReturnSuccess
		&& length == 1 && got[0] == 0x05);
	leds = 0xe0;                                                    /* only padding bits: all LEDs off */
	expect("padding bits are not LEDs", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 0, &leds, 1) == kIOReturnSuccess);
	expect("host got all LEDs off", led_state_is(0x00));

	/* With a callback: the call returns at once, the callback comes later from the run loop, once */
	static int token;
	leds = 0x02;
	async_reset();
	r = IOHIDDeviceSetReportWithCallback(dev, kIOHIDReportTypeOutput, 0, &leds, 1, 5.0, async_cb, &token);
	expect("SetReportWithCallback is accepted", r == kIOReturnSuccess);
	expect("the callback is not called from inside the call", async_done == 0);
	expect("SetReportWithCallback: the callback is called once", async_once());
	expect("SetReportWithCallback: result is success", async_result == kIOReturnSuccess);
	expect("SetReportWithCallback: the context is the caller's", async_ctx == &token);
	expect("SetReportWithCallback: the sender is the device", async_sender == (void *) dev);
	expect("SetReportWithCallback: type Output, report ID 0", async_type == kIOHIDReportTypeOutput && async_id == 0);
	expect("SetReportWithCallback: the report and its length are the caller's", async_report == &leds && async_length == 1);
	expect("host got Caps Lock from the report with a callback", led_state_is(0x02));

	uint8_t buf[4] = { 0xff };
	length = sizeof buf;
	async_reset();
	r = IOHIDDeviceGetReportWithCallback(dev, kIOHIDReportTypeOutput, 0, buf, &length, 5.0, async_cb, &token);
	expect("GetReportWithCallback is accepted", r == kIOReturnSuccess);
	expect("GetReportWithCallback: the callback is called once", async_once());
	expect("GetReportWithCallback: result is success", async_result == kIOReturnSuccess);
	expect("GetReportWithCallback: the context is the caller's", async_ctx == &token);
	expect("GetReportWithCallback: the sender is the device", async_sender == (void *) dev);
	expect("GetReportWithCallback: type Output, report ID 0", async_type == kIOHIDReportTypeOutput && async_id == 0);
	expect("GetReportWithCallback: the report is the caller's buffer with the LED state", async_report == buf && buf[0] == 0x02);
	expect("GetReportWithCallback: the length is 1", async_length == 1);

	/* A timeout is for a call that does not complete in time; these complete at once */
	leds = 0x04;
	async_reset();
	r = IOHIDDeviceSetReportWithCallback(dev, kIOHIDReportTypeOutput, 0, &leds, 1, 0.001, async_cb, &token);
	expect("SetReportWithCallback with a very short timeout is accepted", r == kIOReturnSuccess);
	expect("a short timeout does not fail a call that completes at once", async_once() && async_result == kIOReturnSuccess);
	expect("host got Scroll Lock from the short-timeout report", led_state_is(0x04));

	/* A call that fails returns the error and never calls back */
	uint8_t bad[2] = { 1, 1 };
	async_reset();
	r = IOHIDDeviceSetReportWithCallback(dev, kIOHIDReportTypeOutput, 0, bad, 2, 5.0, async_cb, &token);
	expect("SetReportWithCallback of a 2-byte LED report is refused", r == kIOReturnBadArgument);
	length = sizeof buf;
	r = IOHIDDeviceGetReportWithCallback(dev, kIOHIDReportTypeOutput, 7, buf, &length, 5.0, async_cb, &token);
	expect("GetReportWithCallback of an unknown report is refused", r != kIOReturnSuccess);
	spin(0.5);
	expect("no callback for a call that failed", async_done == 0);

	test_obsolete_interface(dev);

	/* Errors */
	uint8_t two[2] = { 1, 1 };
	expect("a 2-byte LED report is refused", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 0, two, 2) == kIOReturnBadArgument);
	expect("feature reports are unsupported", IOHIDDeviceSetReport(dev, kIOHIDReportTypeFeature, 0, &leds, 1) != kIOReturnSuccess);
	expect("an output report that does not exist", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 7, two, 2) != kIOReturnSuccess);
	expect("SetValue on an input element is refused", ({
		IOHIDValueRef one = IOHIDValueCreateWithIntegerValue(NULL, keyA, 0, 1);
		IOReturn rr = IOHIDDeviceSetValue(dev, keyA, one);
		CFRelease(one);
		rr != kIOReturnSuccess;
	}));

	CFRelease(elems);
	test_dispatch_queue();
	sync_point("unplug");
	for (int i = 0; i < 200 && device_count(kbd) != 0; i++)
		spin(0.05);
	expect("keyboard removed", device_count(kbd) == 0);
	IOHIDManagerClose(kbd, kIOHIDOptionsTypeNone);
	IOHIDManagerClose(mouse, kIOHIDOptionsTypeNone);
}

static void test_mouse(void)
{
	IOHIDManagerRef mouse = make_manager(kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse);
	IOHIDManagerRef pointer = make_manager(kHIDPage_GenericDesktop, kHIDUsage_GD_Pointer);
	IOHIDManagerRef kbd = make_manager(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
	int v;
	char s[128];

	IOHIDManagerRegisterInputValueCallback(mouse, value_cb, NULL);
	IOHIDManagerOpen(mouse, kIOHIDOptionsTypeNone);
	IOHIDManagerOpen(pointer, kIOHIDOptionsTypeNone);
	IOHIDManagerOpen(kbd, kIOHIDOptionsTypeNone);
	sync_point("plug");
	for (int i = 0; i < 200 && device_count(mouse) == 0; i++)
		spin(0.05);
	spin(0.3);
	expect("mouse is matched as Mouse", device_count(mouse) == 1);
	expect("mouse is matched as Pointer", device_count(pointer) == 1);
	expect("mouse is not matched as Keyboard", device_count(kbd) == 0);

	IOHIDDeviceRef dev = first_device(mouse);
	if (!dev)
		return;
	expect("Product", string_prop(dev, CFSTR(kIOHIDProductKey), s, sizeof s) && !strcmp(s, "Vpad Mouse"));
	expect("VendorID", num_prop(dev, CFSTR(kIOHIDVendorIDKey), &v) && v == 0x046d);
	expect("PrimaryUsage is Mouse", num_prop(dev, CFSTR(kIOHIDPrimaryUsageKey), &v) && v == kHIDUsage_GD_Mouse);
	expect("ConformsTo Mouse", IOHIDDeviceConformsTo(dev, kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse));
	expect("MaxOutputReportSize is 0", num_prop(dev, CFSTR(kIOHIDMaxOutputReportSizeKey), &v) && v == 0);

	CFArrayRef elems = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	expect("elements", elems && CFArrayGetCount(elems) > 0);
	if (!elems)
		return;
	IOHIDElementRef left = nth_element(elems, kHIDPage_Button, 1, 0);
	IOHIDElementRef right = nth_element(elems, kHIDPage_Button, 2, 0);
	IOHIDElementRef x = nth_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_X, 0);
	IOHIDElementRef y = nth_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_Y, 0);
	IOHIDElementRef wheel = nth_element(elems, kHIDPage_GenericDesktop, kHIDUsage_GD_Wheel, 0);
	IOHIDElementRef pan = nth_element(elems, kHIDPage_Consumer, kHIDUsage_Csmr_ACPan, 0);
	expect("three button elements", count_page(elems, kHIDPage_Button, kIOHIDElementTypeInput_Button) == 3);
	expect("left button", left && IOHIDElementGetType(left) == kIOHIDElementTypeInput_Button && IOHIDElementGetLogicalMax(left) == 1);
	expect("right button", right != NULL);
	expect("X: 16-bit relative", x && IOHIDElementGetType(x) == kIOHIDElementTypeInput_Misc && IOHIDElementIsRelative(x)
		&& IOHIDElementGetLogicalMin(x) == -32768 && IOHIDElementGetLogicalMax(x) == 32767 && IOHIDElementGetReportSize(x) == 16);
	expect("Y: 16-bit relative", y && IOHIDElementIsRelative(y) && IOHIDElementGetReportSize(y) == 16);
	expect("wheel: 8-bit relative", wheel && IOHIDElementIsRelative(wheel) && IOHIDElementGetLogicalMin(wheel) == -127
		&& IOHIDElementGetLogicalMax(wheel) == 127 && IOHIDElementGetReportSize(wheel) == 8);
	expect("AC Pan (Consumer page): 8-bit relative", pan && IOHIDElementIsRelative(pan) && IOHIDElementGetReportSize(pan) == 8);
	IOHIDElementRef coll = x ? IOHIDElementGetParent(x) : NULL;
	expect("axes are in the Pointer collection of the Mouse", coll && IOHIDElementGetType(coll) == kIOHIDElementTypeCollection
		&& IOHIDElementGetUsage(coll) == kHIDUsage_GD_Pointer && IOHIDElementGetParent(coll)
		&& IOHIDElementGetUsage(IOHIDElementGetParent(coll)) == kHIDUsage_GD_Mouse);

	cbcount = 0;
	sync_point("left-down");
	expect("left button pressed", wait_cb(kHIDPage_Button, 1, 1));
	expect("left button pressed (GetValue)", current(dev, left) == 1);
	sync_point("left-up");
	expect("left button released", wait_cb(kHIDPage_Button, 1, 0));
	sync_point("right-down");
	expect("right button pressed", wait_cb(kHIDPage_Button, 2, 1));
	sync_point("right-up");

	cbcount = 0;
	sync_point("move");                                           /* X +5, Y -3 in one frame */
	expect("X motion +5", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, 5));
	expect("Y motion -3", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Y, -3));
	expect("X value is the last motion", current(dev, x) == 5);
	int before = cbcount;
	sync_point("move-again");                                     /* the same motion again is a new event */
	spin(0.5);
	expect("the same relative motion is reported again", cbcount > before);
	sync_point("move-x-big");                                     /* X -300: beyond 8 bits */
	expect("X motion -300", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, -300));
	sync_point("wheel");                                          /* wheel -1 */
	expect("wheel -1", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_Wheel, -1));
	sync_point("hwheel");                                         /* horizontal wheel +2 */
	expect("AC Pan +2", wait_cb(kHIDPage_Consumer, kHIDUsage_Csmr_ACPan, 2));
	sync_point("two-frames");                                     /* two frames at once: X +1 then X +2 */
	expect("both frames reported (X +1)", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, 1));
	expect("both frames reported (X +2)", wait_cb(kHIDPage_GenericDesktop, kHIDUsage_GD_X, 2));

	CFRelease(elems);
	sync_point("unplug");
	for (int i = 0; i < 200 && device_count(mouse) != 0; i++)
		spin(0.05);
	expect("mouse removed", device_count(mouse) == 0 && device_count(pointer) == 0);
	IOHIDManagerClose(mouse, kIOHIDOptionsTypeNone);
	IOHIDManagerClose(pointer, kIOHIDOptionsTypeNone);
	IOHIDManagerClose(kbd, kIOHIDOptionsTypeNone);
}

/* The last effect the host saw: the values after "ff-upload" in the last line of that kind */
static int last_upload(unsigned *strong, unsigned *weak, unsigned *length, unsigned *delay)
{
	const char *p = logtext, *found = NULL;

	while ((p = strstr(p, "ff-upload ")) != NULL)
	{
		found = p;
		p += 10;
	}
	return found && sscanf(found, "ff-upload strong=%u weak=%u length=%u delay=%u", strong, weak, length, delay) == 4;
}

static int last_play_value(void)
{
	const char *p = logtext, *found = NULL;
	unsigned id; int value;

	while ((p = strstr(p, "ff-play ")) != NULL)
	{
		found = p;
		p += 8;
	}
	return found && sscanf(found, "ff-play %u %d", &id, &value) == 2 ? value : -1;
}

/* kind: 0 DualShock 4 on USB (report 5), 1 Xbox One (report 3), 2 DualShock 4 on Bluetooth (report 17) */
static void test_rumble(int kind)
{
	IOHIDManagerRef mgr = make_manager(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
	unsigned strong, weak, length, delay;
	const int xbox = kind == 1;
	const uint32_t id = kind == 1 ? 3 : kind == 2 ? 0x11 : 5;
	const size_t size = kind == 1 ? 8 : kind == 2 ? 77 : 31;
	const int shared = kind == 2 ? 2 : 0;      /* the Bluetooth report has two bytes before the shared part */
	int v;

	IOHIDManagerOpen(mgr, kIOHIDOptionsTypeNone);
	sync_point("plug");
	for (int i = 0; i < 200 && device_count(mgr) == 0; i++)
		spin(0.05);
	spin(0.3);
	IOHIDDeviceRef dev = first_device(mgr);
	expect("controller with force feedback is matched", dev != NULL);
	if (!dev)
		return;
	expect("MaxOutputReportSize covers the report and its ID", num_prop(dev, CFSTR(kIOHIDMaxOutputReportSizeKey), &v) && v == (int) size + 1);

	CFArrayRef elems = IOHIDDeviceCopyMatchingElements(dev, NULL, kIOHIDOptionsTypeNone);
	expect("elements", elems && CFArrayGetCount(elems) > 0);
	if (!elems)
		return;
	IOHIDElementRef a = nth_element(elems, kHIDPage_Button, 1, 0);
	expect("input elements carry report ID 1", a && IOHIDElementGetReportID(a) == 1);

	/* Input report: the ID comes first */
	uint8_t report[96];
	CFIndex rlen = sizeof report;
	expect("GetReport Input 1", IOHIDDeviceGetReport(dev, kIOHIDReportTypeInput, 1, report, &rlen) == kIOReturnSuccess
		&& rlen > 1 && report[0] == 1);
	rlen = sizeof report;
	expect("GetReport Input 0 does not exist", IOHIDDeviceGetReport(dev, kIOHIDReportTypeInput, 0, report, &rlen) != kIOReturnSuccess);

	uint8_t out[96];
	memset(out, 0, sizeof out);
	out[0] = (uint8_t) id;
	if (xbox)
	{
		/* enable weak and strong, magnitudes 0 .. 100: strong 50 at index 2, weak 100 at index 3; duration 10 * 10 ms */
		out[1] = 0x03; out[4] = 50; out[5] = 100; out[6] = 10; out[8] = 1;
	}
	else
	{
		/* flags: rumble; weak 0x80, strong 0xff */
		out[1 + shared] = 0x01; out[4 + shared] = 0x80; out[5 + shared] = 0xff;
	}
	expect("SetReport Output rumble", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, id, out, size + 1) == kIOReturnSuccess);
	read_log();
	expect("host got an uploaded rumble effect", last_upload(&strong, &weak, &length, &delay));
	if (xbox)
		expect("strong 50% weak 100%, 100 ms", strong == 32767 && weak == 65535 && length == 100 && delay == 0);
	else
		expect("strong 0xffff weak 0x8080", strong == 0xffff && weak == 0x8080);
	expect("the effect is played", last_play_value() == 1);

	/* The report that stops it */
	memset(out, 0, sizeof out);
	out[0] = (uint8_t) id;
	out[xbox ? 1 : 1 + shared] = xbox ? 0x03 : 0x01;
	expect("SetReport Output with zero strengths", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, id, out, size + 1) == kIOReturnSuccess);
	read_log();
	expect("the effect is stopped", last_play_value() == 0);

	rlen = sizeof report;
	expect("GetReport Output is what was sent", IOHIDDeviceGetReport(dev, kIOHIDReportTypeOutput, id, report, &rlen) == kIOReturnSuccess
		&& rlen == (CFIndex) size + 1 && report[0] == id && report[xbox ? 1 : 1 + shared] == out[xbox ? 1 : 1 + shared]);

	/* Errors */
	expect("a longer report is refused", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, id, out, size + 2) != kIOReturnSuccess);
	expect("first byte must be the report ID", ({ out[0] = 0x09; IOReturn rr = IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, id, out, size + 1); out[0] = id; rr == kIOReturnBadArgument; }));
	expect("an unknown report ID", IOHIDDeviceSetReport(dev, kIOHIDReportTypeOutput, 9, out, size + 1) != kIOReturnSuccess);

	if (xbox)
	{
		/* The same through SetValue on the output elements of the report */
		IOHIDElementRef enable1 = nth_element(elems, kHIDPage_PID, 0x97, 1);
		IOHIDElementRef magnitude2 = nth_element(elems, kHIDPage_PID, 0x70, 2);
		IOHIDElementRef duration = nth_element(elems, kHIDPage_PID, 0x50, 0);
		int n_mag = 0;
		while (nth_element(elems, kHIDPage_PID, 0x70, n_mag))
			n_mag++;
		expect("four Magnitude output elements", n_mag == 4);
		expect("Magnitude element: type, range, report ID", magnitude2 && IOHIDElementGetType(magnitude2) == kIOHIDElementTypeOutput
			&& IOHIDElementGetLogicalMax(magnitude2) == 100 && IOHIDElementGetReportID(magnitude2) == 3);
		expect("Duration element: 0 .. 255", duration && IOHIDElementGetLogicalMax(duration) == 255);
		if (!enable1 || !magnitude2 || !duration)
			return;
		IOHIDValueRef value = IOHIDValueCreateWithIntegerValue(NULL, enable1, 0, 1);
		expect("SetValue: enable the second actuator", IOHIDDeviceSetValue(dev, enable1, value) == kIOReturnSuccess);
		CFRelease(value);
		value = IOHIDValueCreateWithIntegerValue(NULL, magnitude2, 0, 100);
		expect("SetValue: Magnitude 100 at index 2", IOHIDDeviceSetValue(dev, magnitude2, value) == kIOReturnSuccess);
		CFRelease(value);
		read_log();
		expect("host got strong 100%, weak 0", last_upload(&strong, &weak, &length, &delay) && strong == 65535 && weak == 0);
		expect("and plays it", last_play_value() == 1);
		rlen = sizeof report;
		expect("GetReport Output has the element values", IOHIDDeviceGetReport(dev, kIOHIDReportTypeOutput, 3, report, &rlen) == kIOReturnSuccess
			&& rlen == 9 && report[1] == 0x03 && report[4] == 100);
		expect("GetValue of an output element", current(dev, magnitude2) == 100);
	}
	else
	{
		IOHIDElementRef blob = nth_element(elems, 0xff00, 0x22, 0);
		expect("one vendor-defined output element covering the report", blob && IOHIDElementGetType(blob) == kIOHIDElementTypeOutput
			&& IOHIDElementGetReportID(blob) == id && IOHIDElementGetReportSize(blob) * IOHIDElementGetReportCount(blob) == size * 8);
		if (blob)
		{
			uint8_t bytes[77] = { 0 };
			bytes[0 + shared] = 0x01; bytes[3 + shared] = 0x40; bytes[4 + shared] = 0x20;
			IOHIDValueRef value = IOHIDValueCreateWithBytes(NULL, blob, 0, bytes, size);
			expect("SetValue with the bytes of the report", IOHIDDeviceSetValue(dev, blob, value) == kIOReturnSuccess);
			CFRelease(value);
			read_log();
			expect("host got weak 0x4040 strong 0x2020", last_upload(&strong, &weak, &length, &delay) && strong == 0x2020 && weak == 0x4040);
		}
	}

	CFRelease(elems);
	sync_point("unplug");
	for (int i = 0; i < 200 && device_count(mgr) != 0; i++)
		spin(0.05);
	expect("controller removed", device_count(mgr) == 0);
	IOHIDManagerClose(mgr, kIOHIDOptionsTypeNone);
}

int main(int argc, char **argv)
{
	if (argc < 3)
		return 2;
	ctl = argv[1];
	setvbuf(stdout, NULL, _IOLBF, 0);
	if (!strcmp(argv[2], "kbd"))
		test_keyboard();
	else if (!strcmp(argv[2], "mouse"))
		test_mouse();
	else if (!strcmp(argv[2], "xboxff"))
		test_rumble(1);
	else if (!strcmp(argv[2], "ds4ff"))
		test_rumble(2);
	else if (!strcmp(argv[2], "ds4usbff"))
		test_rumble(0);
	else
		return 2;
	printf("%s\n", failures ? "FAILED" : "OK");
	return failures != 0;
}
