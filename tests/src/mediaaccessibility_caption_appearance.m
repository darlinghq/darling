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

// MACaptionAppearance and the other MediaAccessibility getters return real
// values: retained RGB colours, CGFloat opacities and sizes, enum styles, font
// descriptors, and the "behavior" out-parameter (which may be NULL).
//
// The expected values are the system defaults observed on macOS 26.7.3
// (vd-upstream/mac-probes/mediaaccessibility-caption-*.mac26.txt); the program
// passes unchanged there with default settings. The setters change settings
// system-wide on a Mac, so they are exercised only with the "--mutate" argument.
//
// Built without ARC.
#import <CoreFoundation/CoreFoundation.h>
#import <MediaAccessibility/MediaAccessibility.h>
#import <CoreText/CoreText.h>
#include <stdio.h>
#include <string.h>

// Private signature used by WebKit.
extern CTFontDescriptorRef MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(MACaptionAppearanceDomain,
	MACaptionAppearanceBehavior*, MACaptionAppearanceFontStyle, CFStringRef, CGFloat, CGFloat*);

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static int near(double a, double b)
{
	return a - b < 0.0001 && b - a < 0.0001;
}

static int isStr(CFStringRef s, const char* expected)
{
	char buf[256];
	return s && CFStringGetCString(s, buf, sizeof(buf), kCFStringEncodingUTF8) && strcmp(buf, expected) == 0;
}

// Checks an RGB colour of four components with equal r, g, b and alpha 1.
static void checkColor(CGColorRef c, double gray, MACaptionAppearanceBehavior got, MACaptionAppearanceBehavior expected)
{
	CHECK(c != NULL);
	CHECK(got == expected);
	if (!c)
		return;
	CHECK(CGColorGetNumberOfComponents(c) == 4);
	CHECK(CGColorSpaceGetModel(CGColorGetColorSpace(c)) == kCGColorSpaceModelRGB);
	const CGFloat* p = CGColorGetComponents(c);
	CHECK(p != NULL);
	if (p && CGColorGetNumberOfComponents(c) == 4)
		CHECK(near(p[0], gray) && near(p[1], gray) && near(p[2], gray) && near(p[3], 1));
	CGColorRelease(c);
}

static void checkValue(double got, double expected, MACaptionAppearanceBehavior gotB, MACaptionAppearanceBehavior expectedB, int line)
{
	if (!near(got, expected) || gotB != expectedB)
	{
		printf("FAIL line %d: value %g behavior %ld, expected %g behavior %ld\n", line, got, (long) gotB, expected, (long) expectedB);
		failures++;
	}
}
#define CHECKV(got, exp, gotB, expB) checkValue((got), (exp), (gotB), (expB), __LINE__)

static const char* const fonts[] = {
	".AppleSystemUIFontMedium", ".AppleSystemUIFontMonospaced-Regular", "IowanOldStyle-Bold", "Menlo-Bold",
	"TrebuchetMS-Bold", "AvenirNext-Medium", ".SavoyeLetPlainCC", "Copperplate-Bold", ".AppleSystemUIFont",
};

// Expected values of one profile in the user domain.
struct expect
{
	const char* profile;
	const char* name;
	MACaptionAppearanceBehavior fg, bg, win;
	double fgOp; MACaptionAppearanceBehavior fgOpB;
	double bgOp; MACaptionAppearanceBehavior bgOpB;
	double winOp; MACaptionAppearanceBehavior winOpB;
	double radius, size; MACaptionAppearanceBehavior sizeB;
	long edge; double stroke20;
};

static const struct expect profiles[] = {
	{ "MACaptionProfile-Classic", "Classic", 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ "MACaptionProfile-LargeText", "Large Text", 1, 0, 1, 1, 1, 0, 0, 0.5, 0, 10, 1.5, 0, 1, 0 },
	{ "MACaptionProfile-OutlineText", "Outline Text", 1, 0, 1, 1, 1, 0, 0, 0, 0, 10, 1.5, 1, 4, 1.94444 },
	{ "MACaptionProfile-Default", "Transparent Background", 1, 0, 1, 1, 1, 0, 0, 0.5, 0, 8, 1, 1, 1, 0 },
};

static void checkDomain(MACaptionAppearanceDomain d, const struct expect* e)
{
	MACaptionAppearanceBehavior b;
	int user = d == kMACaptionAppearanceDomainUser;

	b = 99; checkColor(MACaptionAppearanceCopyForegroundColor(d, &b), 1, b, user ? e->fg : 0);
	b = 99; checkColor(MACaptionAppearanceCopyBackgroundColor(d, &b), 0, b, user ? e->bg : 0);
	b = 99; checkColor(MACaptionAppearanceCopyWindowColor(d, &b), 0, b, user ? e->win : 0);
	b = 99; CHECKV(MACaptionAppearanceGetForegroundOpacity(d, &b), 1, b, user ? e->fgOpB : 0);
	b = 99; CHECKV(MACaptionAppearanceGetBackgroundOpacity(d, &b), user ? e->bgOp : 0, b, user ? e->bgOpB : 0);
	b = 99; CHECKV(MACaptionAppearanceGetWindowOpacity(d, &b), user ? e->winOp : 0.5, b, user ? e->winOpB : 0);
	b = 99; CHECKV(MACaptionAppearanceGetWindowRoundedCornerRadius(d, &b), user ? e->radius : 8, b, 0);
	b = 99; CHECKV(MACaptionAppearanceGetRelativeCharacterSize(d, &b), user ? e->size : 1, b, user ? e->sizeB : 0);
	b = 99;
	long edge = MACaptionAppearanceGetTextEdgeStyle(d, &b);
	CHECK(edge == (user ? e->edge : kMACaptionAppearanceTextEdgeStyleNone));
	CHECK(b == (user ? 1 : 0));

	for (int s = 0; s <= 8; s++)
	{
		b = 99;
		CTFontDescriptorRef fd = MACaptionAppearanceCopyFontDescriptorForStyle(d, &b, s);
		CHECK(fd != NULL);
		CHECK(b == ((user && s < 8) ? 1 : 0));
		if (!fd)
			continue;
		CFStringRef name = CTFontDescriptorCopyAttribute(fd, kCTFontNameAttribute);
		const char* want = fonts[s];
		if (user && s < 8 && e->profile[17] == 'C') // Classic: every style is monospaced
			want = fonts[1];
		else if (user && s == 0 && e->profile[17] != 'D') // Large Text, Outline Text: plain system font
			want = fonts[8];
		if (!isStr(name, want))
		{
			printf("FAIL line %d: font of style %d is not %s\n", __LINE__, s, want);
			failures++;
		}
		if (name)
			CFRelease(name);
		CFRelease(fd);
	}

	CGFloat stroke = -1;
	b = 99;
	CTFontDescriptorRef fd = MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(d, &b, kMACaptionAppearanceFontStyleDefault, CFSTR("en"), 20, &stroke);
	CHECK(fd != NULL);
	CHECK(b == (user ? 1 : 0));
	CHECK(near(stroke, user ? e->stroke20 : 0));
	if (fd)
		CFRelease(fd);
}

int main(int argc, char** argv)
{
	int mutate = argc > 1 && strcmp(argv[1], "--mutate") == 0;

	// Constants.
	CHECK(isStr(kMACaptionAppearanceSettingsChangedNotification, "com.apple.mediaaccessibility.captionAppearanceSettingsChanged"));
	CHECK(isStr(kMAAudibleMediaSettingsChangedNotification, "com.apple.mediaaccessibility.audibleMediaSettingsChanged"));
	CHECK(isStr(kMADimFlashingLightsChangedNotification, "com.apple.accessibility.dim.flashing.lights.mitigation"));
	CHECK(isStr(MAMediaCharacteristicDescribesMusicAndSoundForAccessibility, "public.accessibility.describes-music-and-sound"));
	CHECK(isStr(MAMediaCharacteristicTranscribesSpokenDialogForAccessibility, "public.accessibility.transcribes-spoken-dialog"));
	CHECK(isStr(MAMediaCharacteristicDescribesVideoForAccessibility, "public.accessibility.describes-video"));

	// Profiles, all checked through the same entry points.
	CFArrayRef ids = MACaptionAppearanceCopyProfileIDs();
	CHECK(ids && CFArrayGetCount(ids) == 4);
	CFStringRef active = MACaptionAppearanceCopyActiveProfileID();
	CHECK(isStr(active, "MACaptionProfile-Default"));
	CHECK(isStr(MACaptionAppearanceCopyProfileName(active), "Transparent Background"));
	CHECK(MACaptionAppearanceCopyProfileName(CFSTR("no-such-profile")) == NULL);

	// The values of the active profile, without ExecuteBlockForProfileID.
	checkDomain(kMACaptionAppearanceDomainUser, &profiles[3]);
	checkDomain(kMACaptionAppearanceDomainDefault, &profiles[3]);

	for (size_t i = 0; i < 4; i++)
	{
		CHECK(ids && CFArrayGetCount(ids) == 4 && isStr(CFArrayGetValueAtIndex(ids, i), profiles[i].profile));
		CFStringRef pid = CFStringCreateWithCString(NULL, profiles[i].profile, kCFStringEncodingUTF8);
		CHECK(isStr(MACaptionAppearanceCopyProfileName(pid), profiles[i].name));
		__block int ran = 0;
		const struct expect* e = &profiles[i];
		MACaptionAppearanceExecuteBlockForProfileID(pid, ^{
			ran++;
			checkDomain(kMACaptionAppearanceDomainUser, e);
			checkDomain(kMACaptionAppearanceDomainDefault, e);
		});
		CHECK(ran == 1);
		CFRelease(pid);
	}
	// The block does not change the active profile.
	CFStringRef again = MACaptionAppearanceCopyActiveProfileID();
	CHECK(isStr(again, "MACaptionProfile-Default"));

	// NULL behavior is allowed everywhere.
	CGColorRef c = MACaptionAppearanceCopyForegroundColor(kMACaptionAppearanceDomainUser, NULL);
	CHECK(c != NULL);
	if (c)
	{
		CHECK(CGColorGetNumberOfComponents(c) == 4);
		CGColorRelease(c);
	}
	CHECK(near(MACaptionAppearanceGetForegroundOpacity(kMACaptionAppearanceDomainUser, NULL), 1));
	CHECK(near(MACaptionAppearanceGetRelativeCharacterSize(kMACaptionAppearanceDomainUser, NULL), 1));
	CHECK(MACaptionAppearanceGetTextEdgeStyle(kMACaptionAppearanceDomainUser, NULL) == kMACaptionAppearanceTextEdgeStyleNone);
	CTFontDescriptorRef fd = MACaptionAppearanceCopyFontDescriptorForStyle(kMACaptionAppearanceDomainUser, NULL, kMACaptionAppearanceFontStyleCursive);
	CHECK(fd != NULL);
	if (fd)
		CFRelease(fd);
	fd = MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(kMACaptionAppearanceDomainUser, NULL, 0, NULL, 20, NULL);
	CHECK(fd != NULL);
	if (fd)
		CFRelease(fd);

	// Outline stroke grows with the point size up to a limit.
	__block CGFloat s10 = -1, s100 = -1;
	MACaptionAppearanceBehavior b;
	CFStringRef outline = CFSTR("MACaptionProfile-OutlineText");
	MACaptionAppearanceExecuteBlockForProfileID(outline, ^{
		MACaptionAppearanceBehavior bb;
		CTFontDescriptorRef f1 = MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(kMACaptionAppearanceDomainUser, &bb, 0, CFSTR("en"), 10, &s10);
		CTFontDescriptorRef f2 = MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(kMACaptionAppearanceDomainUser, &bb, 0, CFSTR("en"), 100, &s100);
		if (f1) CFRelease(f1);
		if (f2) CFRelease(f2);
	});
	CHECK(near(s10, 0.972222));
	CHECK(near(s100, 6.31944));

	// General preferences.
	CHECK(MACaptionAppearanceGetDisplayType(kMACaptionAppearanceDomainUser) == kMACaptionAppearanceDisplayTypeAutomatic);
	CHECK(MACaptionAppearanceGetDisplayType(kMACaptionAppearanceDomainDefault) == kMACaptionAppearanceDisplayTypeAutomatic);
	CHECK(!MACaptionAppearanceIsCustomized(kMACaptionAppearanceDomainUser));
	for (int d = 0; d <= 1; d++)
	{
		CFArrayRef chars = MACaptionAppearanceCopyPreferredCaptioningMediaCharacteristics(d);
		CHECK(chars && CFArrayGetTypeID() == CFGetTypeID(chars) && CFArrayGetCount(chars) == 0);
		CFArrayRef langs = MACaptionAppearanceCopySelectedLanguages(d);
		CHECK(langs && CFArrayGetCount(langs) > 0);
		for (CFIndex i = 0; langs && i < CFArrayGetCount(langs); i++)
			CHECK(CFGetTypeID(CFArrayGetValueAtIndex(langs, i)) == CFStringGetTypeID());
	}
	CFArrayRef audible = MAAudibleMediaCopyPreferredCharacteristics();
	CHECK(audible && CFArrayGetCount(audible) == 0);
	CHECK(!MADimFlashingLightsEnabled());
	CFStringRef tagPath = MAImageCaptioningCopyMetadataTagPath();
	CHECK(isStr(tagPath, "Iptc4xmpExt:ArtworkContentDescription"));
	CFArrayRef none = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	MACaptionAppearanceDidDisplayCaptions(none);
	CFRelease(none);

	if (mutate)
	{
		// Setters take effect for later reads; a language added last comes first.
		CHECK(MACaptionAppearanceAddSelectedLanguage(kMACaptionAppearanceDomainUser, CFSTR("fr")));
		CFArrayRef langs = MACaptionAppearanceCopySelectedLanguages(kMACaptionAppearanceDomainUser);
		CHECK(langs && CFArrayGetCount(langs) > 0 && isStr(CFArrayGetValueAtIndex(langs, 0), "fr"));
		CHECK(!MACaptionAppearanceAddSelectedLanguage(kMACaptionAppearanceDomainUser, CFSTR("")));

		MACaptionAppearanceSetDisplayType(kMACaptionAppearanceDomainUser, kMACaptionAppearanceDisplayTypeAlwaysOn);
		CHECK(MACaptionAppearanceGetDisplayType(kMACaptionAppearanceDomainUser) == kMACaptionAppearanceDisplayTypeAlwaysOn);
		CHECK(MACaptionAppearanceGetDisplayType(kMACaptionAppearanceDomainDefault) == kMACaptionAppearanceDisplayTypeAutomatic);
		MACaptionAppearanceSetDisplayType(kMACaptionAppearanceDomainUser, kMACaptionAppearanceDisplayTypeAutomatic);

		MACaptionAppearanceSetActiveProfileID(outline);
		b = 99;
		CHECK(MACaptionAppearanceGetTextEdgeStyle(kMACaptionAppearanceDomainUser, &b) == kMACaptionAppearanceTextEdgeStyleUniform);
		MACaptionAppearanceSetActiveProfileID(CFSTR("MACaptionProfile-Default"));
	}

	if (failures)
	{
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
