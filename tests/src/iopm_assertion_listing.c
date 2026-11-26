// Copyright (C) 2026 Lubos Dolezel
//
// The listing calls of IOPMLib for the power assertions of the calling process:
// IOPMCopyAssertionsByProcess() and IOPMCopyAssertionsStatus(), and the change notification of
// IOPMAssertionNotify(). The program passes unchanged on macOS 26.7.3 and 10.14.4, where the
// expectations were observed (black-box): only active (level on) assertions are listed, a legacy
// type name such as kIOPMAssertionTypeNoDisplaySleep is listed as given with the current name of the
// type under "AssertionTrueType", and the status value of a type is 1 while any process holds it, else 0.
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <notify.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Not in the public header: the registration for the change notification (IOPMLibPrivate.h).
extern IOReturn IOPMAssertionNotify(char *name, int req_type);
#define kNotifyRegister 1
#define kAnyChangedNotifyString "com.apple.system.powermanagement.assertions.anychange"

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static CFStringRef const kName = CFSTR("iopm_assertion_listing");

// The dictionary of this process' assertion with the id, or NULL. The caller releases the array it is in
// through *list.
static CFDictionaryRef find_mine(IOPMAssertionID id, CFDictionaryRef* byProcess)
{
	int pid = getpid();
	CFNumberRef key = CFNumberCreate(NULL, kCFNumberIntType, &pid);
	CFArrayRef mine;
	CFDictionaryRef found = NULL;

	*byProcess = NULL;
	if (IOPMCopyAssertionsByProcess(byProcess) != kIOReturnSuccess || !*byProcess)
	{
		CFRelease(key);
		return NULL;
	}
	mine = CFDictionaryGetValue(*byProcess, key);
	CFRelease(key);
	for (CFIndex i = 0; mine && i < CFArrayGetCount(mine); i++)
	{
		CFDictionaryRef entry = CFArrayGetValueAtIndex(mine, i);
		CFNumberRef number = CFDictionaryGetValue(entry, CFSTR("AssertionId"));
		int value = 0;

		if (number && CFNumberGetValue(number, kCFNumberIntType, &value) && (IOPMAssertionID) value == id)
			found = entry;
	}
	return found;
}

static int status_of(CFStringRef type)
{
	CFDictionaryRef status = NULL;
	int value = -1;

	if (IOPMCopyAssertionsStatus(&status) == kIOReturnSuccess && status)
	{
		CFNumberRef number = CFDictionaryGetValue(status, type);

		if (number)
			CFNumberGetValue(number, kCFNumberIntType, &value);
		CFRelease(status);
	}
	return value;
}

static int level_of(CFDictionaryRef entry)
{
	CFNumberRef number = CFDictionaryGetValue(entry, kIOPMAssertionLevelKey);
	int value = -1;

	if (number)
		CFNumberGetValue(number, kCFNumberIntType, &value);
	return value;
}

static void check_listing(void)
{
	IOPMAssertionID id = kIOPMNullAssertionID, legacy = kIOPMNullAssertionID;
	CFDictionaryRef byProcess, entry;

	CHECK(status_of(kIOPMAssertPreventUserIdleDisplaySleep) == 0);
	CHECK(IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep, kIOPMAssertionLevelOn, kName, &id) ==
	      kIOReturnSuccess);

	entry = find_mine(id, &byProcess);
	CHECK(entry != NULL);
	if (entry)
	{
		CHECK(CFEqual(CFDictionaryGetValue(entry, kIOPMAssertionTypeKey), kIOPMAssertPreventUserIdleDisplaySleep));
		CHECK(CFEqual(CFDictionaryGetValue(entry, kIOPMAssertionNameKey), kName));
		CHECK(level_of(entry) == kIOPMAssertionLevelOn);
	}
	if (byProcess)
		CFRelease(byProcess);
	CHECK(status_of(kIOPMAssertPreventUserIdleDisplaySleep) == 1);

	// A legacy name of the same type.
	CHECK(IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, kName, &legacy) ==
	      kIOReturnSuccess);
	entry = find_mine(legacy, &byProcess);
	CHECK(entry != NULL);
	if (entry)
	{
		CHECK(CFEqual(CFDictionaryGetValue(entry, kIOPMAssertionTypeKey), kIOPMAssertionTypeNoDisplaySleep));
		CHECK(CFEqual(CFDictionaryGetValue(entry, CFSTR("AssertionTrueType")), kIOPMAssertPreventUserIdleDisplaySleep));
	}
	if (byProcess)
		CFRelease(byProcess);

	// The type stays held until the last assertion of it goes.
	CHECK(IOPMAssertionRelease(id) == kIOReturnSuccess);
	entry = find_mine(id, &byProcess);
	CHECK(entry == NULL);
	if (byProcess)
		CFRelease(byProcess);
	CHECK(status_of(kIOPMAssertPreventUserIdleDisplaySleep) == 1);

	CHECK(IOPMAssertionRelease(legacy) == kIOReturnSuccess);
	entry = find_mine(legacy, &byProcess);
	CHECK(entry == NULL);
	if (byProcess)
		CFRelease(byProcess);
	CHECK(status_of(kIOPMAssertPreventUserIdleDisplaySleep) == 0);
}

// An assertion that is turned off is not listed, and is again when turned on.
static void check_level_off(void)
{
	IOPMAssertionID id = kIOPMNullAssertionID;
	CFDictionaryRef byProcess;
	int off = kIOPMAssertionLevelOff;
	CFNumberRef offNumber = CFNumberCreate(NULL, kCFNumberIntType, &off);

	CHECK(IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep, kIOPMAssertionLevelOn, kName, &id) ==
	      kIOReturnSuccess);
	CHECK(IOPMAssertionSetProperty(id, kIOPMAssertionLevelKey, offNumber) == kIOReturnSuccess);
	CHECK(find_mine(id, &byProcess) == NULL);
	if (byProcess)
		CFRelease(byProcess);
	CHECK(status_of(kIOPMAssertPreventUserIdleDisplaySleep) == 0);

	CHECK(IOPMAssertionSetProperty(id, kIOPMAssertionLevelKey, CFNumberCreate(NULL, kCFNumberIntType, &(int){ kIOPMAssertionLevelOn })) ==
	      kIOReturnSuccess);
	CHECK(find_mine(id, &byProcess) != NULL);
	if (byProcess)
		CFRelease(byProcess);
	CHECK(status_of(kIOPMAssertPreventUserIdleDisplaySleep) == 1);
	IOPMAssertionRelease(id);
	CFRelease(offNumber);
}

// A client that registered is notified of a change of an assertion.
static void check_notification(void)
{
	int token = 0, changed = 0;
	IOPMAssertionID id = kIOPMNullAssertionID;

	CHECK(IOPMAssertionNotify((char*) kAnyChangedNotifyString, kNotifyRegister) == kIOReturnSuccess);
	CHECK(notify_register_check(kAnyChangedNotifyString, &token) == NOTIFY_STATUS_OK);
	notify_check(token, &changed);	// the first check reports the registration
	CHECK(IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep, kIOPMAssertionLevelOn, kName, &id) ==
	      kIOReturnSuccess);
	changed = 0;
	for (int i = 0; i < 40 && !changed; i++)
	{
		notify_check(token, &changed);
		if (!changed)
			usleep(50000);
	}
	CHECK(changed);
	IOPMAssertionRelease(id);
	notify_cancel(token);
}

// The type of a bad argument.
static void check_bad_arguments(void)
{
	CHECK(IOPMCopyAssertionsByProcess(NULL) == kIOReturnBadArgument);
	CHECK(IOPMCopyAssertionsStatus(NULL) == kIOReturnBadArgument);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);
	check_listing();
	check_level_off();
	check_notification();
	check_bad_arguments();
	printf(g_failures ? "FAIL (%d)\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
