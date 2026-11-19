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

// -[NSFontManager availableMembersOfFontFamily:] for the families every macOS
// provides (Times, Helvetica, Arial, Lucida Grande), with no window server:
// each member is [PostScript name, style name, weight, traits], and the
// PostScript name makes a font through +[NSFont fontWithName:size:]. The
// expected behaviour was read from macOS 26.7.3 with
// vd-upstream/mac-probes/appkit-fontmanager-family-members/probe.m: every one
// of these families has Regular and Bold members, and a family that does not
// exist has no members. Font enumeration needs no display: run it with DISPLAY
// and WAYLAND_DISPLAY unset (unsetenv below only covers the process itself).
// No ARC.

#import <AppKit/AppKit.h>
#include <stdio.h>
#include <stdlib.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void checkFamily(NSFontManager *manager, NSString *family)
{
	NSArray *members = [manager availableMembersOfFontFamily: family];
	if ([members count] == 0)
	{
		printf("FAIL: no members for %s\n", [family UTF8String]);
		g_failures++;
		return;
	}

	BOOL hasRegular = NO, hasBold = NO;
	for (NSArray *member in members)
	{
		if (![member isKindOfClass: [NSArray class]] || [member count] != 4)
		{
			printf("FAIL: a member of %s is not an array of four\n", [family UTF8String]);
			g_failures++;
			continue;
		}
		NSString *name = [member objectAtIndex: 0];
		NSString *style = [member objectAtIndex: 1];
		NSNumber *weight = [member objectAtIndex: 2];
		NSNumber *traits = [member objectAtIndex: 3];
		CHECK([name isKindOfClass: [NSString class]] && [name length] > 0);
		CHECK([style isKindOfClass: [NSString class]] && [style length] > 0);
		CHECK([weight isKindOfClass: [NSNumber class]] && [weight intValue] >= 0 && [weight intValue] <= 15);
		CHECK([traits isKindOfClass: [NSNumber class]]);
		if (![name isKindOfClass: [NSString class]] || ![traits isKindOfClass: [NSNumber class]])
			continue;

		NSFontTraitMask mask = [traits unsignedIntValue];
		if ((mask & (NSItalicFontMask | NSBoldFontMask)) == 0 && [weight intValue] == 5)
			hasRegular = YES;
		if (mask & NSBoldFontMask)
			hasBold = YES;

		// The PostScript name makes a font.
		NSFont *font = [NSFont fontWithName: name size: 12];
		if (font == nil)
		{
			printf("FAIL: +[NSFont fontWithName:size:] has no font for %s\n", [name UTF8String]);
			g_failures++;
			continue;
		}
		CHECK([[font fontName] caseInsensitiveCompare: name] == NSOrderedSame);
	}
	if (!hasRegular || !hasBold)
	{
		printf("FAIL: %s lacks a regular or a bold member\n", [family UTF8String]);
		g_failures++;
	}
}

int main(void)
{
	unsetenv("DISPLAY");
	unsetenv("WAYLAND_DISPLAY");

	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSFontManager *manager = [NSFontManager sharedFontManager];

	// Listing needs no window server.
	CHECK([[manager availableFontFamilies] count] > 0);
	CHECK([[manager availableFonts] count] > 0);

	checkFamily(manager, @"Times");
	checkFamily(manager, @"Helvetica");
	checkFamily(manager, @"Arial");
	checkFamily(manager, @"Lucida Grande");

	CHECK([[manager availableMembersOfFontFamily: @"NoSuchFamilyXyz"] count] == 0);

	[pool release];
	if (g_failures != 0)
	{
		printf("appkit_fontmanager_family_members: %d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS appkit_fontmanager_family_members\n");
	return 0;
}
