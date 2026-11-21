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

// HITheme metrics and brushes: GetThemeMetric for every public kThemeMetric
// constant, the theme brush colours and the small Appearance Manager queries.
// The expected values were read from macOS 10.14.4 through the public API with
// vd-upstream/mac-probes/V655-hitheme.c (macOS 26.7.3 reports the same values
// except for kThemeMetricMenuMarkColumnWidth, kThemeMetricMenuExcludedMarkColumnWidth,
// kThemeMetricMenuMarkIndent, kThemeMetricMenuTextTrailingEdgeMargin and
// kThemeMetricMenuIconTrailingEdgeMargin; the older values are used).

#include <Carbon/Carbon.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// kThemeMetric* constants 0...137 in order.
static const SInt16 kExpectedMetrics[] = {
	15, 11, 18, 18, 2, 1, 1, 4, 3, 0, 23, 12,
	3, -1, 1, 19, 10, 20, 20, 20, 17, 16, 21, 18,
	19, 13, 13, 21, 13, 9, 20, 17, 16, 20, 17, 17,
	15, 10, 15, 11, 25, 22, 5, 15, 4, 22, 5, 15,
	4, 13, 18, 14, 18, 14, 13, 14, 6, 6, 12, 4,
	2, 1, 1, 21, 5, 5, 0, 19, 12, 4, 2, 2,
	1, 2, 17, 14, 13, 13, 2, 3, 17, 0, 15, 0,
	15, 11, 18, 12, 11, 10, 15, 19, 12, 11, 4, 5,
	15, 15, 16, 11, 10, 8, 2, 15, 0, 11, 5, 4,
	12, 25, 23, 10, 2, 11, 11, 3, 21, 16, 14, 2,
	12, 12, 3, 24, 20, 2, 0, 7, 9, 9, 19, 20,
	26, 26, 20, 20, 18, 17,
};

static const struct {
	SInt16 brush;
	int components;
	double values[4];
	SInt16 textActive, textInactive;   // 0: no text colour
} kExpectedBrushes[] = {
	{ -5, 4, { 0.0348f, 0.3152f, 0.8149f, 1.0f }, 0, 0 },
	{ -4, 4, { 0.8157f, 0.8157f, 0.8157f, 1.0f }, 0, 0 },
	{ -3, 4, { 0.698f, 0.843f, 1.0f, 1.0f }, 0, 0 },
	{ -2, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ -1, 4, { 0.0f, 0.0f, 0.0f, 1.0f }, 0, 0 },
	{ 1, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 1, 2 },
	{ 2, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 1, 2 },
	{ 3, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 3, 4 },
	{ 4, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 3, 4 },
	{ 5, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 5, 6 },
	{ 6, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 5, 6 },
	{ 7, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 0, 0 },
	{ 8, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 0, 0 },
	{ 9, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 10, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 11, 4, { 0.98f, 0.99f, 0.99f, 1.0f }, 0, 0 },
	{ 12, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 13, 4, { 0.0f, 0.0f, 0.0f, 1.0f }, 0, 0 },
	{ 14, 4, { 0.698f, 0.843f, 1.0f, 1.0f }, 0, 0 },
	{ 15, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 16, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 17, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 18, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 19, 4, { 0.4922f, 0.6758f, 0.8477f, 1.0f }, 0, 0 },
	{ 20, 4, { 0.0f, 0.0f, 0.0f, 1.0f }, 0, 0 },
	{ 21, 4, { 0.0f, 0.0f, 0.0f, 1.0f }, 0, 0 },
	{ 22, 4, { 0.5f, 0.5f, 0.5f, 1.0f }, 0, 0 },
	{ 23, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 24, 4, { 0.6f, 0.6f, 0.6f, 1.0f }, 0, 0 },
	{ 25, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 26, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 27, 4, { 0.51f, 0.51f, 0.51f, 1.0f }, 0, 0 },
	{ 28, 4, { 0.76f, 0.76f, 0.76f, 1.0f }, 0, 0 },
	{ 29, 4, { 0.94f, 0.94f, 0.94f, 1.0f }, 0, 0 },
	{ 30, 4, { 0.98f, 0.98f, 0.98f, 1.0f }, 0, 0 },
	{ 31, 4, { 0.67f, 0.67f, 0.67f, 1.0f }, 0, 0 },
	{ 32, 4, { 0.86f, 0.86f, 0.86f, 1.0f }, 0, 0 },
	{ 33, 4, { 0.88f, 0.88f, 0.88f, 1.0f }, 0, 0 },
	{ 34, 4, { 0.88f, 0.88f, 0.88f, 1.0f }, 0, 0 },
	{ 35, 4, { 0.99f, 0.99f, 0.99f, 1.0f }, 0, 0 },
	{ 36, 4, { 0.9f, 0.9f, 0.9f, 1.0f }, 0, 0 },
	{ 37, 4, { 0.93f, 0.93f, 0.93f, 1.0f }, 0, 0 },
	{ 38, 4, { 0.93f, 0.93f, 0.93f, 1.0f }, 0, 0 },
	{ 39, 4, { 0.99f, 0.99f, 0.99f, 1.0f }, 0, 0 },
	{ 40, 4, { 0.54f, 0.54f, 0.54f, 1.0f }, 0, 0 },
	{ 41, 4, { 0.59f, 0.59f, 0.59f, 1.0f }, 0, 0 },
	{ 42, 4, { 0.59f, 0.59f, 0.59f, 1.0f }, 0, 0 },
	{ 43, 4, { 0.73f, 0.73f, 0.73f, 1.0f }, 0, 0 },
	{ 44, 4, { 0.64f, 0.64f, 0.64f, 1.0f }, 0, 0 },
	{ 45, 4, { 0.64f, 0.64f, 0.64f, 1.0f }, 0, 0 },
	{ 46, 4, { 0.82f, 0.82f, 0.82f, 1.0f }, 0, 0 },
	{ 47, 4, { 0.82f, 0.82f, 0.82f, 1.0f }, 0, 0 },
	{ 48, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 47, 47 },
	{ 49, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 0, 0 },
	{ 50, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 0, 0 },
	{ 51, 2, { 0.9098f, 1.0f }, 0, 0 },
	{ 52, 4, { 0.9255f, 0.9255f, 0.9255f, 1.0f }, 0, 0 },
	{ 53, 2, { 0.9098f, 0.95f }, 0, 0 },
	{ 54, 2, { 1.0f, 0.95f }, 0, 0 },
	{ 55, 4, { 0.2078f, 0.4431f, 0.8039f, 0.95f }, 0, 0 },
	{ 56, 4, { 1.0f, 1.0f, 1.0f, 1.0f }, 0, 0 },
	{ 57, 4, { 0.9608f, 0.9608f, 0.9608f, 1.0f }, 0, 0 },
	{ 58, 4, { 0.0f, 0.0f, 0.0f, 0.302f }, 0, 0 },
};

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

static void test_metrics(void)
{
	for (UInt32 m = 0; m < COUNT(kExpectedMetrics); m++)
	{
		SInt32 value = -12345;
		OSStatus st = GetThemeMetric(m, &value);
		if (st != noErr || value != kExpectedMetrics[m])
		{
			printf("FAIL metric %u: status %d value %d, expected %d\n", (unsigned) m, (int) st, (int) value, kExpectedMetrics[m]);
			g_failures++;
		}
	}
	// named constants of the public header, at the values of the table
	SInt32 v;
	CHECK(GetThemeMetric(kThemeMetricScrollBarWidth, &v) == noErr && v == 15);
	CHECK(GetThemeMetric(kThemeMetricSmallScrollBarWidth, &v) == noErr && v == 11);
	CHECK(GetThemeMetric(kThemeMetricCheckBoxHeight, &v) == noErr && v == 18);
	CHECK(GetThemeMetric(kThemeMetricRadioButtonHeight, &v) == noErr && v == 18);
	CHECK(GetThemeMetric(kThemeMetricPushButtonHeight, &v) == noErr && v == 20);
	CHECK(GetThemeMetric(kThemeMetricSmallPushButtonHeight, &v) == noErr && v == 17);
	CHECK(GetThemeMetric(kThemeMetricMiniPushButtonHeight, &v) == noErr && v == 16);
	CHECK(GetThemeMetric(kThemeMetricPopupButtonHeight, &v) == noErr && v == 20);
	CHECK(GetThemeMetric(kThemeMetricListHeaderHeight, &v) == noErr && v == 17);
	CHECK(GetThemeMetric(kThemeMetricHSliderHeight, &v) == noErr && v == 22);
	CHECK(GetThemeMetric(kThemeMetricNormalProgressBarThickness, &v) == noErr && v == 12);
	CHECK(GetThemeMetric(kThemeMetricLargeProgressBarThickness, &v) == noErr && v == 16);
	CHECK(GetThemeMetric(kThemeMetricResizeControlHeight, &v) == noErr && v == 15);
	CHECK(GetThemeMetric(kThemeMetricMenuMarkColumnWidth, &v) == noErr && v == 21);
	// aliases
	CHECK(GetThemeMetric(kThemeMetricCheckBoxGlyphHeight, &v) == noErr && v == 18);
	CHECK(GetThemeMetric(kThemeMetricBestListHeaderHeight, &v) == noErr && v == 17);
	CHECK(GetThemeMetric(kThemeMetricProgressBarThickness, &v) == noErr && v == 16);
}

static void test_metric_errors(void)
{
	SInt32 v = 77;
	CHECK(GetThemeMetric(0, NULL) == paramErr);
	CHECK(GetThemeMetric(kThemeMetricButtonRoundedRecessedHeight + 1000, &v) == paramErr);
	CHECK(v == 77);
	CHECK(GetThemeMetric(0xffffffffu, &v) == paramErr);
}

static void test_brushes(void)
{
	for (UInt32 i = 0; i < COUNT(kExpectedBrushes); i++)
	{
		CGColorRef color = NULL;
		OSStatus st = HIThemeBrushCreateCGColor(kExpectedBrushes[i].brush, &color);
		if (st != noErr || color == NULL)
		{
			printf("FAIL brush %d: status %d\n", kExpectedBrushes[i].brush, (int) st);
			g_failures++;
			continue;
		}
		size_t n = CGColorGetNumberOfComponents(color);
		const CGFloat *c = CGColorGetComponents(color);
		if ((int) n != kExpectedBrushes[i].components)
		{
			printf("FAIL brush %d: %d components, expected %d\n", kExpectedBrushes[i].brush, (int) n, kExpectedBrushes[i].components);
			g_failures++;
		}
		else
		{
			for (int k = 0; k < kExpectedBrushes[i].components; k++)
			{
				if (fabs(c[k] - kExpectedBrushes[i].values[k]) > 0.002)
				{
					printf("FAIL brush %d component %d: %g, expected %g\n", kExpectedBrushes[i].brush, k, c[k], kExpectedBrushes[i].values[k]);
					g_failures++;
				}
			}
		}
		CGColorRelease(color);

		ThemeTextColor tc = 99;
		st = HIThemeGetTextColorForThemeBrush(kExpectedBrushes[i].brush, true, &tc);
		if (kExpectedBrushes[i].textActive != 0)
			CHECK(st == noErr && tc == kExpectedBrushes[i].textActive);
		else
			CHECK(st != noErr);
		st = HIThemeGetTextColorForThemeBrush(kExpectedBrushes[i].brush, false, &tc);
		if (kExpectedBrushes[i].textInactive != 0)
			CHECK(st == noErr && tc == kExpectedBrushes[i].textInactive);
		else
			CHECK(st != noErr);
	}
	// well known brushes by name
	CGColorRef c = NULL;
	CHECK(HIThemeBrushCreateCGColor(kThemeBrushBlack, &c) == noErr && c != NULL);
	if (c != NULL)
	{
		const CGFloat *comp = CGColorGetComponents(c);
		CHECK(comp[0] == 0 && comp[1] == 0 && comp[2] == 0 && comp[3] == 1);
		CGColorRelease(c);
	}
	CHECK(HIThemeBrushCreateCGColor(kThemeBrushWhite, &c) == noErr && c != NULL);
	if (c != NULL)
	{
		const CGFloat *comp = CGColorGetComponents(c);
		CHECK(comp[0] == 1 && comp[1] == 1 && comp[2] == 1 && comp[3] == 1);
		CGColorRelease(c);
	}
}

// The error codes are not documented; they were observed on macOS 10.14.4 and 26.7.3.
static void hitheme_quirk_brush_error_codes(void)
{
	CGColorRef c = (CGColorRef) 1;
	CHECK(HIThemeBrushCreateCGColor(0, &c) == paramErr);
	CHECK(c == NULL);
	CHECK(HIThemeBrushCreateCGColor(9999, &c) == -192);
	CHECK(HIThemeBrushCreateCGColor(-6, &c) == -192);
	CHECK(HIThemeBrushCreateCGColor(kThemeBrushBlack, NULL) == paramErr);
	ThemeTextColor tc = 5;
	CHECK(HIThemeGetTextColorForThemeBrush(kThemeBrushBlack, true, &tc) == -30568);
	CHECK(HIThemeGetTextColorForThemeBrush(9999, true, &tc) == -30568);
	CHECK(HIThemeGetTextColorForThemeBrush(kThemeBrushDialogBackgroundActive, true, NULL) == paramErr);
}

static void test_context_brushes(void)
{
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGContextRef ctx = CGBitmapContextCreate(NULL, 4, 4, 8, 16, cs, kCGImageAlphaPremultipliedLast);
	CGColorSpaceRelease(cs);
	CHECK(ctx != NULL);
	if (ctx == NULL)
		return;
	CHECK(HIThemeSetFill(kThemeBrushWhite, NULL, ctx, kHIThemeOrientationInverted) == noErr);
	CHECK(HIThemeSetStroke(kThemeBrushBlack, NULL, ctx, kHIThemeOrientationInverted) == noErr);
	CHECK(HIThemeSetTextFill(kThemeTextColorBlack, NULL, ctx, kHIThemeOrientationInverted) == noErr);
	CHECK(HIThemeSetFill(9999, NULL, ctx, kHIThemeOrientationInverted) == -192);
	CHECK(HIThemeSetFill(kThemeBrushWhite, NULL, NULL, kHIThemeOrientationInverted) == paramErr);
	CHECK(HIThemeSetTextFill(1234, NULL, ctx, kHIThemeOrientationInverted) == paramErr);
	// fill with the white brush, then read the pixel back
	HIThemeSetFill(kThemeBrushWhite, NULL, ctx, kHIThemeOrientationInverted);
	CGContextFillRect(ctx, CGRectMake(0, 0, 4, 4));
	const unsigned char *px = CGBitmapContextGetData(ctx);
	CHECK(px != NULL && px[0] == 255 && px[1] == 255 && px[2] == 255 && px[3] == 255);
	CGContextRelease(ctx);
}

static void test_small_queries(void)
{
	SInt16 h = -1, w = -1;
	CHECK(GetThemeMenuSeparatorHeight(&h) == noErr && h == 12);
	CHECK(GetThemeMenuSeparatorHeight(NULL) == paramErr);
	for (int type = 0; type < 4; type++)
	{
		h = w = -1;
		CHECK(GetThemeMenuItemExtra(type, &h, &w) == noErr);
		CHECK(h == 2);
		CHECK(w == (type == kThemeMenuItemHierarchical ? 39 : 0));
	}
	CHECK(GetThemeMenuItemExtra(kThemeMenuItemPlain, NULL, &w) == paramErr);
	CHECK(GetThemeMenuTitleExtra(&w, false) == noErr && w == 18);
	CHECK(GetThemeMenuTitleExtra(&w, true) == noErr && w == 10);
	CHECK(GetThemeMenuTitleExtra(NULL, true) == paramErr);

	CFStringRef ident = NULL;
	CHECK(CopyThemeIdentifier(&ident) == noErr && ident != NULL);
	if (ident != NULL)
	{
		CHECK(CFEqual(ident, CFSTR("com.apple.theme.appearance.aqua.blue")));
		CFRelease(ident);
	}
	CHECK(CopyThemeIdentifier(NULL) == paramErr);

	// kThemeFont constants to the CoreText UI font types
	static const struct { int theme, type; } fonts[] = {
		{ 0, 2 }, { 1, 4 }, { 2, 5 }, { 3, 8 }, { 4, 3 }, { 5, 9 }, { 6, 10 }, { 7, 19 }, { 8, 20 },
		{ 100, 11 }, { 101, 12 }, { 102, 13 }, { 103, 14 }, { 104, 15 }, { 105, 16 }, { 106, 17 },
		{ 107, 18 }, { 108, 21 }, { 109, 6 }, { 200, -1 }, { 201, -1 }, { 999, -1 },
	};
	for (unsigned i = 0; i < COUNT(fonts); i++)
	{
		if ((int) HIThemeGetUIFontType(fonts[i].theme) != fonts[i].type)
		{
			printf("FAIL ui font of theme font %d: %d, expected %d\n", fonts[i].theme, (int) HIThemeGetUIFontType(fonts[i].theme), fonts[i].type);
			g_failures++;
		}
	}
}

int main(void)
{
	test_metrics();
	test_metric_errors();
	test_brushes();
	hitheme_quirk_brush_error_codes();
	test_context_brushes();
	test_small_queries();
	if (g_failures != 0)
	{
		printf("hitheme_metrics: %d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS hitheme_metrics\n");
	return 0;
}
