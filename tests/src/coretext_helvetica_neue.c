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

// The family "Helvetica Neue". On macOS it is /System/Library/Fonts/HelveticaNeue.ttc;
// applications look the family up through CoreText, take the file URL of the
// match and open the file themselves, comparing the family name of its faces
// with the name they looked up. The expected behaviour (what CTFontCreateWithName
// returns for the family and PostScript names, the order and names of the first
// faces of the collection, kCTFontURLAttribute) was read from macOS 26.7.3 and
// 10.14.4 through the public API with vd-upstream/mac-probes/helvetica-neue/hn-faces.c.
// macOS lists 14 faces (Regular, Bold, Italic, Bold Italic first); Darling
// provides the four of them its freely licensed stand-in has, so only
// those are required here, and every face listed must be of the family.
//
// Usage: coretext_helvetica_neue

#include <CoreText/CoreText.h>
#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int is(CFTypeRef value, const char *expected)
{
	char buffer[256];
	return value != NULL && CFGetTypeID(value) == CFStringGetTypeID() &&
		CFStringGetCString((CFStringRef) value, buffer, sizeof buffer, kCFStringEncodingUTF8) &&
		strcmp(buffer, expected) == 0;
}

static int urlPathIs(CFTypeRef value, const char *expected)
{
	char buffer[1024];
	return value != NULL && CFGetTypeID(value) == CFURLGetTypeID() &&
		CFURLGetFileSystemRepresentation((CFURLRef) value, true, (UInt8 *) buffer, sizeof buffer) &&
		strcmp(buffer, expected) == 0;
}

static const char *kFile = "/System/Library/Fonts/HelveticaNeue.ttc";

static void checkNamedFont(const char *name, const char *postScript, const char *full)
{
	CFStringRef cfName = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
	CTFontRef font = CTFontCreateWithName(cfName, 12, NULL);
	CFRelease(cfName);
	if (font == NULL) {
		printf("FAIL: no font for %s\n", name);
		g_failures++;
		return;
	}
	CFStringRef family = CTFontCopyFamilyName(font);
	CFStringRef ps = CTFontCopyPostScriptName(font);
	CFStringRef fullName = CTFontCopyFullName(font);
	CFTypeRef url = CTFontCopyAttribute(font, kCTFontURLAttribute);
	if (!is(family, "Helvetica Neue") || !is(ps, postScript) || !is(fullName, full) ||
			!urlPathIs(url, kFile)) {
		printf("FAIL: %s: family/postscript/full/url differ\n", name);
		g_failures++;
	}
	if (family) CFRelease(family);
	if (ps) CFRelease(ps);
	if (fullName) CFRelease(fullName);
	if (url) CFRelease(url);
	CFRelease(font);
}

int main(void)
{
	checkNamedFont("Helvetica Neue", "HelveticaNeue", "Helvetica Neue");
	checkNamedFont("HelveticaNeue", "HelveticaNeue", "Helvetica Neue");
	checkNamedFont("HelveticaNeue-Bold", "HelveticaNeue-Bold", "Helvetica Neue Bold");
	checkNamedFont("HelveticaNeue-Italic", "HelveticaNeue-Italic", "Helvetica Neue Italic");
	checkNamedFont("HelveticaNeue-BoldItalic", "HelveticaNeue-BoldItalic", "Helvetica Neue Bold Italic");
	checkNamedFont("Helvetica Neue Bold", "HelveticaNeue-Bold", "Helvetica Neue Bold");

	// The file the font names, read the way an application reads it.
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) kFile, strlen(kFile), false);
	CFArrayRef faces = CTFontManagerCreateFontDescriptorsFromURL(url);
	CFRelease(url);
	CHECK(faces != NULL && CFArrayGetCount(faces) >= 4);
	static const char *postScriptNames[] = {
		"HelveticaNeue", "HelveticaNeue-Bold", "HelveticaNeue-Italic", "HelveticaNeue-BoldItalic" };
	static const char *styles[] = { "Regular", "Bold", "Italic", "Bold Italic" };
	for (CFIndex i = 0; faces != NULL && i < CFArrayGetCount(faces); i++) {
		CTFontDescriptorRef face = (CTFontDescriptorRef) CFArrayGetValueAtIndex(faces, i);
		CFTypeRef family = CTFontDescriptorCopyAttribute(face, kCTFontFamilyNameAttribute);
		CHECK(is(family, "Helvetica Neue"));
		if (family) CFRelease(family);
		if (i < 4) {
			CFTypeRef ps = CTFontDescriptorCopyAttribute(face, kCTFontNameAttribute);
			CFTypeRef style = CTFontDescriptorCopyAttribute(face, kCTFontStyleNameAttribute);
			CHECK(is(ps, postScriptNames[i]));
			CHECK(is(style, styles[i]));
			if (ps) CFRelease(ps);
			if (style) CFRelease(style);
		}
	}
	if (faces) CFRelease(faces);

	// The family lookup through a collection lists faces of the file.
	CFStringRef familyName = CFSTR("Helvetica Neue");
	const void *keys[] = { kCTFontFamilyNameAttribute };
	const void *values[] = { familyName };
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CTFontDescriptorRef query = CTFontDescriptorCreateWithAttributes(attributes);
	CFRelease(attributes);
	CFArrayRef queries = CFArrayCreate(NULL, (const void **) &query, 1, &kCFTypeArrayCallBacks);
	CTFontCollectionRef collection = CTFontCollectionCreateWithFontDescriptors(queries, NULL);
	CFArrayRef matches = CTFontCollectionCreateMatchingFontDescriptors(collection);
	CHECK(matches != NULL && CFArrayGetCount(matches) >= 4);
	for (CFIndex i = 0; matches != NULL && i < CFArrayGetCount(matches); i++) {
		CTFontDescriptorRef match = (CTFontDescriptorRef) CFArrayGetValueAtIndex(matches, i);
		CFTypeRef family = CTFontDescriptorCopyAttribute(match, kCTFontFamilyNameAttribute);
		CFTypeRef url = CTFontDescriptorCopyAttribute(match, kCTFontURLAttribute);
		CHECK(is(family, "Helvetica Neue"));
		CHECK(urlPathIs(url, kFile));
		if (family) CFRelease(family);
		if (url) CFRelease(url);

		// The traits of a match carry all four values; a client that reads the slant of every match
		// without a check for its presence (a text renderer choosing bold and italic faces) must find
		// it. macOS: Regular weight 0 slant 0, Bold weight 0.4, Italic slant 0.067, width 0 for these four
		// faces (the condensed faces of the macOS font have -0.2).
		CFDictionaryRef traits = CTFontDescriptorCopyAttribute(match, kCTFontTraitsAttribute);
		CFNumberRef weight = traits ? CFDictionaryGetValue(traits, kCTFontWeightTrait) : NULL;
		CFNumberRef width = traits ? CFDictionaryGetValue(traits, kCTFontWidthTrait) : NULL;
		CFNumberRef slant = traits ? CFDictionaryGetValue(traits, kCTFontSlantTrait) : NULL;
		CFNumberRef symbolic = traits ? CFDictionaryGetValue(traits, kCTFontSymbolicTrait) : NULL;
		CHECK(weight != NULL && width != NULL && slant != NULL && symbolic != NULL);
		if (weight && width && slant && symbolic) {
			double w = 0, x = 0, sl = 0;
			CFNumberGetValue(weight, kCFNumberDoubleType, &w);
			CFNumberGetValue(width, kCFNumberDoubleType, &x);
			CFNumberGetValue(slant, kCFNumberDoubleType, &sl);
			CFTypeRef ps = CTFontDescriptorCopyAttribute(match, kCTFontNameAttribute);
			if (is(ps, "HelveticaNeue")) CHECK(w == 0 && x == 0 && sl == 0);
			if (is(ps, "HelveticaNeue-Bold")) CHECK(w > 0.23 && x == 0 && sl == 0);
			if (is(ps, "HelveticaNeue-Italic")) CHECK(w == 0 && x == 0 && sl > 0.03);
			if (is(ps, "HelveticaNeue-BoldItalic")) CHECK(w > 0.23 && x == 0 && sl > 0.03);
			if (ps) CFRelease(ps);
		}
		if (traits) CFRelease(traits);
	}
	if (matches) CFRelease(matches);
	CFRelease(collection);
	CFRelease(queries);
	CFRelease(query);

	printf(g_failures == 0 ? "PASS\n" : "FAILED (%d)\n", g_failures);
	return g_failures != 0;
}
