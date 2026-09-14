#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct IOReportSubscriptionRef* IOReportSubscriptionRef;
typedef CFDictionaryRef IOReportSampleRef;

CFMutableDictionaryRef IOReportCopyChannelsInGroup(CFStringRef group, CFStringRef subgroup, uint64_t a, uint64_t b, uint64_t c) {
	return NULL;
}

CFMutableDictionaryRef IOReportCopyAllChannels(uint64_t a, uint64_t b) {
	return NULL;
}

int IOReportGetChannelCount(CFMutableDictionaryRef dict) {
	return 0;
}

IOReportSubscriptionRef IOReportCreateSubscription(void* a, CFMutableDictionaryRef desiredChannels, CFMutableDictionaryRef* subbedChannels, uint64_t channel_id, CFTypeRef b) {
	if (subbedChannels) *subbedChannels = NULL;
	return NULL;
}

CFDictionaryRef IOReportCreateSamples(IOReportSubscriptionRef iorsub, CFMutableDictionaryRef subbedChannels, CFTypeRef a) {
	return NULL;
}

CFDictionaryRef IOReportCreateSamplesDelta(CFDictionaryRef prev, CFDictionaryRef current, CFTypeRef a) {
	return NULL;
}

int IOReportMergeChannels(CFMutableDictionaryRef a, CFDictionaryRef b, CFTypeRef c) {
	return 0;
}

typedef int (^ioreportiterateblock)(IOReportSampleRef ch);

void IOReportIterate(CFDictionaryRef samples, ioreportiterateblock block) {
}

int IOReportChannelGetFormat(CFDictionaryRef samples) {
	return 0;
}

long IOReportSimpleGetIntegerValue(CFDictionaryRef ch, int a) {
	return 0;
}

CFStringRef IOReportChannelGetDriverName(CFDictionaryRef ch) {
	return CFSTR("");
}

CFStringRef IOReportChannelGetChannelName(CFDictionaryRef ch) {
	return CFSTR("");
}

CFStringRef IOReportChannelGetGroup(CFDictionaryRef ch) {
	return CFSTR("");
}

CFStringRef IOReportChannelGetSubGroup(CFDictionaryRef ch) {
	return CFSTR("");
}

CFStringRef IOReportChannelGetUnitLabel(CFDictionaryRef ch) {
	return CFSTR("");
}

int IOReportStateGetCount(CFDictionaryRef ch) {
	return 0;
}

uint64_t IOReportStateGetResidency(CFDictionaryRef ch, int index) {
	return 0;
}

CFStringRef IOReportStateGetNameForIndex(CFDictionaryRef ch, int index) {
	return CFSTR("");
}

int IOReportHistogramGetBucketCount(CFDictionaryRef ch) {
	return 0;
}

CFStringRef IOReportHistogramGetBucketName(CFDictionaryRef ch, int index) {
	return CFSTR("");
}

int IOReportHistogramGetBucketBounds(CFDictionaryRef ch, int index, uint64_t* lower, uint64_t* upper) {
	return 0;
}

uint64_t IOReportHistogramGetBucketValue(CFDictionaryRef ch, int index) {
	return 0;
}

uint64_t IOReportHistogramGetBucketHits(CFDictionaryRef ch, int index) {
	return 0;
}

long IOReportArrayGetValueAtIndex(CFDictionaryRef ch, int index) {
	return 0;
}

int IOReportArrayGetCount(CFDictionaryRef ch) {
	return 0;
}
