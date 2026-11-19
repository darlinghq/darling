/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Family queries for the font families every macOS provides (Times,
// Helvetica, Arial, Lucida Grande, ...): the collection query, the descriptor
// query and the fonts made from the results agree with each other, and a
// family that does not exist matches nothing. The expected behaviour was read
// from macOS 26.7.3 through the public API with
// vd-upstream/mac-probes/coretext-family-matching/probe.c: each of these
// families has at least one face, the descriptors and fonts of a family report
// one family name (on macOS the requested one; where another installed family
// stands in for the requested one, that family's name), and CTFontCreateWithName
// for an unknown name returns a (fallback) font rather than NULL.

#include <CoreText/CoreText.h>
#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static CFStringRef makeString(const char *text)
{
	return CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
}

static int familyIs(CFStringRef actual, CFStringRef expected)
{
	return actual != NULL && CFStringCompare(actual, expected, 0) == kCFCompareEqualTo;
}

static CTFontDescriptorRef familyDescriptor(CFStringRef family)
{
	const void *keys[] = { kCTFontFamilyNameAttribute };
	const void *values[] = { family };
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
	CFRelease(attributes);
	return descriptor;
}

static CFStringRef copyDescriptorFamily(CFArrayRef descriptors, CFIndex index)
{
	return CTFontDescriptorCopyAttribute(
		(CTFontDescriptorRef) CFArrayGetValueAtIndex(descriptors, index),
		kCTFontFamilyNameAttribute);
}

static void checkFamily(const char *name)
{
	CFStringRef family = makeString(name);
	CTFontCollectionRef all = CTFontCollectionCreateFromAvailableFonts(NULL);

	// The collection query.
	CFArrayRef fromCollection =
		CTFontCollectionCreateMatchingFontDescriptorsForFamily(all, family, NULL);
	if (fromCollection == NULL || CFArrayGetCount(fromCollection) == 0)
	{
		printf("FAIL: collection query for %s found nothing\n", name);
		g_failures++;
		goto done;
	}

	// Whatever family the descriptors report (macOS: the requested name), they all
	// report the same one.
	CFStringRef reportedFamily = copyDescriptorFamily(fromCollection, 0);
	CHECK(reportedFamily != NULL);
	if (reportedFamily == NULL)
		goto done;
	for (CFIndex i = 1; i < CFArrayGetCount(fromCollection); i++)
	{
		CFStringRef other = copyDescriptorFamily(fromCollection, i);
		if (!familyIs(other, reportedFamily))
		{
			printf("FAIL: collection descriptors of %s report different families\n", name);
			g_failures++;
		}
		if (other != NULL)
			CFRelease(other);
	}
	// The descriptor query with a family attribute gives the same faces under the
	// same family.
	CTFontDescriptorRef query = familyDescriptor(family);
	CFArrayRef fromDescriptor = CTFontDescriptorCreateMatchingFontDescriptors(query, NULL);
	CHECK(fromDescriptor != NULL);
	if (fromDescriptor != NULL)
	{
		CHECK(CFArrayGetCount(fromDescriptor) == CFArrayGetCount(fromCollection));
		for (CFIndex i = 0; i < CFArrayGetCount(fromDescriptor); i++)
		{
			CFStringRef other = copyDescriptorFamily(fromDescriptor, i);
			if (!familyIs(other, reportedFamily))
			{
				printf("FAIL: descriptor query for %s reports another family\n", name);
				g_failures++;
			}
			if (other != NULL)
				CFRelease(other);
		}
		CFRelease(fromDescriptor);
	}

	// A font made from a matched descriptor reports that family, and so does the
	// font made by the family name.
	CTFontRef fromMatch = CTFontCreateWithFontDescriptor(
		(CTFontDescriptorRef) CFArrayGetValueAtIndex(fromCollection, 0), 12, NULL);
	CHECK(fromMatch != NULL);
	if (fromMatch != NULL)
	{
		CFStringRef fontFamily = CTFontCopyFamilyName(fromMatch);
		if (!familyIs(fontFamily, reportedFamily))
		{
			printf("FAIL: font made from a descriptor of %s reports another family\n", name);
			g_failures++;
		}
		if (fontFamily != NULL)
			CFRelease(fontFamily);
		CFStringRef postScript = CTFontCopyPostScriptName(fromMatch);
		CHECK(postScript != NULL);
		if (postScript != NULL)
			CFRelease(postScript);
		CFRelease(fromMatch);
	}

	CTFontRef byName = CTFontCreateWithName(family, 12, NULL);
	CHECK(byName != NULL);
	if (byName != NULL)
	{
		CFStringRef fontFamily = CTFontCopyFamilyName(byName);
		if (!familyIs(fontFamily, reportedFamily))
		{
			printf("FAIL: CTFontCreateWithName(%s) reports another family\n", name);
			g_failures++;
		}
		if (fontFamily != NULL)
			CFRelease(fontFamily);
		CFRelease(byName);
	}
	CFRelease(query);
	CFRelease(reportedFamily);

done:
	if (fromCollection != NULL)
		CFRelease(fromCollection);
	CFRelease(all);
	CFRelease(family);
}

static void checkNonexistentFamily(void)
{
	CFStringRef family = CFSTR("NoSuchFamilyXyz");
	CTFontCollectionRef all = CTFontCollectionCreateFromAvailableFonts(NULL);

	CFArrayRef fromCollection =
		CTFontCollectionCreateMatchingFontDescriptorsForFamily(all, family, NULL);
	CHECK(fromCollection == NULL);

	CTFontDescriptorRef query = familyDescriptor(family);
	CFArrayRef fromDescriptor = CTFontDescriptorCreateMatchingFontDescriptors(query, NULL);
	CHECK(fromDescriptor == NULL);

	// An unknown name still gives a font (the documented fallback), and that font
	// does not claim to be the unknown family.
	CTFontRef fallback = CTFontCreateWithName(family, 12, NULL);
	CHECK(fallback != NULL);
	if (fallback != NULL)
	{
		CFStringRef reported = CTFontCopyFamilyName(fallback);
		CHECK(reported != NULL && !familyIs(reported, family));
		if (reported != NULL)
			CFRelease(reported);
		CFRelease(fallback);
	}

	if (fromCollection != NULL)
		CFRelease(fromCollection);
	if (fromDescriptor != NULL)
		CFRelease(fromDescriptor);
	CFRelease(query);
	CFRelease(all);
}

int main(void)
{
	static const char *const families[] = {
		"Times", "Helvetica", "Arial", "Lucida Grande", "Courier", "Menlo", "Georgia",
	};

	for (size_t i = 0; i < sizeof(families) / sizeof(families[0]); i++)
		checkFamily(families[i]);
	checkNonexistentFamily();

	if (g_failures != 0)
	{
		printf("coretext_family_matching: %d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS coretext_family_matching\n");
	return 0;
}
