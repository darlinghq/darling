/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ICCBuilder.h"
#include "ICCProfile.h"

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

struct __ColorSyncProfile
{
	CFRuntimeBase base;
	CFDataRef data;
	CFURLRef url;
	bool isMutable;
};

static void ColorSyncProfileFinalize(CFTypeRef cf)
{
	struct __ColorSyncProfile* profile = (struct __ColorSyncProfile*) cf;
	CFRelease(profile->data);
	if (profile->url)
		CFRelease(profile->url);
}

static const CFRuntimeClass ColorSyncProfileClass = {
	.version = 0,
	.className = "ColorSyncProfile",
	.finalize = ColorSyncProfileFinalize,
};

CFTypeID ColorSyncProfileGetTypeID(void)
{
	static CFTypeID typeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{ typeID = _CFRuntimeRegisterClass(&ColorSyncProfileClass); });
	return typeID;
}

static const struct __ColorSyncProfile* profileOrNull(ColorSyncProfileRef prof)
{
	if (!prof || CFGetTypeID(prof) != ColorSyncProfileGetTypeID())
		return NULL;
	return (const struct __ColorSyncProfile*) prof;
}

// GUESS: the page says only that an error describes the failure. The domain and code are not documented.
// Also used for allocation failure, which the page does not mention.
static void setInvalidDataError(CFErrorRef* error)
{
	if (error)
		*error = CFErrorCreate(kCFAllocatorDefault, kCFErrorDomainOSStatus, -50 /* paramErr */, NULL);
}

// GUESS: the pages do not say what Create does with a NULL data argument; it fails like invalid data.
ColorSyncProfileRef ColorSyncProfileCreate(CFDataRef data, CFErrorRef* error)
{
	if (error)
		*error = NULL;

	if (!data || !ICCProfileValidate(CFDataGetBytePtr(data), (size_t) CFDataGetLength(data)))
	{
		setInvalidDataError(error);
		return NULL;
	}

	// An immutable copy, so the profile does not change if the caller's data is mutable and changes.
	CFDataRef copy = CFDataCreateCopy(kCFAllocatorDefault, data);
	struct __ColorSyncProfile* profile = copy ? (struct __ColorSyncProfile*) _CFRuntimeCreateInstance(
		kCFAllocatorDefault, ColorSyncProfileGetTypeID(), sizeof(*profile) - sizeof(CFRuntimeBase), NULL) : NULL;
	if (!profile)
	{
		if (copy)
			CFRelease(copy);
		setInvalidDataError(error);
		return NULL;
	}

	profile->data = copy;
	return (ColorSyncProfileRef) profile;
}

// The page says only that a predefined name makes a profile and NULL means failure. The names are the kColorSync...Profile
// constants. The pages give no profile data, so the profile is built from the public definition of the colour space (see
// ICCBuilder.h): its colorants, curve and description text are not those of any other implementation. Only sRGB, Adobe RGB
// (1998), ITU-R BT.2020 and the generic gray gamma 2.2 have such a definition. The generic Lab, XYZ and CMYK names, which
// macOS has profiles for, give NULL here, like any other string, a NULL name and a non-string: the Lab and XYZ profiles have
// no public definition as profile data, and the CMYK one needs a measured device table.
//
// QUIRK: each name gives the same profile object on every call (the caller owns a reference to it, as with other Create
// functions). Evidence: black-box observation on macOS 26.7.3 (x86_64) and 10.14.4 (x86_64 and i386), tier 2: two calls with
// one name return the same pointer. Test: tests/src/colorsync_profile_name.c, "the same object".
ColorSyncProfileRef ColorSyncProfileCreateWithName(CFStringRef name)
{
	static const struct { CFStringRef* name; ICCNamedProfile which; } names[] = {
		{ &kColorSyncSRGBProfile, ICCNamedSRGB },
		{ &kColorSyncAdobeRGB1998Profile, ICCNamedAdobeRGB1998 },
		{ &kColorSyncITUR2020Profile, ICCNamedITUR2020 },
		{ &kColorSyncGenericGrayGamma22Profile, ICCNamedGrayGamma22 },
	};
	static ColorSyncProfileRef cache[sizeof names / sizeof names[0]];
	static dispatch_once_t once[sizeof names / sizeof names[0]];
	if (!name || CFGetTypeID(name) != CFStringGetTypeID())
		return NULL;

	for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
	{
		if (!CFEqual(name, *names[i].name))
			continue;
		dispatch_once(&once[i], ^{
			const ICCDisplayProfileSpec spec = ICCNamedProfileSpec(names[i].which);
			size_t length;
			uint8_t* bytes = ICCBuildDisplayProfile(&spec, &length);
			if (!bytes)
				return;
			CFDataRef data = CFDataCreate(kCFAllocatorDefault, bytes, (CFIndex) length);
			free(bytes);
			if (!data)
				return;
			cache[i] = ColorSyncProfileCreate(data, NULL);
			CFRelease(data);
		});
		return cache[i] ? (ColorSyncProfileRef) CFRetain(cache[i]) : NULL;
	}
	return NULL;
}

// GUESS: a NULL or non-profile argument fails with NULL and the same error as invalid data; the page does not say.
CFDataRef ColorSyncProfileCopyData(ColorSyncProfileRef prof, CFErrorRef* error)
{
	if (error)
		*error = NULL;

	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	if (!profile)
	{
		setInvalidDataError(error);
		return NULL;
	}
	return CFRetain(profile->data);
}

// The page says the digest is "calculated as defined by the ICC specification" (ICC.1:2022, 7.2.18): MD5 over the profile with
// the flags, rendering intent and Profile ID header fields zero. The digest is always computed: a Profile ID already stored in
// the header is not returned. It covers every byte held (see the QUIRK on ICCProfileComputeID for the size field). A NULL or
// non-profile argument gives the zero digest, which the page defines as the failure result (macOS 26.7.3 and 10.14.4, observed).
ColorSyncMD5 ColorSyncProfileGetMD5(ColorSyncProfileRef prof)
{
	ColorSyncMD5 md5;
	memset(&md5, 0, sizeof md5);
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	if (profile && !ICCProfileComputeID(CFDataGetBytePtr(profile->data), (size_t) CFDataGetLength(profile->data), md5.digest))
		memset(&md5, 0, sizeof md5);
	return md5;
}

// GUESS: a tag signature is taken to be the four characters of its ICC signature, such as "desc". The
// pages do not describe the string form; the ICC specification defines the signature as four bytes.
static bool signatureFromString(CFStringRef string, uint32_t* outSignature)
{
	uint8_t bytes[4];
	CFIndex used = 0;
	if (!string || CFStringGetLength(string) != 4)
		return false;
	if (CFStringGetBytes(string, CFRangeMake(0, 4), kCFStringEncodingISOLatin1, 0, false, bytes, sizeof bytes, &used) != 4 || used != 4)
		return false;
	*outSignature = ((uint32_t) bytes[0] << 24) | ((uint32_t) bytes[1] << 16) | ((uint32_t) bytes[2] << 8) | bytes[3];
	return true;
}

static CFStringRef stringFromSignature(uint32_t signature)
{
	const uint8_t bytes[4] = { signature >> 24, signature >> 16, signature >> 8, signature };
	return CFStringCreateWithBytes(kCFAllocatorDefault, bytes, sizeof bytes, kCFStringEncodingISOLatin1, false);
}

// GUESS: a NULL or non-profile argument, or a signature that is not four characters, gives false; the page does not say.
bool ColorSyncProfileContainsTag(ColorSyncProfileRef prof, CFStringRef signature)
{
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	uint32_t sig;
	ICCTag tag;
	return profile && signatureFromString(signature, &sig) && ICCProfileFindTag(CFDataGetBytePtr(profile->data), sig, &tag);
}

// GUESS: the tag data is the tag element as stored in the profile, starting with its type signature,
// without the padding after it (ICC.1:2022, 7.3.5). The page says only "the tag data".
CFDataRef ColorSyncProfileCopyTag(ColorSyncProfileRef prof, CFStringRef signature)
{
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	uint32_t sig;
	ICCTag tag;
	if (!profile || !signatureFromString(signature, &sig))
		return NULL;

	const uint8_t* bytes = CFDataGetBytePtr(profile->data);
	if (!ICCProfileFindTag(bytes, sig, &tag))
		return NULL;
	return CFDataCreate(kCFAllocatorDefault, bytes + tag.offset, tag.size);
}

// The two characters of a language or country code (ISO 639-1, ISO 3166-1) in the locale value, or false.
static bool twoLetterCode(CFTypeRef value, char out[3])
{
	return value && CFGetTypeID(value) == CFStringGetTypeID() && CFStringGetLength(value) == 2 &&
		CFStringGetCString(value, out, 3, kCFStringEncodingASCII);
}

// The page says "the localized description string ... localized to the current locale". The description is the profile's
// 'desc' tag; of its records the one for English (United States) is chosen if there is one, else the one for the language
// and country of CFLocaleCopyCurrent(), else the first of the same language, else the first (ICC.1:2022, 10.15; see the QUIRK
// in ICCProfile.c). The tag of an older profile has one string, and its 7-bit ASCII part is used, as in 10.20.3 of that
// edition. A NULL or non-profile argument gives NULL.
//
// QUIRK: a profile without a 'desc' tag, and one whose description cannot be read (an unknown tag type, a text that runs
// past the tag, no records or an empty text), give a fixed English text instead of NULL: "This profile has no description tag"
// and "This profile has a bad description tag". Evidence: black-box observation on macOS 26.7.3 (x86_64) and 10.14.4 (x86_64
// and i386), tier 2, of hand-built profiles. Whether the texts are localized is not known.
// Test: tests/src/colorsync_profile_description.c, "no description tag" and "bad description tag".
CFStringRef ColorSyncProfileCopyDescriptionString(ColorSyncProfileRef prof)
{
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	if (!profile)
		return NULL;

	char language[3], country[3];
	bool hasLanguage = false, hasCountry = false;
	CFLocaleRef locale = CFLocaleCopyCurrent();
	if (locale)
	{
		hasLanguage = twoLetterCode(CFLocaleGetValue(locale, kCFLocaleLanguageCode), language);
		hasCountry = twoLetterCode(CFLocaleGetValue(locale, kCFLocaleCountryCode), country);
		CFRelease(locale);
	}

	ICCTag descriptionTag;
	if (!ICCProfileFindTag(CFDataGetBytePtr(profile->data), SIG('d', 'e', 's', 'c'), &descriptionTag))
		return CFSTR("This profile has no description tag");
	ICCText text;
	if (!ICCProfileFindDescription(CFDataGetBytePtr(profile->data), hasLanguage ? language : NULL, hasCountry ? country : NULL, &text) || text.length == 0)
		return CFSTR("This profile has a bad description tag");
	return CFStringCreateWithBytes(kCFAllocatorDefault, text.bytes, (CFIndex) text.length,
		text.isUTF16BE ? kCFStringEncodingUTF16BE : kCFStringEncodingISOLatin1, false);
}

// GUESS: the signatures come in tag table order (the original's order for distinct signatures looked hash-based
// on macOS 26.7.3, so it is not reproduced), and a signature that repeats (invalid under ICC.1:2022, 7.3.1)
// is listed once, at its first position. The page says neither.
CFArrayRef ColorSyncProfileCopyTagSignatures(ColorSyncProfileRef prof)
{
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	if (!profile)
		return NULL;

	const uint8_t* bytes = CFDataGetBytePtr(profile->data);
	uint32_t* indices = malloc((size_t) ICCProfileTagCount(bytes) * sizeof(uint32_t) + 1);
	uint32_t count;
	if (!indices || !ICCProfileFirstTagIndices(bytes, indices, &count))
	{
		free(indices);
		return NULL;
	}

	CFMutableArrayRef signatures = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
	for (uint32_t i = 0; signatures && i < count; i++)
	{
		CFStringRef signature = stringFromSignature(ICCProfileTagAt(bytes, indices[i]).signature);
		if (!signature)
			continue;
		CFArrayAppendValue(signatures, signature);
		CFRelease(signature);
	}
	free(indices);
	return signatures;
}

// The page says only "matrix-based". The rule is that of ICCProfileHasMatrixTRC (see the QUIRK in ICCProfile.c: the colour
// space, the six tags of an RGB profile or kTRC of a gray one, and no AToB table). A NULL or non-profile argument gives
// false (macOS 26.7.3, observed).
bool ColorSyncProfileIsMatrixBased(ColorSyncProfileRef prof)
{
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	return profile && ICCProfileHasMatrixTRC(CFDataGetBytePtr(profile->data));
}

// The bytes of the file at a file URL, or NULL when the URL is not a file URL or the file cannot be read in full:
// missing, unreadable, not a regular file, or larger than a profile can be (its size field is 32 bits).
static CFDataRef copyFileData(CFURLRef url)
{
	char path[PATH_MAX];
	CFStringRef scheme = CFURLCopyScheme(url);
	const bool isFile = scheme && CFStringCompare(scheme, CFSTR("file"), kCFCompareCaseInsensitive) == kCFCompareEqualTo;
	if (scheme)
		CFRelease(scheme);
	if (!isFile || !CFURLGetFileSystemRepresentation(url, true, (UInt8*) path, sizeof path))
		return NULL;

	const int fd = open(path, O_RDONLY);
	if (fd < 0)
		return NULL;

	struct stat st;
	if (fstat(fd, &st) < 0 || !S_ISREG(st.st_mode) || st.st_size > UINT32_MAX)
	{
		close(fd);
		return NULL;
	}

	// Read at most the size seen by fstat, so a file that grows meanwhile cannot make the read run on.
	CFMutableDataRef data = CFDataCreateMutable(kCFAllocatorDefault, 0);
	if (!data)
	{
		close(fd);
		return NULL;
	}

	uint8_t chunk[16384];
	off_t remaining = st.st_size;
	while (remaining > 0)
	{
		const ssize_t got = read(fd, chunk, remaining < (off_t) sizeof chunk ? (size_t) remaining : sizeof chunk);
		if (got < 0 && errno == EINTR)
			continue;
		if (got < 0)
		{
			CFRelease(data);
			data = NULL;
			break;
		}
		if (got == 0)
			break; // the file shrank; what was read is what the profile is made from
		CFDataAppendBytes(data, chunk, got);
		remaining -= got;
	}
	close(fd);
	return data;
}

// Observed on macOS 26.7.3 and 10.14.4 (probe, tests/src/colorsync_profile_url.c): every failure, whether the URL is
// NULL, not a file URL, missing, unreadable, a directory or not a profile, returns NULL and leaves the error NULL;
// the error is only cleared. The profile keeps the URL as given (a relative URL stays relative).
ColorSyncProfileRef ColorSyncProfileCreateWithURL(CFURLRef url, CFErrorRef* error)
{
	if (error)
		*error = NULL;
	if (!url)
		return NULL;

	CFDataRef data = copyFileData(url);
	if (!data)
		return NULL;

	ColorSyncProfileRef prof = ColorSyncProfileCreate(data, NULL);
	CFRelease(data);
	if (prof)
		((struct __ColorSyncProfile*) prof)->url = CFRetain(url);
	return prof;
}

// Observed on macOS 26.7.3 and 10.14.4: a profile that was not created from a URL, a NULL argument and an argument
// that is not a profile all give NULL with the error left NULL. The URL belongs to the profile (a "Get" function adds
// no reference) and is returned even after the file is gone.
CFURLRef ColorSyncProfileGetURL(ColorSyncProfileRef prof, CFErrorRef* error)
{
	if (error)
		*error = NULL;

	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	return profile ? profile->url : NULL;
}

// The page says only that the result is a mutable copy of the profile's data. The copy is a new object with the same data
// and no URL (GetURL gives NULL, as for a profile made from data); a copy of a mutable profile is mutable too, and a NULL
// or non-profile argument gives NULL (macOS 26.7.3 and 10.14.4, observed).
ColorSyncMutableProfileRef ColorSyncProfileCreateMutableCopy(ColorSyncProfileRef prof)
{
	const struct __ColorSyncProfile* source = profileOrNull(prof);
	if (!source)
		return NULL;
	ColorSyncProfileRef copy = ColorSyncProfileCreate(source->data, NULL);
	if (copy)
		((struct __ColorSyncProfile*) copy)->isMutable = true;
	return copy;
}

// The page says the header is "in host endianness" and does not list the fields. Observed on macOS 26.7.3 and 10.14.4 (x86_64):
// each field of the header is a number of its type, swapped on its own, as in the C struct that the public CMICCProfile.h of
// the MacOSX10.1.5 SDK declares as CM2Header and CM4Header (github.com/phracker/MacOSX-SDKs). So the 32-bit words are
// swapped (signatures too, so 'RGB ' is 0x52474220 on a little-endian host), the date and time is six 16-bit numbers
// (CMDateTime), and the device attributes are two 32-bit words (`UInt32 deviceAttributes[2]`), not one 64-bit number;
// the Profile ID and the reserved bytes are bytes. See ICCHeaderSwapFields. A NULL or non-profile argument gives NULL, which
// the page says is the failure result.
CFDataRef ColorSyncProfileCopyHeader(ColorSyncProfileRef prof)
{
	const struct __ColorSyncProfile* profile = profileOrNull(prof);
	if (!profile)
		return NULL;
	uint8_t header[ICC_HEADER_SIZE];
	ICCHeaderSwapFields(CFDataGetBytePtr(profile->data), header);
	return CFDataCreate(kCFAllocatorDefault, header, sizeof header);
}

// The function has no result. Observed on macOS 26.7.3 and 10.14.4: a profile that is not mutable, a NULL profile, a non-profile
// and a header that is not 128 bytes leave the profile as it was, without a result. The header, taken in the layout of
// CopyHeader, is stored as given: no field is checked or recomputed, so a header without the file signature 'acsp', or
// with another size, gives data that ColorSyncProfileCreate would reject or that disagrees with its size field. A NULL
// header crashes on macOS; here it is ignored too. Like other mutable CoreFoundation objects, a mutable profile is not safe
// to change from several threads at once.
void ColorSyncProfileSetHeader(ColorSyncMutableProfileRef prof, CFDataRef header)
{
	struct __ColorSyncProfile* profile = (struct __ColorSyncProfile*) profileOrNull(prof);
	if (!profile || !profile->isMutable || !header || CFDataGetLength(header) != ICC_HEADER_SIZE)
		return;

	const CFIndex length = CFDataGetLength(profile->data);
	CFMutableDataRef updated = CFDataCreateMutableCopy(kCFAllocatorDefault, length, profile->data);
	if (!updated)
		return;
	ICCHeaderSwapFields(CFDataGetBytePtr(header), CFDataGetMutableBytePtr(updated));

	// An immutable copy, as in ColorSyncProfileCreate: data returned earlier by CopyData must not change under its holder.
	CFDataRef frozen = CFDataCreateCopy(kCFAllocatorDefault, updated);
	CFRelease(updated);
	if (!frozen)
		return;
	CFRelease(profile->data);
	profile->data = frozen;
}
