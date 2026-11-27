// Copyright (C) 2026 Lubos Dolezel
//
// The BitsPerComponent and AmbientViewingEnvironment format description
// extension keys are exported and declared, with the string values the system
// uses (read from the exported constants on macOS 26.7.3). The keys are usable
// as extensions of a video format description.
#include <CoreMedia/CoreMedia.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	CHECK(kCMFormatDescriptionExtension_BitsPerComponent != NULL);
	CHECK(kCMFormatDescriptionExtension_AmbientViewingEnvironment != NULL);
	CHECK(CFEqual(kCMFormatDescriptionExtension_BitsPerComponent, CFSTR("BitsPerComponent")));
	CHECK(CFEqual(kCMFormatDescriptionExtension_AmbientViewingEnvironment, CFSTR("AmbientViewingEnvironment")));

	int32_t bits = 10;
	CFNumberRef bitsNumber = CFNumberCreate(NULL, kCFNumberSInt32Type, &bits);
	const void* keys[1] = { kCMFormatDescriptionExtension_BitsPerComponent };
	const void* values[1] = { bitsNumber };
	CFDictionaryRef extensions = CFDictionaryCreate(NULL, keys, values, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CMVideoFormatDescriptionRef desc = NULL;
	OSStatus status = CMVideoFormatDescriptionCreate(NULL, kCMVideoCodecType_HEVC, 64, 48, extensions, &desc);
	CHECK(status == noErr && desc != NULL);
	if (desc) {
		CFTypeRef value = CMFormatDescriptionGetExtension(desc, kCMFormatDescriptionExtension_BitsPerComponent);
		int32_t read = 0;
		CHECK(value != NULL && CFGetTypeID(value) == CFNumberGetTypeID());
		CHECK(value != NULL && CFNumberGetValue((CFNumberRef) value, kCFNumberSInt32Type, &read) && read == 10);
		CHECK(CMFormatDescriptionGetExtension(desc, kCMFormatDescriptionExtension_AmbientViewingEnvironment) == NULL);
		CFRelease(desc);
	}
	CFRelease(extensions);
	CFRelease(bitsNumber);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
