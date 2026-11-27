/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * CGImagePropertyOrientation uses the TIFF/EXIF orientation numbering, the
 * same values ImageIO stores under kCGImagePropertyOrientation: 1 up,
 * 2 up mirrored, 3 down, 4 down mirrored, 5 left mirrored, 6 right,
 * 7 right mirrored, 8 left (CIPA DC-008 EXIF 2.3, table of Orientation tag
 * values; TIFF 6.0 for the base values). Including the header twice must work.
 */
#include <ImageIO/CGImageProperties.h>
#include <ImageIO/CGImageProperties.h>
#include <stdio.h>

static int failures;

static void check(const char *what, long value, long expected)
{
	printf("%s: %s (%ld)\n", value == expected ? "PASS" : "FAIL", what, value);
	if (value != expected)
		failures++;
}

int main(void)
{
	check("Up", kCGImagePropertyOrientationUp, 1);
	check("UpMirrored", kCGImagePropertyOrientationUpMirrored, 2);
	check("Down", kCGImagePropertyOrientationDown, 3);
	check("DownMirrored", kCGImagePropertyOrientationDownMirrored, 4);
	check("LeftMirrored", kCGImagePropertyOrientationLeftMirrored, 5);
	check("Right", kCGImagePropertyOrientationRight, 6);
	check("RightMirrored", kCGImagePropertyOrientationRightMirrored, 7);
	check("Left", kCGImagePropertyOrientationLeft, 8);
	check("type is 32-bit unsigned", sizeof(CGImagePropertyOrientation), 4);
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
