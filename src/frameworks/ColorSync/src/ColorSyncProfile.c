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
#include <stdlib.h>
#include "ICCProfile.h"

struct __ColorSyncProfile
{
	CFRuntimeBase base;
	CFDataRef data;
};

static void ColorSyncProfileFinalize(CFTypeRef cf)
{
	CFRelease(((struct __ColorSyncProfile*) cf)->data);
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
