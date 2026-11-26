// Copyright (C) 2026 Lubos Dolezel
//
// libAccessibility exports kAXSEnhanceTextLegibilityChangedNotification, the
// name of the notification posted when the Bold Text (enhanced legibility)
// setting changes; WebKit observes it on the local notification center.
// Checked: the constant is a non-empty CFString, and an observer registered
// for it on the local center gets a notification posted under that name.
// Its string, observed on macOS, is checked by private_constant_values.m.
// build: guest-cc.sh -o t accessibility_text_legibility_notification.c -framework CoreFoundation
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL: %s\n", what); failures++; } else printf("PASS: %s\n", what); } while (0)

static int received;

static void observer(CFNotificationCenterRef center, void *context, CFNotificationName name, const void *object, CFDictionaryRef userInfo)
{
	(void)center; (void)context; (void)name; (void)object; (void)userInfo;
	received++;
}

int main(void)
{
	void *handle = dlopen("/usr/lib/libAccessibility.dylib", RTLD_NOW);
	CHECK(handle != NULL, "libAccessibility loads");
	if (!handle)
		return 1;
	const CFStringRef *symbol = dlsym(handle, "kAXSEnhanceTextLegibilityChangedNotification");
	CHECK(symbol != NULL, "the notification name is exported");
	if (!symbol)
		return 1;
	CFStringRef name = *symbol;
	CHECK(name != NULL && CFGetTypeID(name) == CFStringGetTypeID() && CFStringGetLength(name) > 0,
		"the notification name is a non-empty CFString");

	CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
	CFNotificationCenterAddObserver(center, &received, observer, name, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
	CFNotificationCenterPostNotification(center, name, NULL, NULL, true);
	CHECK(received == 1, "an observer of the name gets a notification posted under it");
	CFNotificationCenterRemoveObserver(center, &received, name, NULL);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
