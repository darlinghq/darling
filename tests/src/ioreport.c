/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Guest test: libIOReport answers channel queries with empty results and
 * creates no subscription. Link with /usr/lib/libIOReport.dylib.
 */
#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <stdio.h>

CFMutableDictionaryRef IOReportCopyAllChannels(uint64_t a, uint64_t b);
CFMutableDictionaryRef IOReportCopyChannelsInGroup(CFStringRef group, CFStringRef subgroup, uint64_t a, uint64_t b, uint64_t c);
int IOReportGetChannelCount(CFMutableDictionaryRef dict);
void *IOReportCreateSubscription(void *a, CFMutableDictionaryRef desired, CFMutableDictionaryRef *subbed, uint64_t channelId, CFTypeRef b);
int IOReportStateGetCount(CFDictionaryRef ch);
CFStringRef IOReportChannelGetGroup(CFDictionaryRef ch);

#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "FAIL: %s\n", #cond); failures++; } } while (0)

int main(void)
{
	int failures = 0;
	CFMutableDictionaryRef all = IOReportCopyAllChannels(0, 0);
	CHECK(all == NULL);
	CHECK(IOReportCopyChannelsInGroup(CFSTR("CPU Stats"), NULL, 0, 0, 0) == NULL);
	CHECK(IOReportGetChannelCount(all) == 0);

	CFMutableDictionaryRef subbed = (CFMutableDictionaryRef)0x1;
	CHECK(IOReportCreateSubscription(NULL, all, &subbed, 0, NULL) == NULL);
	CHECK(subbed == NULL);

	CHECK(IOReportStateGetCount(NULL) == 0);
	CFStringRef group = IOReportChannelGetGroup(NULL);
	CHECK(group != NULL && CFStringGetLength(group) == 0);

	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
