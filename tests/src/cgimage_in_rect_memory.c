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

/*
 * An image made by CGImageCreateWithImageInRect gives back all of its memory
 * when it is released: making and releasing many of them leaves the
 * process's malloc'ed memory where it was. Each image here is a 1000x900
 * part of a 1000x1000 bitmap context's image (3.6 MB of pixels), made 100
 * times, and its pixels are read once, as a renderer that uploads them does.
 */
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <malloc/malloc.h>
#include <stdio.h>

#define SIDE 1000
#define ROUNDS 100
// Growth allowed over all rounds: well below one image per round.
#define MAX_GROWTH (32u << 20)

static size_t bytesInUse(void)
{
	malloc_statistics_t stats;
	malloc_zone_statistics(NULL, &stats);
	return stats.size_in_use;
}

static int makeAndRelease(CGImageRef whole)
{
	CGImageRef part = CGImageCreateWithImageInRect(whole, CGRectMake(0, 0, SIDE, SIDE - 100));
	if (part == NULL)
		return 0;
	int ok = CGImageGetWidth(part) == SIDE && CGImageGetHeight(part) == SIDE - 100;
	CFDataRef pixels = CGDataProviderCopyData(CGImageGetDataProvider(part));
	if (pixels == NULL)
		ok = 0;
	else
		CFRelease(pixels);
	CGImageRelease(part);
	return ok;
}

int main(void)
{
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef context = CGBitmapContextCreate(NULL, SIDE, SIDE, 8, SIDE * 4, space,
	                                             kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
	CGColorSpaceRelease(space);
	if (context == NULL) {
		printf("FAIL: no bitmap context\n");
		return 1;
	}
	CGContextSetRGBFillColor(context, 0.2, 0.4, 0.6, 1);
	CGContextFillRect(context, CGRectMake(0, 0, SIDE, SIDE));
	CGImageRef whole = CGBitmapContextCreateImage(context);
	if (whole == NULL) {
		printf("FAIL: no image of the context\n");
		return 1;
	}

	// The first round may set up caches that stay.
	if (!makeAndRelease(whole)) {
		printf("FAIL: CGImageCreateWithImageInRect gave no usable image\n");
		return 1;
	}
	size_t before = bytesInUse();
	for (int i = 0; i < ROUNDS; i++) {
		if (!makeAndRelease(whole)) {
			printf("FAIL: CGImageCreateWithImageInRect gave no usable image in round %d\n", i);
			return 1;
		}
	}
	size_t after = bytesInUse();

	CGImageRelease(whole);
	CGContextRelease(context);

	long growth = (long) after - (long) before;
	printf("%d images: malloc'ed memory %zu -> %zu bytes\n", ROUNDS, before, after);
	if (growth > (long) MAX_GROWTH) {
		printf("FAIL: %ld bytes were not given back\n", growth);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
