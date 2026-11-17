// Copyright (C) 2026 Darling Developers
//
// The AudioFile global properties that list, for a file type, its file extensions, Uniform Type
// Identifiers and MIME types. AudioFile.h documents ExtensionsForType, UTIsForType and
// MIMETypesForType as returning a CFArray of CFStrings for the AudioFileTypeID given as specifier,
// and AllExtensions, AllUTIs and AllMIMETypes as returning every recognized value. The same
// answers are expected from 32-bit and 64-bit processes.
//
// Usage: audiofile_type_tables
#include <AudioToolbox/AudioFile.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long a_ = (a), b_ = (b); if (a_ != b_) { printf("FAIL %s:%d: %s = %lld (0x%llx), expected %lld (0x%llx)\n", __FILE__, __LINE__, #a, a_, a_, b_, b_); g_failures++; } } while (0)

// The per-type property and the global property listing the same kind of value
static const struct
{
	AudioFilePropertyID forType, all;
	const char* name;
} g_tables[] = {
	{ kAudioFileGlobalInfo_ExtensionsForType, kAudioFileGlobalInfo_AllExtensions, "extension" },
	{ kAudioFileGlobalInfo_UTIsForType, kAudioFileGlobalInfo_AllUTIs, "UTI" },
	{ kAudioFileGlobalInfo_MIMETypesForType, kAudioFileGlobalInfo_AllMIMETypes, "MIME type" },
};

static CFArrayRef stringsForType(AudioFilePropertyID property, AudioFileTypeID type, OSStatus* status)
{
	CFArrayRef array = NULL;
	UInt32 size = 0;

	CHECK_EQ(AudioFileGetGlobalInfoSize(property, sizeof(type), &type, &size), noErr);
	CHECK_EQ(size, sizeof(CFArrayRef));
	size = sizeof(array);
	*status = AudioFileGetGlobalInfo(property, sizeof(type), &type, &size, &array);
	if (*status == noErr)
		CHECK_EQ(size, sizeof(CFArrayRef));
	return array;
}

static CFArrayRef allStrings(AudioFilePropertyID property)
{
	CFArrayRef array = NULL;
	UInt32 size = 0;

	CHECK_EQ(AudioFileGetGlobalInfoSize(property, 0, NULL, &size), noErr);
	CHECK_EQ(size, sizeof(CFArrayRef));
	size = sizeof(array);
	CHECK_EQ(AudioFileGetGlobalInfo(property, 0, NULL, &size, &array), noErr);
	CHECK(array != NULL);
	return array;
}

static int arrayContains(CFArrayRef array, CFStringRef value)
{
	return array && CFArrayContainsValue(array, CFRangeMake(0, CFArrayGetCount(array)), (const void*) value);
}

static void test_well_known_types(void)
{
	// The usual extensions of each container, the UTI Apple declares for WAVE audio and the MIME
	// type registered for it by RFC 2361
	static const struct { AudioFileTypeID type; AudioFilePropertyID property; const char* value; } expected[] = {
		{ kAudioFileWAVEType, kAudioFileGlobalInfo_ExtensionsForType, "wav" },
		{ kAudioFileAIFFType, kAudioFileGlobalInfo_ExtensionsForType, "aiff" },
		{ kAudioFileAIFFType, kAudioFileGlobalInfo_ExtensionsForType, "aif" },
		{ kAudioFileAIFCType, kAudioFileGlobalInfo_ExtensionsForType, "aifc" },
		{ kAudioFileCAFType, kAudioFileGlobalInfo_ExtensionsForType, "caf" },
		{ kAudioFileWAVEType, kAudioFileGlobalInfo_UTIsForType, "com.microsoft.waveform-audio" },
		{ kAudioFileWAVEType, kAudioFileGlobalInfo_MIMETypesForType, "audio/vnd.wave" },
	};

	for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
	{
		OSStatus status;
		CFArrayRef array = stringsForType(expected[i].property, expected[i].type, &status);
		CFStringRef value = CFStringCreateWithCString(NULL, expected[i].value, kCFStringEncodingUTF8);

		CHECK_EQ(status, noErr);
		if (!arrayContains(array, value))
		{
			printf("FAIL %s:%d: '%s' missing for type 0x%08x\n", __FILE__, __LINE__, expected[i].value, (unsigned) expected[i].type);
			g_failures++;
		}
		CFRelease(value);
		if (array)
			CFRelease(array);
	}
}

// Every readable type answers each per-type property, and what it lists is in the global list
static void test_tables_are_consistent(void)
{
	UInt32 size = 0;
	AudioFileTypeID* types;
	CFArrayRef all[3];

	CHECK_EQ(AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size), noErr);
	CHECK(size >= sizeof(AudioFileTypeID));
	types = (AudioFileTypeID*) malloc(size);
	CHECK_EQ(AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size, types), noErr);

	for (int t = 0; t < 3; t++)
	{
		all[t] = allStrings(g_tables[t].all);
		CHECK(all[t] && CFArrayGetCount(all[t]) > 0);
	}

	for (UInt32 i = 0; i < size / sizeof(AudioFileTypeID); i++)
	{
		for (int t = 0; t < 3; t++)
		{
			OSStatus status;
			CFArrayRef array = stringsForType(g_tables[t].forType, types[i], &status);

			CHECK_EQ(status, noErr);
			if (!array)
				continue;
			for (CFIndex j = 0; j < CFArrayGetCount(array); j++)
			{
				CFStringRef value = (CFStringRef) CFArrayGetValueAtIndex(array, j);

				if (!arrayContains(all[t], value))
				{
					char buf[128] = "";
					CFStringGetCString(value, buf, sizeof(buf), kCFStringEncodingUTF8);
					printf("FAIL %s:%d: %s '%s' of type 0x%08x is not in the global list\n", __FILE__, __LINE__, g_tables[t].name, buf, (unsigned) types[i]);
					g_failures++;
				}
			}
			CFRelease(array);
		}
	}

	for (int t = 0; t < 3; t++)
		if (all[t])
			CFRelease(all[t]);
	free(types);
}

static void test_bad_specifiers(void)
{
	AudioFileTypeID unknown = 'zzzz';
	UInt16 shortType = 0;
	CFArrayRef array = NULL;
	UInt32 size = sizeof(array);

	// A file type nobody handles, and a specifier of the wrong size, are errors
	CHECK(AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof(unknown), &unknown, &size, &array) != noErr);
	if (array)
		CFRelease(array);
	array = NULL;
	size = sizeof(array);
	CHECK(AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType, sizeof(shortType), &shortType, &size, &array) != noErr);
	if (array)
		CFRelease(array);
}

int main(void)
{
	printf("audiofile_type_tables: %d-bit process\n", (int) sizeof(void*) * 8);

	test_well_known_types();
	test_tables_are_consistent();
	test_bad_specifiers();

	if (g_failures)
	{
		printf("audiofile_type_tables: %d failure(s)\n", g_failures);
		return 1;
	}

	printf("audiofile_type_tables: OK\n");
	return 0;
}
