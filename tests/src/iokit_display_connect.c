/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * The IODisplayConnect services of the IORegistry. Every one has the product name (a dictionary
 * of localised names), the vendor, product and serial numbers, and an EDID whose fixed header
 * and checksum are valid. A physical size, when given, is a pair of positive numbers.
 * With the argument "require" (for Darling with a display, from DRM or from an X server) there
 * is at least one such service, and no two services have the same serial number and product ID.
 * Without it nothing has to be there (a headless host).
 */
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failed;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failed = 1; } } while (0)

static CFTypeRef property(io_registry_entry_t e, const char *key)
{
	CFStringRef k = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
	CFTypeRef v = IORegistryEntryCreateCFProperty(e, k, NULL, 0);
	CFRelease(k);
	return v;
}

static int64_t number(io_registry_entry_t e, const char *key, int *ok)
{
	CFTypeRef v = property(e, key);
	int64_t result = 0;
	*ok = v && CFGetTypeID(v) == CFNumberGetTypeID() && CFNumberGetValue(v, kCFNumberSInt64Type, &result);
	if (v)
		CFRelease(v);
	return result;
}

int main(int argc, char **argv)
{
	int require = argc > 1 && strcmp(argv[1], "require") == 0;
	int displays = 0;
	int64_t seen[16][2];
	io_iterator_t it;
	io_registry_entry_t e;

	CHECK(IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IODisplayConnect"), &it) == KERN_SUCCESS);
	while ((e = IOIteratorNext(it))) {
		int ok;
		CFTypeRef name = property(e, "DisplayProductName");
		CHECK(name && CFGetTypeID(name) == CFDictionaryGetTypeID() && CFDictionaryGetCount(name) > 0);
		if (name)
			CFRelease(name);

		int64_t vendor = number(e, "DisplayVendorID", &ok);
		CHECK(ok);
		int64_t product = number(e, "DisplayProductID", &ok);
		CHECK(ok);
		int64_t serial = number(e, "DisplaySerialNumber", &ok);
		CHECK(ok);
		(void) vendor;

		CFTypeRef edid = property(e, "IODisplayEDID");
		CHECK(edid && CFGetTypeID(edid) == CFDataGetTypeID() && CFDataGetLength(edid) >= 128);
		if (edid && CFDataGetLength(edid) >= 128) {
			const uint8_t *b = CFDataGetBytePtr(edid);
			static const uint8_t header[8] = { 0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0 };
			uint8_t sum = 0;
			for (int i = 0; i < 128; i++)
				sum += b[i];
			CHECK(memcmp(b, header, 8) == 0);
			CHECK(sum == 0);
		}
		if (edid)
			CFRelease(edid);

		int okw, okh;
		int64_t w = number(e, "DisplayHorizontalImageSize", &okw);
		int64_t h = number(e, "DisplayVerticalImageSize", &okh);
		CHECK(okw == okh);
		if (okw && okh)
			CHECK(w > 0 && h > 0);

		printf("IODisplayConnect: product=%lld serial=%lld\n", (long long) product, (long long) serial);
		if (displays < 16) {
			seen[displays][0] = product;
			seen[displays][1] = serial;
		}
		displays++;
		IOObjectRelease(e);
	}
	IOObjectRelease(it);

	for (int i = 0; i < displays && i < 16; i++)
		for (int j = i + 1; j < displays && j < 16; j++)
			CHECK(seen[i][0] != seen[j][0] || seen[i][1] != seen[j][1]);

	if (require)
		CHECK(displays >= 1);

	printf("%s\n", failed ? "FAIL" : "PASS");
	return failed;
}
