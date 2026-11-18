// Copyright (C) 2026 Darling Developers
//
// kAudioFileGlobalInfo_TypesForExtension, _TypesForUTI and _TypesForMIMEType. AudioFile.h
// describes them as the inverses of ExtensionsForType, UTIsForType and MIMETypesForType, with a
// CFStringRef as specifier; like every specifier of AudioFileGetGlobalInfo, it is passed in a
// buffer: the address of the CFStringRef, with sizeof(CFStringRef) as specifier size.
//
// Usage: audiofile_types_for_strings
#include <AudioToolbox/AudioFile.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long a_ = (a), b_ = (b); if (a_ != b_) { printf("FAIL %s:%d: %s = %lld (0x%llx), expected %lld (0x%llx)\n", __FILE__, __LINE__, #a, a_, a_, b_, b_); g_failures++; } } while (0)

static const struct
{
	AudioFilePropertyID forType, typesFor;
	const char* name;
} g_tables[] = {
	{ kAudioFileGlobalInfo_ExtensionsForType, kAudioFileGlobalInfo_TypesForExtension, "extension" },
	{ kAudioFileGlobalInfo_UTIsForType, kAudioFileGlobalInfo_TypesForUTI, "UTI" },
	{ kAudioFileGlobalInfo_MIMETypesForType, kAudioFileGlobalInfo_TypesForMIMEType, "MIME type" },
};

// Returns how many file types the property gives for the string; sets *found if one is type
static UInt32 typesFor(AudioFilePropertyID property, CFStringRef string, AudioFileTypeID type, int* found)
{
	UInt32 size = 0;
	UInt32 count;
	AudioFileTypeID* types;

	*found = 0;
	CHECK_EQ(AudioFileGetGlobalInfoSize(property, sizeof(string), &string, &size), noErr);
	CHECK_EQ(size % sizeof(AudioFileTypeID), 0);
	count = size / sizeof(AudioFileTypeID);
	if (count == 0)
		return 0;

	types = (AudioFileTypeID*) malloc(size);
	CHECK_EQ(AudioFileGetGlobalInfo(property, sizeof(string), &string, &size, types), noErr);
	CHECK_EQ(size, count * sizeof(AudioFileTypeID));
	for (UInt32 i = 0; i < size / sizeof(AudioFileTypeID); i++)
		*found |= types[i] == type;
	free(types);
	return count;
}

static void test_well_known_strings(void)
{
	static const struct { AudioFilePropertyID property; const char* value; AudioFileTypeID type; } expected[] = {
		{ kAudioFileGlobalInfo_TypesForExtension, "wav", kAudioFileWAVEType },
		{ kAudioFileGlobalInfo_TypesForExtension, "aiff", kAudioFileAIFFType },
		{ kAudioFileGlobalInfo_TypesForExtension, "caf", kAudioFileCAFType },
		{ kAudioFileGlobalInfo_TypesForUTI, "com.microsoft.waveform-audio", kAudioFileWAVEType },
		{ kAudioFileGlobalInfo_TypesForMIMEType, "audio/vnd.wave", kAudioFileWAVEType },
	};

	for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
	{
		CFStringRef string = CFStringCreateWithCString(NULL, expected[i].value, kCFStringEncodingUTF8);
		int found;

		typesFor(expected[i].property, string, expected[i].type, &found);
		if (!found)
		{
			printf("FAIL %s:%d: '%s' does not give type 0x%08x\n", __FILE__, __LINE__, expected[i].value, (unsigned) expected[i].type);
			g_failures++;
		}
		CFRelease(string);
	}
}

// Strings made from UTF-16 characters. CFStringGetCStringPtr() is allowed to return NULL for any
// string, and does for one that is not ASCII: such a string is still a valid specifier.
static void test_unicode_storage(void)
{
	static const UniChar wav[] = { 'w', 'a', 'v' };
	static const UniChar notAscii[] = { 'w', 0x00E4, 'v' };
	CFStringRef string = CFStringCreateWithCharacters(NULL, wav, 3);
	int found;

	typesFor(kAudioFileGlobalInfo_TypesForExtension, string, kAudioFileWAVEType, &found);
	CHECK(found);
	CFRelease(string);

	string = CFStringCreateWithCharacters(NULL, notAscii, 3);
	CHECK_EQ(typesFor(kAudioFileGlobalInfo_TypesForExtension, string, kAudioFileWAVEType, &found), 0);
	CFRelease(string);
}

// Each value listed for a readable type maps back to that type
static void test_inverse_of_for_type(void)
{
	UInt32 size = 0;
	AudioFileTypeID* types;

	CHECK_EQ(AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size), noErr);
	CHECK(size >= sizeof(AudioFileTypeID));
	types = (AudioFileTypeID*) malloc(size);
	CHECK_EQ(AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ReadableTypes, 0, NULL, &size, types), noErr);

	for (UInt32 i = 0; i < size / sizeof(AudioFileTypeID); i++)
	{
		for (int t = 0; t < 3; t++)
		{
			AudioFileTypeID type = types[i];
			CFArrayRef array = NULL;
			UInt32 arraySize = sizeof(array);

			if (AudioFileGetGlobalInfo(g_tables[t].forType, sizeof(type), &type, &arraySize, &array) != noErr || !array)
				continue;
			for (CFIndex j = 0; j < CFArrayGetCount(array); j++)
			{
				CFStringRef value = (CFStringRef) CFArrayGetValueAtIndex(array, j);
				int found;

				typesFor(g_tables[t].typesFor, value, type, &found);
				if (!found)
				{
					char buf[128] = "";
					CFStringGetCString(value, buf, sizeof(buf), kCFStringEncodingUTF8);
					printf("FAIL %s:%d: %s '%s' does not map back to type 0x%08x\n", __FILE__, __LINE__, g_tables[t].name, buf, (unsigned) type);
					g_failures++;
				}
			}
			CFRelease(array);
		}
	}
	free(types);
}

static void test_bad_specifiers(void)
{
	CFStringRef string = CFSTR("wav");
	CFStringRef none = NULL;
	UInt32 size = 0;
	int found;

	// A string nobody handles maps to no file type
	CHECK_EQ(typesFor(kAudioFileGlobalInfo_TypesForExtension, CFSTR("zzzq"), kAudioFileWAVEType, &found), 0);

	// A specifier of the wrong size, a missing specifier and a NULL string are errors
	CHECK(AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_TypesForExtension, sizeof(string) - 1, &string, &size) != noErr);
	CHECK(AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_TypesForExtension, sizeof(string), NULL, &size) != noErr);
	CHECK(AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_TypesForUTI, sizeof(none), &none, &size) != noErr);
}

int main(void)
{
	printf("audiofile_types_for_strings: %d-bit process\n", (int) sizeof(void*) * 8);

	test_well_known_strings();
	test_unicode_storage();
	test_inverse_of_for_type();
	test_bad_specifiers();

	if (g_failures)
	{
		printf("audiofile_types_for_strings: %d failure(s)\n", g_failures);
		return 1;
	}

	printf("audiofile_types_for_strings: OK\n");
	return 0;
}
