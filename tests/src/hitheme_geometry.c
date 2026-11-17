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

// HITheme geometry queries: button content and background bounds for every
// button kind, shape queries of tabs, menus, windows and the grow box, scroll
// bar and slider geometry. The HIShape values they return are covered by hishape.c.
// The expected values were read from macOS 10.14.4 through the public API with
// vd-upstream/mac-probes/V655-hitheme.c. The button table lists the rectangles
// for an input rectangle at (20, 20); rows flagged 1 (content) or 2 (background)
// are listed for completeness but are known not to match. These are inputs
// narrower than the margins of the kind (10 or 20 wide), where macOS enforces
// a minimum width or flips the sign of the width in a way that differs from
// kind to kind and, for the push button kinds, between the small and normal
// appearance. The same size gives different results across kinds and the rule
// is not consistent, so it is not modelled. Heights below the normal
// appearance are modelled (HIThemeData.h, kButtonHeightOverrides).

#include <Carbon/Carbon.h>
#include <stdio.h>
#include <string.h>

static int g_failures;
static int g_known_deviations;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

static const struct {
	int kind, w, h;
	int content[4], background[4];
	int known;   // 1: content row not reproduced, 2: background row not reproduced
} kButtonRows[] = {
	{ 0, 10, 10, { 25, 21, 0, 17 }, { 14, 17, 32, 21 }, 3 },
	{ 0, 20, 20, { 28, 20, 0, 20 }, { 14, 16, 32, 32 }, 0 },
	{ 0, 60, 60, { 34, 20, 32, 60 }, { 14, 16, 72, 72 }, 0 },
	{ 0, 100, 10, { 32, 21, 76, 17 }, { 14, 17, 112, 21 }, 0 },
	{ 0, 100, 18, { 32, 21, 76, 17 }, { 14, 17, 112, 28 }, 0 },
	{ 0, 100, 24, { 34, 20, 72, 24 }, { 14, 16, 112, 36 }, 0 },
	{ 0, 100, 40, { 34, 20, 72, 40 }, { 14, 16, 112, 52 }, 0 },
	{ 0, 130, 70, { 34, 20, 102, 70 }, { 14, 16, 142, 82 }, 0 },
	{ 1, 10, 10, { 30, 20, 0, 10 }, { 20, 16, 10, 18 }, 0 },
	{ 1, 20, 20, { 40, 20, 0, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 1, 60, 60, { 40, 20, 40, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 1, 100, 10, { 40, 20, 80, 10 }, { 20, 16, 100, 18 }, 0 },
	{ 1, 100, 18, { 40, 20, 80, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 1, 100, 24, { 40, 20, 80, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 1, 100, 40, { 40, 20, 80, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 1, 130, 70, { 40, 20, 110, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 2, 10, 10, { 30, 20, 0, 10 }, { 20, 16, 10, 18 }, 0 },
	{ 2, 20, 20, { 40, 20, 0, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 2, 60, 60, { 40, 20, 40, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 2, 100, 10, { 40, 20, 80, 10 }, { 20, 16, 100, 18 }, 0 },
	{ 2, 100, 18, { 40, 20, 80, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 2, 100, 24, { 40, 20, 80, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 2, 100, 40, { 40, 20, 80, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 2, 130, 70, { 40, 20, 110, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 3, 10, 10, { 23, 23, 4, 4 }, { 20, 20, 10, 10 }, 0 },
	{ 3, 20, 20, { 23, 23, 14, 14 }, { 20, 20, 20, 20 }, 0 },
	{ 3, 60, 60, { 23, 23, 54, 54 }, { 20, 20, 60, 60 }, 0 },
	{ 3, 100, 10, { 23, 23, 94, 4 }, { 20, 20, 100, 10 }, 0 },
	{ 3, 100, 18, { 23, 23, 94, 12 }, { 20, 20, 100, 18 }, 0 },
	{ 3, 100, 24, { 23, 23, 94, 18 }, { 20, 20, 100, 24 }, 0 },
	{ 3, 100, 40, { 23, 23, 94, 34 }, { 20, 20, 100, 40 }, 0 },
	{ 3, 130, 70, { 23, 23, 124, 64 }, { 20, 20, 130, 70 }, 0 },
	{ 4, 10, 10, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 20, 20, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 60, 60, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 100, 10, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 100, 18, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 100, 24, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 100, 40, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 4, 130, 70, { 0, 0, 0, 0 }, { 17, 18, 27, 26 }, 0 },
	{ 5, 10, 10, { 27, 22, 0, 6 }, { 17, 19, 16, 15 }, 0 },
	{ 5, 20, 20, { 29, 22, 0, 16 }, { 18, 18, 25, 26 }, 0 },
	{ 5, 60, 60, { 29, 22, 21, 56 }, { 18, 18, 65, 66 }, 0 },
	{ 5, 100, 10, { 27, 22, 68, 6 }, { 17, 19, 106, 15 }, 0 },
	{ 5, 100, 18, { 27, 22, 68, 14 }, { 17, 19, 106, 23 }, 0 },
	{ 5, 100, 24, { 29, 22, 61, 20 }, { 18, 18, 105, 30 }, 0 },
	{ 5, 100, 40, { 29, 22, 61, 36 }, { 18, 18, 105, 46 }, 0 },
	{ 5, 130, 70, { 29, 22, 91, 66 }, { 18, 18, 135, 76 }, 0 },
	{ 6, 10, 10, { 30, 20, 0, 10 }, { 20, 19, 13, 13 }, 0 },
	{ 6, 20, 20, { 33, 20, 7, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 6, 60, 60, { 33, 20, 47, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 6, 100, 10, { 33, 20, 87, 10 }, { 20, 19, 100, 13 }, 0 },
	{ 6, 100, 18, { 33, 20, 87, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 6, 100, 24, { 33, 20, 87, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 6, 100, 40, { 33, 20, 87, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 6, 130, 70, { 33, 20, 117, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 7, 10, 10, { 0, 0, 0, 0 }, { 17, 12, 19, 27 }, 0 },
	{ 7, 20, 20, { 0, 0, 0, 0 }, { 17, 17, 21, 27 }, 0 },
	{ 7, 60, 60, { 0, 0, 0, 0 }, { 17, 20, 61, 60 }, 0 },
	{ 7, 100, 10, { 0, 0, 0, 0 }, { 17, 12, 101, 27 }, 0 },
	{ 7, 100, 18, { 0, 0, 0, 0 }, { 17, 16, 101, 27 }, 0 },
	{ 7, 100, 24, { 0, 0, 0, 0 }, { 17, 19, 101, 27 }, 0 },
	{ 7, 100, 40, { 0, 0, 0, 0 }, { 17, 20, 101, 40 }, 0 },
	{ 7, 130, 70, { 0, 0, 0, 0 }, { 17, 20, 131, 70 }, 0 },
	{ 8, 10, 10, { 22, 22, 6, 6 }, { 20, 20, 10, 10 }, 0 },
	{ 8, 20, 20, { 22, 22, 16, 16 }, { 20, 20, 20, 20 }, 0 },
	{ 8, 60, 60, { 22, 22, 56, 56 }, { 20, 20, 60, 60 }, 0 },
	{ 8, 100, 10, { 22, 22, 96, 6 }, { 20, 20, 100, 10 }, 0 },
	{ 8, 100, 18, { 22, 22, 96, 14 }, { 20, 20, 100, 18 }, 0 },
	{ 8, 100, 24, { 22, 22, 96, 20 }, { 20, 20, 100, 24 }, 0 },
	{ 8, 100, 40, { 22, 22, 96, 36 }, { 20, 20, 100, 40 }, 0 },
	{ 8, 130, 70, { 22, 22, 126, 66 }, { 20, 20, 130, 70 }, 0 },
	{ 9, 10, 10, { 24, 24, 2, 2 }, { 20, 20, 10, 10 }, 0 },
	{ 9, 20, 20, { 24, 24, 12, 12 }, { 20, 20, 20, 20 }, 0 },
	{ 9, 60, 60, { 24, 24, 52, 52 }, { 20, 20, 60, 60 }, 0 },
	{ 9, 100, 10, { 24, 24, 92, 2 }, { 20, 20, 100, 10 }, 0 },
	{ 9, 100, 18, { 24, 24, 92, 10 }, { 20, 20, 100, 18 }, 0 },
	{ 9, 100, 24, { 24, 24, 92, 16 }, { 20, 20, 100, 24 }, 0 },
	{ 9, 100, 40, { 24, 24, 92, 32 }, { 20, 20, 100, 40 }, 0 },
	{ 9, 130, 70, { 24, 24, 122, 62 }, { 20, 20, 130, 70 }, 0 },
	{ 10, 10, 10, { 30, 20, 12, 16 }, { 20, 20, 22, 17 }, 3 },
	{ 10, 20, 20, { 32, 20, 12, 16 }, { 20, 20, 24, 17 }, 3 },
	{ 10, 60, 60, { 32, 20, 48, 16 }, { 20, 20, 60, 17 }, 0 },
	{ 10, 100, 10, { 32, 20, 88, 16 }, { 20, 20, 100, 17 }, 0 },
	{ 10, 100, 18, { 32, 20, 88, 16 }, { 20, 20, 100, 17 }, 0 },
	{ 10, 100, 24, { 32, 20, 88, 16 }, { 20, 20, 100, 17 }, 0 },
	{ 10, 100, 40, { 32, 20, 88, 16 }, { 20, 20, 100, 17 }, 0 },
	{ 10, 130, 70, { 32, 20, 118, 16 }, { 20, 20, 130, 17 }, 0 },
	{ 11, 10, 10, { 20, 20, 10, 10 }, { 9, 11, 32, 31 }, 0 },
	{ 11, 20, 20, { 20, 20, 20, 20 }, { 14, 16, 32, 31 }, 0 },
	{ 11, 60, 60, { 20, 20, 60, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 11, 100, 10, { 20, 20, 100, 10 }, { 20, 11, 100, 31 }, 0 },
	{ 11, 100, 18, { 20, 20, 100, 18 }, { 20, 15, 100, 31 }, 0 },
	{ 11, 100, 24, { 20, 20, 100, 24 }, { 20, 18, 100, 31 }, 0 },
	{ 11, 100, 40, { 20, 20, 100, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 11, 130, 70, { 20, 20, 130, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 12, 10, 10, { 20, 20, 10, 10 }, { 6, 9, 39, 38 }, 2 },
	{ 12, 20, 20, { 20, 20, 20, 20 }, { 11, 14, 39, 38 }, 2 },
	{ 12, 60, 60, { 20, 20, 60, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 12, 100, 10, { 20, 20, 100, 10 }, { 20, 9, 100, 38 }, 0 },
	{ 12, 100, 18, { 20, 20, 100, 18 }, { 20, 13, 100, 38 }, 0 },
	{ 12, 100, 24, { 20, 20, 100, 24 }, { 20, 16, 100, 38 }, 0 },
	{ 12, 100, 40, { 20, 20, 100, 40 }, { 20, 20, 100, 42 }, 0 },
	{ 12, 130, 70, { 20, 20, 130, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 13, 10, 10, { 30, 20, 0, 10 }, { 20, 17, 10, 16 }, 0 },
	{ 13, 20, 20, { 39, 20, 1, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 13, 60, 60, { 39, 20, 41, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 13, 100, 10, { 39, 20, 81, 10 }, { 20, 17, 100, 16 }, 0 },
	{ 13, 100, 18, { 39, 20, 81, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 13, 100, 24, { 39, 20, 81, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 13, 100, 40, { 39, 20, 81, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 13, 130, 70, { 39, 20, 111, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 14, 10, 10, { 30, 20, 0, 10 }, { 20, 18, 14, 15 }, 0 },
	{ 14, 20, 20, { 38, 20, 2, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 14, 60, 60, { 38, 20, 42, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 14, 100, 10, { 38, 20, 82, 10 }, { 20, 18, 100, 15 }, 0 },
	{ 14, 100, 18, { 38, 20, 82, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 14, 100, 24, { 38, 20, 82, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 14, 100, 40, { 38, 20, 82, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 14, 130, 70, { 38, 20, 112, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 15, 10, 10, { 23, 23, 4, 4 }, { 18, 18, 14, 15 }, 0 },
	{ 15, 20, 20, { 23, 23, 14, 14 }, { 18, 18, 24, 25 }, 0 },
	{ 15, 60, 60, { 23, 23, 54, 54 }, { 18, 18, 64, 65 }, 0 },
	{ 15, 100, 10, { 23, 23, 94, 4 }, { 18, 18, 104, 15 }, 0 },
	{ 15, 100, 18, { 23, 23, 94, 12 }, { 18, 18, 104, 23 }, 0 },
	{ 15, 100, 24, { 23, 23, 94, 18 }, { 18, 18, 104, 29 }, 0 },
	{ 15, 100, 40, { 23, 23, 94, 34 }, { 18, 18, 104, 45 }, 0 },
	{ 15, 130, 70, { 23, 23, 124, 64 }, { 18, 18, 134, 75 }, 0 },
	{ 16, 10, 10, { 0, 0, 0, 0 }, { 10, 20, 26, 23 }, 0 },
	{ 16, 20, 20, { 0, 0, 0, 0 }, { 20, 20, 22, 23 }, 0 },
	{ 16, 60, 60, { 0, 0, 0, 0 }, { 20, 20, 62, 23 }, 0 },
	{ 16, 100, 10, { 0, 0, 0, 0 }, { 20, 20, 102, 23 }, 0 },
	{ 16, 100, 18, { 0, 0, 0, 0 }, { 20, 20, 102, 23 }, 0 },
	{ 16, 100, 24, { 0, 0, 0, 0 }, { 20, 20, 102, 23 }, 0 },
	{ 16, 100, 40, { 0, 0, 0, 0 }, { 20, 20, 102, 23 }, 0 },
	{ 16, 130, 70, { 0, 0, 0, 0 }, { 20, 20, 132, 23 }, 0 },
	{ 17, 10, 10, { 0, 0, 0, 0 }, { 13, 20, 20, 19 }, 0 },
	{ 17, 20, 20, { 0, 0, 0, 0 }, { 20, 20, 22, 19 }, 0 },
	{ 17, 60, 60, { 0, 0, 0, 0 }, { 20, 20, 62, 19 }, 0 },
	{ 17, 100, 10, { 0, 0, 0, 0 }, { 20, 20, 102, 19 }, 0 },
	{ 17, 100, 18, { 0, 0, 0, 0 }, { 20, 20, 102, 19 }, 0 },
	{ 17, 100, 24, { 0, 0, 0, 0 }, { 20, 20, 102, 19 }, 0 },
	{ 17, 100, 40, { 0, 0, 0, 0 }, { 20, 20, 102, 19 }, 0 },
	{ 17, 130, 70, { 0, 0, 0, 0 }, { 20, 20, 132, 19 }, 0 },
	{ 18, 10, 10, { 0, 0, 0, 0 }, { 12, 20, 22, 15 }, 0 },
	{ 18, 20, 20, { 0, 0, 0, 0 }, { 20, 20, 20, 15 }, 0 },
	{ 18, 60, 60, { 0, 0, 0, 0 }, { 20, 20, 60, 15 }, 0 },
	{ 18, 100, 10, { 0, 0, 0, 0 }, { 20, 20, 100, 15 }, 0 },
	{ 18, 100, 18, { 0, 0, 0, 0 }, { 20, 20, 100, 15 }, 0 },
	{ 18, 100, 24, { 0, 0, 0, 0 }, { 20, 20, 100, 15 }, 0 },
	{ 18, 100, 40, { 0, 0, 0, 0 }, { 20, 20, 100, 15 }, 0 },
	{ 18, 130, 70, { 0, 0, 0, 0 }, { 20, 20, 130, 15 }, 0 },
	{ 19, 10, 10, { 33, 20, 3, 10 }, { 20, 19, 16, 11 }, 0 },
	{ 19, 20, 20, { 33, 20, 7, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 19, 60, 60, { 33, 20, 47, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 19, 100, 10, { 33, 20, 87, 10 }, { 20, 19, 100, 11 }, 0 },
	{ 19, 100, 18, { 33, 20, 87, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 19, 100, 24, { 33, 20, 87, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 19, 100, 40, { 33, 20, 87, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 19, 130, 70, { 33, 20, 117, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 20, 10, 10, { 33, 20, 3, 10 }, { 20, 19, 16, 11 }, 0 },
	{ 20, 20, 20, { 33, 20, 7, 20 }, { 20, 20, 20, 20 }, 0 },
	{ 20, 60, 60, { 33, 20, 47, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 20, 100, 10, { 33, 20, 87, 10 }, { 20, 19, 100, 11 }, 0 },
	{ 20, 100, 18, { 33, 20, 87, 18 }, { 20, 20, 100, 18 }, 0 },
	{ 20, 100, 24, { 33, 20, 87, 24 }, { 20, 20, 100, 24 }, 0 },
	{ 20, 100, 40, { 33, 20, 87, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 20, 130, 70, { 33, 20, 117, 70 }, { 20, 20, 130, 70 }, 0 },
	{ 21, 10, 10, { 0, 0, 0, 0 }, { 20, 14, 15, 22 }, 0 },
	{ 21, 20, 20, { 0, 0, 0, 0 }, { 20, 19, 15, 22 }, 0 },
	{ 21, 60, 60, { 0, 0, 0, 0 }, { 20, 39, 15, 22 }, 0 },
	{ 21, 100, 10, { 0, 0, 0, 0 }, { 20, 14, 15, 22 }, 0 },
	{ 21, 100, 18, { 0, 0, 0, 0 }, { 20, 18, 15, 22 }, 0 },
	{ 21, 100, 24, { 0, 0, 0, 0 }, { 20, 21, 15, 22 }, 0 },
	{ 21, 100, 40, { 0, 0, 0, 0 }, { 20, 29, 15, 22 }, 0 },
	{ 21, 130, 70, { 0, 0, 0, 0 }, { 20, 44, 15, 22 }, 0 },
	{ 22, 10, 10, { 0, 0, 0, 0 }, { 20, 18, 13, 15 }, 0 },
	{ 22, 20, 20, { 0, 0, 0, 0 }, { 20, 23, 13, 15 }, 0 },
	{ 22, 60, 60, { 0, 0, 0, 0 }, { 20, 43, 13, 15 }, 0 },
	{ 22, 100, 10, { 0, 0, 0, 0 }, { 20, 18, 13, 15 }, 0 },
	{ 22, 100, 18, { 0, 0, 0, 0 }, { 20, 22, 13, 15 }, 0 },
	{ 22, 100, 24, { 0, 0, 0, 0 }, { 20, 25, 13, 15 }, 0 },
	{ 22, 100, 40, { 0, 0, 0, 0 }, { 20, 33, 13, 15 }, 0 },
	{ 22, 130, 70, { 0, 0, 0, 0 }, { 20, 48, 13, 15 }, 0 },
	{ 23, 10, 10, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 20, 20, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 60, 60, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 100, 10, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 100, 18, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 100, 24, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 100, 40, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 23, 130, 70, { 0, 0, 0, 0 }, { 18, 20, 23, 20 }, 0 },
	{ 24, 10, 10, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 20, 20, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 60, 60, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 100, 10, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 100, 18, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 100, 24, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 100, 40, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 24, 130, 70, { 0, 0, 0, 0 }, { 20, 20, 19, 15 }, 0 },
	{ 25, 10, 10, { 18, 20, 0, 10 }, { 14, 16, 22, 32 }, 0 },
	{ 25, 20, 20, { 28, 20, 0, 20 }, { 14, 16, 32, 32 }, 0 },
	{ 25, 60, 60, { 32, 20, 36, 60 }, { 14, 16, 72, 64 }, 0 },
	{ 25, 100, 10, { 32, 20, 76, 10 }, { 14, 16, 112, 32 }, 0 },
	{ 25, 100, 18, { 32, 20, 76, 18 }, { 14, 16, 112, 32 }, 0 },
	{ 25, 100, 24, { 32, 20, 76, 24 }, { 14, 16, 112, 32 }, 0 },
	{ 25, 100, 40, { 32, 20, 76, 40 }, { 14, 16, 112, 44 }, 0 },
	{ 25, 130, 70, { 32, 20, 106, 70 }, { 14, 16, 142, 74 }, 0 },
	{ 26, 10, 10, { 25, 21, 0, 17 }, { 14, 17, 32, 21 }, 0 },
	{ 26, 20, 20, { 30, 21, 0, 17 }, { 14, 17, 32, 30 }, 0 },
	{ 26, 60, 60, { 32, 21, 36, 17 }, { 14, 17, 72, 70 }, 0 },
	{ 26, 100, 10, { 32, 21, 76, 17 }, { 14, 17, 112, 21 }, 0 },
	{ 26, 100, 18, { 32, 21, 76, 17 }, { 14, 17, 112, 28 }, 0 },
	{ 26, 100, 24, { 32, 21, 76, 17 }, { 14, 17, 112, 34 }, 0 },
	{ 26, 100, 40, { 32, 21, 76, 17 }, { 14, 17, 112, 50 }, 0 },
	{ 26, 130, 70, { 32, 21, 106, 17 }, { 14, 17, 142, 80 }, 0 },
	{ 27, 10, 10, { 32, 21, 14, 13 }, { 20, 20, 26, 16 }, 1 },
	{ 27, 20, 20, { 32, 21, 4, 13 }, { 20, 20, 20, 16 }, 1 },
	{ 27, 60, 60, { 32, 21, 36, 13 }, { 20, 20, 60, 16 }, 0 },
	{ 27, 100, 10, { 32, 21, 76, 13 }, { 20, 20, 100, 16 }, 0 },
	{ 27, 100, 18, { 32, 21, 76, 13 }, { 20, 20, 100, 16 }, 0 },
	{ 27, 100, 24, { 32, 21, 76, 13 }, { 20, 20, 100, 16 }, 0 },
	{ 27, 100, 40, { 32, 21, 76, 13 }, { 20, 20, 100, 16 }, 0 },
	{ 27, 130, 70, { 32, 21, 106, 13 }, { 20, 20, 130, 16 }, 0 },
	{ 28, 10, 10, { 29, 22, 0, 6 }, { 18, 18, 15, 16 }, 0 },
	{ 28, 20, 20, { 29, 22, 0, 16 }, { 18, 18, 25, 26 }, 0 },
	{ 28, 60, 60, { 29, 22, 21, 56 }, { 18, 18, 65, 66 }, 0 },
	{ 28, 100, 10, { 29, 22, 61, 6 }, { 18, 18, 105, 16 }, 0 },
	{ 28, 100, 18, { 29, 22, 61, 14 }, { 18, 18, 105, 24 }, 0 },
	{ 28, 100, 24, { 29, 22, 61, 20 }, { 18, 18, 105, 30 }, 0 },
	{ 28, 100, 40, { 29, 22, 61, 36 }, { 18, 18, 105, 46 }, 0 },
	{ 28, 130, 70, { 29, 22, 91, 66 }, { 18, 18, 135, 76 }, 0 },
	{ 29, 10, 10, { 27, 22, 0, 6 }, { 17, 19, 16, 15 }, 0 },
	{ 29, 20, 20, { 27, 22, 0, 16 }, { 17, 19, 26, 25 }, 0 },
	{ 29, 60, 60, { 27, 22, 28, 56 }, { 17, 19, 66, 65 }, 0 },
	{ 29, 100, 10, { 27, 22, 68, 6 }, { 17, 19, 106, 15 }, 0 },
	{ 29, 100, 18, { 27, 22, 68, 14 }, { 17, 19, 106, 23 }, 0 },
	{ 29, 100, 24, { 27, 22, 68, 20 }, { 17, 19, 106, 29 }, 0 },
	{ 29, 100, 40, { 27, 22, 68, 36 }, { 17, 19, 106, 45 }, 0 },
	{ 29, 130, 70, { 27, 22, 98, 66 }, { 17, 19, 136, 75 }, 0 },
	{ 30, 10, 10, { 27, 21, 0, 13 }, { 13, 20, 26, 15 }, 0 },
	{ 30, 20, 20, { 27, 21, 0, 13 }, { 20, 20, 20, 15 }, 0 },
	{ 30, 60, 60, { 27, 21, 32, 13 }, { 20, 20, 60, 15 }, 0 },
	{ 30, 100, 10, { 27, 21, 72, 13 }, { 20, 20, 100, 15 }, 0 },
	{ 30, 100, 18, { 27, 21, 72, 13 }, { 20, 20, 100, 15 }, 0 },
	{ 30, 100, 24, { 27, 21, 72, 13 }, { 20, 20, 100, 15 }, 0 },
	{ 30, 100, 40, { 27, 21, 72, 13 }, { 20, 20, 100, 15 }, 0 },
	{ 30, 130, 70, { 27, 21, 102, 13 }, { 20, 20, 130, 15 }, 0 },
	{ 31, 10, 10, { 20, 19, 10, 11 }, { 17, 16, 16, 17 }, 0 },
	{ 31, 20, 20, { 20, 19, 20, 21 }, { 17, 16, 26, 27 }, 0 },
	{ 31, 60, 60, { 20, 19, 60, 61 }, { 17, 16, 66, 67 }, 0 },
	{ 31, 100, 10, { 20, 19, 100, 11 }, { 17, 16, 106, 17 }, 0 },
	{ 31, 100, 18, { 20, 19, 100, 19 }, { 17, 16, 106, 25 }, 0 },
	{ 31, 100, 24, { 20, 19, 100, 25 }, { 17, 16, 106, 31 }, 0 },
	{ 31, 100, 40, { 20, 19, 100, 41 }, { 17, 16, 106, 47 }, 0 },
	{ 31, 130, 70, { 20, 19, 130, 71 }, { 17, 16, 136, 77 }, 0 },
	{ 32, 10, 10, { 22, 19, 6, 11 }, { 20, 16, 10, 18 }, 0 },
	{ 32, 20, 20, { 22, 19, 16, 11 }, { 20, 16, 20, 18 }, 0 },
	{ 32, 60, 60, { 24, 24, 52, 51 }, { 20, 20, 60, 60 }, 0 },
	{ 32, 100, 10, { 22, 19, 96, 11 }, { 20, 16, 100, 18 }, 0 },
	{ 32, 100, 18, { 22, 19, 96, 11 }, { 20, 16, 100, 18 }, 0 },
	{ 32, 100, 24, { 24, 24, 92, 15 }, { 20, 20, 100, 24 }, 0 },
	{ 32, 100, 40, { 24, 24, 92, 31 }, { 20, 20, 100, 40 }, 0 },
	{ 32, 130, 70, { 24, 24, 122, 61 }, { 20, 20, 130, 70 }, 0 },
	{ 33, 10, 10, { 25, 21, 0, 17 }, { 14, 17, 32, 21 }, 3 },
	{ 33, 20, 20, { 28, 20, 0, 20 }, { 14, 16, 32, 32 }, 0 },
	{ 33, 60, 60, { 34, 20, 32, 60 }, { 14, 16, 72, 72 }, 0 },
	{ 33, 100, 10, { 32, 21, 76, 17 }, { 14, 17, 112, 21 }, 0 },
	{ 33, 100, 18, { 32, 21, 76, 17 }, { 14, 17, 112, 28 }, 0 },
	{ 33, 100, 24, { 34, 20, 72, 24 }, { 14, 16, 112, 36 }, 0 },
	{ 33, 100, 40, { 34, 20, 72, 40 }, { 14, 16, 112, 52 }, 0 },
	{ 33, 130, 70, { 34, 20, 102, 70 }, { 14, 16, 142, 82 }, 0 },
	{ 34, 10, 10, { 20, 20, 10, 10 }, { 13, 13, 25, 25 }, 0 },
	{ 34, 20, 20, { 20, 20, 20, 20 }, { 18, 18, 25, 25 }, 0 },
	{ 34, 60, 60, { 20, 20, 60, 60 }, { 20, 20, 60, 60 }, 0 },
	{ 34, 100, 10, { 20, 20, 100, 10 }, { 20, 13, 100, 25 }, 0 },
	{ 34, 100, 18, { 20, 20, 100, 18 }, { 20, 17, 100, 25 }, 0 },
	{ 34, 100, 24, { 20, 20, 100, 24 }, { 20, 20, 100, 25 }, 0 },
	{ 34, 100, 40, { 20, 20, 100, 40 }, { 20, 20, 100, 40 }, 0 },
	{ 34, 130, 70, { 20, 20, 130, 70 }, { 20, 20, 130, 70 }, 0 },
};

static int rect_equals(CGRect r, const int e[4])
{
	return r.origin.x == e[0] && r.origin.y == e[1] && r.size.width == e[2] && r.size.height == e[3];
}

static void test_button_bounds(void)
{
	for (unsigned i = 0; i < COUNT(kButtonRows); i++)
	{
		HIThemeButtonDrawInfo info;
		memset(&info, 0, sizeof(info));
		info.state = kThemeStateActive;
		info.kind = kButtonRows[i].kind;
		HIRect in = CGRectMake(20, 20, kButtonRows[i].w, kButtonRows[i].h);
		HIRect content = CGRectMake(-1, -1, -1, -1), bg = content;
		OSStatus s1 = HIThemeGetButtonContentBounds(&in, &info, &content);
		OSStatus s2 = HIThemeGetButtonBackgroundBounds(&in, &info, &bg);
		CHECK(s1 == noErr && s2 == noErr);
		if (!(kButtonRows[i].known & 1))
		{
			if (!rect_equals(content, kButtonRows[i].content))
			{
				printf("FAIL content bounds kind %d in %dx%d: %g,%g,%g,%g\n", kButtonRows[i].kind, kButtonRows[i].w, kButtonRows[i].h,
					content.origin.x, content.origin.y, content.size.width, content.size.height);
				g_failures++;
			}
		}
		else
			g_known_deviations++;
		if (!(kButtonRows[i].known & 2))
		{
			if (!rect_equals(bg, kButtonRows[i].background))
			{
				printf("FAIL background bounds kind %d in %dx%d: %g,%g,%g,%g\n", kButtonRows[i].kind, kButtonRows[i].w, kButtonRows[i].h,
					bg.origin.x, bg.origin.y, bg.size.width, bg.size.height);
				g_failures++;
			}
		}
		else
			g_known_deviations++;

		// the shape is the background rectangle
		HIShapeRef shape = NULL;
		CHECK(HIThemeGetButtonShape(&in, &info, &shape) == noErr && shape != NULL);
		if (shape != NULL)
		{
			CGRect sb;
			HIShapeGetBounds(shape, &sb);
			CHECK(CGRectEqualToRect(sb, bg));
			CFRelease(shape);
		}
	}
}

static void test_button_errors(void)
{
	HIThemeButtonDrawInfo info;
	memset(&info, 0, sizeof(info));
	HIRect in = CGRectMake(0, 0, 100, 30), out;
	HIShapeRef shape = NULL;
	CHECK(HIThemeGetButtonContentBounds(NULL, &info, &out) == paramErr);
	CHECK(HIThemeGetButtonContentBounds(&in, NULL, &out) == paramErr);
	CHECK(HIThemeGetButtonContentBounds(&in, &info, NULL) == paramErr);
	CHECK(HIThemeGetButtonBackgroundBounds(NULL, &info, &out) == paramErr);
	CHECK(HIThemeGetButtonBackgroundBounds(&in, &info, NULL) == paramErr);
	CHECK(HIThemeGetButtonShape(&in, &info, NULL) == paramErr);
	CHECK(HIThemeGetButtonShape(NULL, &info, &shape) == paramErr);
	info.kind = 999;
	CHECK(HIThemeGetButtonContentBounds(&in, &info, &out) != noErr);
	CHECK(HIThemeGetButtonBackgroundBounds(&in, &info, &out) != noErr);
	CHECK(HIThemeGetButtonShape(&in, &info, &shape) != noErr);
}

static void expect_shape(const char *what, OSStatus st, HIShapeRef shape, double x, double y, double w, double h)
{
	if (st != noErr || shape == NULL)
	{
		printf("FAIL %s: status %d\n", what, (int) st);
		g_failures++;
		return;
	}
	CGRect b;
	HIShapeGetBounds(shape, &b);
	if (b.origin.x != x || b.origin.y != y || b.size.width != w || b.size.height != h)
	{
		printf("FAIL %s: %g,%g,%g,%g, expected %g,%g,%g,%g\n", what, b.origin.x, b.origin.y, b.size.width, b.size.height, x, y, w, h);
		g_failures++;
	}
	CHECK(HIShapeIsRectangular(shape));
	CHECK(!HIShapeIsEmpty(shape));
	CFRelease(shape);
}

static void test_tab_menu_grow_shapes(void)
{
	HIRect pane = CGRectMake(10, 20, 140, 70);
	HIShapeRef shape = NULL;
	for (int size = 0; size <= 3; size += size == 1 ? 2 : 1)
	{
		expect_shape("tab pane draw shape", HIThemeGetTabPaneDrawShape(&pane, kThemeTabNorth, size, &shape), shape, 2, 14, 157, 84);
		expect_shape("tab pane content north", HIThemeGetTabPaneContentShape(&pane, kThemeTabNorth, size, &shape), shape, 10, 30, 140, 60);
		expect_shape("tab pane content south", HIThemeGetTabPaneContentShape(&pane, kThemeTabSouth, size, &shape), shape, 10, 20, 140, 60);
		expect_shape("tab pane content east", HIThemeGetTabPaneContentShape(&pane, kThemeTabEast, size, &shape), shape, 10, 20, 130, 70);
		expect_shape("tab pane content west", HIThemeGetTabPaneContentShape(&pane, kThemeTabWest, size, &shape), shape, 20, 20, 130, 70);
	}
	CHECK(HIThemeGetTabPaneDrawShape(&pane, 9, 0, &shape) == paramErr);
	CHECK(HIThemeGetTabPaneContentShape(NULL, kThemeTabNorth, 0, &shape) == paramErr);
	CHECK(HIThemeGetTabPaneContentShape(&pane, kThemeTabNorth, 0, NULL) == paramErr);

	HIRect tabRect = CGRectMake(20, 20, 60, 22);
	HIThemeTabDrawInfo tab;
	memset(&tab, 0, sizeof(tab));
	tab.version = 1;
	tab.style = kThemeTabFront;
	tab.direction = kThemeTabNorth;
	expect_shape("tab shape", HIThemeGetTabShape(&tabRect, &tab, &shape), shape, 20, 20, 60, 22);
	expect_shape("tab draw shape", HIThemeGetTabDrawShape(&tabRect, &tab, &shape), shape, 20, 20, 60, 22);

	HIRect menu = CGRectMake(10, 10, 120, 80);
	HIThemeMenuDrawInfo mi;
	memset(&mi, 0, sizeof(mi));
	for (int type = kThemeMenuTypePullDown; type <= kThemeMenuTypeHierarchical; type++)
	{
		mi.menuType = type;
		expect_shape("menu background shape", HIThemeGetMenuBackgroundShape(&menu, &mi, &shape), shape, 10, 6, 120, 88);
	}

	HIPoint origin = CGPointMake(50, 50);
	HIThemeGrowBoxDrawInfo gi;
	memset(&gi, 0, sizeof(gi));
	gi.state = kThemeStateActive;
	gi.kind = kHIThemeGrowBoxKindNormal;
	HIRect gb;
	gi.size = kHIThemeGrowBoxSizeNormal;
	CHECK(HIThemeGetGrowBoxBounds(&origin, &gi, &gb) == noErr && gb.origin.x == 50 && gb.origin.y == 50 && gb.size.width == 15 && gb.size.height == 15);
	gi.size = kHIThemeGrowBoxSizeSmall;
	CHECK(HIThemeGetGrowBoxBounds(&origin, &gi, &gb) == noErr && gb.size.width == 11 && gb.size.height == 11);
	CHECK(HIThemeGetGrowBoxBounds(NULL, &gi, &gb) == paramErr);
	CHECK(HIThemeGetGrowBoxBounds(&origin, &gi, NULL) == paramErr);
}

static void test_window_shapes(void)
{
	HIRect content = CGRectMake(20, 30, 100, 50);
	HIThemeWindowDrawInfo wi;
	memset(&wi, 0, sizeof(wi));
	wi.state = kThemeStateActive;
	wi.attributes = kThemeWindowHasCloseBox | kThemeWindowHasFullZoom | kThemeWindowHasCollapseBox | kThemeWindowHasTitleText;
	HIShapeRef shape = NULL;

	// the plain dialog, shadow dialog and popup window types are rejected (observed on macOS 10.14.4)
	for (int type = kThemePlainDialogWindow; type <= kThemePopupWindow; type++)
	{
		wi.windowType = type;
		shape = NULL;
		CHECK(HIThemeGetWindowShape(&content, &wi, kAppearanceRegionStructure, &shape) == paramErr);
		CHECK(shape == NULL);
	}

	wi.windowType = kThemeDocumentWindow;
	expect_shape("document title bar", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionTitleBar, &shape), shape, 20, 8, 100, 22);
	expect_shape("document drag", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionDrag, &shape), shape, 20, 8, 100, 22);
	expect_shape("document close box", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionCloseBox, &shape), shape, 26, 10, 19, 18);
	expect_shape("document zoom box", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionZoomBox, &shape), shape, 70, 10, 19, 18);
	expect_shape("document collapse box", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionCollapseBox, &shape), shape, 48, 10, 19, 19);
	expect_shape("document structure", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionStructure, &shape), shape, 20, 8, 100, 72);
	expect_shape("document content", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionContent, &shape), shape, 20, 30, 100, 50);

	wi.windowType = kThemeUtilityWindow;
	expect_shape("utility title bar", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionTitleBar, &shape), shape, 20, 14, 100, 16);
	expect_shape("utility close box", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionCloseBox, &shape), shape, 26, 16, 11, 12);
	expect_shape("utility zoom box", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionZoomBox, &shape), shape, 62, 16, 11, 12);
	expect_shape("utility collapse box", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionCollapseBox, &shape), shape, 44, 16, 11, 12);
	expect_shape("utility structure", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionStructure, &shape), shape, 20, 14, 100, 66);

	wi.windowType = kThemeMovableDialogWindow;
	expect_shape("movable dialog title bar", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionTitleBar, &shape), shape, 16, 8, 108, 22);
	expect_shape("movable dialog structure", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionStructure, &shape), shape, 16, 8, 108, 72);

	wi.windowType = kThemeDialogWindow;
	expect_shape("dialog structure", HIThemeGetWindowShape(&content, &wi, kAppearanceRegionStructure, &shape), shape, 20, 30, 100, 50);

	// region hit testing on a document window
	wi.windowType = kThemeDocumentWindow;
	WindowRegionCode region = 0;
	HIPoint p = CGPointMake(30, 15);
	CHECK(HIThemeGetWindowRegionHit(&content, &wi, &p, &region) && region == kAppearanceRegionCloseBox);
	p = CGPointMake(75, 15);
	CHECK(HIThemeGetWindowRegionHit(&content, &wi, &p, &region) && region == kAppearanceRegionZoomBox);
	p = CGPointMake(100, 15);
	CHECK(HIThemeGetWindowRegionHit(&content, &wi, &p, &region) && region == kAppearanceRegionTitleBar);
	p = CGPointMake(60, 50);
	CHECK(HIThemeGetWindowRegionHit(&content, &wi, &p, &region) && region == kAppearanceRegionContent);
	p = CGPointMake(5, 5);
	CHECK(!HIThemeGetWindowRegionHit(&content, &wi, &p, &region));
	CHECK(!HIThemeGetWindowRegionHit(NULL, &wi, &p, &region));

	CHECK(HIThemeGetWindowShape(NULL, &wi, kAppearanceRegionContent, &shape) == paramErr);
	CHECK(HIThemeGetWindowShape(&content, NULL, kAppearanceRegionContent, &shape) == paramErr);
	CHECK(HIThemeGetWindowShape(&content, &wi, kAppearanceRegionContent, NULL) == paramErr);
	wi.windowType = 99;
	CHECK(HIThemeGetWindowShape(&content, &wi, kAppearanceRegionContent, &shape) == paramErr);
}

static HIThemeTrackDrawInfo track_info(ThemeTrackKind kind, CGRect bounds, int horizontal, SInt32 value)
{
	HIThemeTrackDrawInfo ti;
	memset(&ti, 0, sizeof(ti));
	ti.kind = kind;
	ti.bounds = bounds;
	ti.min = 0;
	ti.max = 100;
	ti.value = value;
	ti.attributes = (horizontal ? kThemeTrackHorizontal : 0) | kThemeTrackShowThumb;
	ti.enableState = kThemeTrackActive;
	if (kind == kThemeScrollBarMedium || kind == kThemeScrollBarSmall)
		ti.trackInfo.scrollbar.viewsize = 25;
	return ti;
}

static int rect_is(CGRect r, double x, double y, double w, double h)
{
	return r.origin.x == x && r.origin.y == y && r.size.width == w && r.size.height == h;
}

static void test_tracks(void)
{
	// medium horizontal scroll bar, thumb at 30 of 100 with a view size of 25
	HIThemeTrackDrawInfo sb = track_info(kThemeScrollBarMedium, CGRectMake(10, 20, 120, 20), 1, 30);
	HIRect r;
	CHECK(HIThemeGetTrackBounds(&sb, &r) == noErr && rect_is(r, 9, 20, 122, 15));
	CHECK(HIThemeGetTrackPartBounds(&sb, kAppearancePartUpButton, &r) == noErr && rect_is(r, 86, 20, 27, 15));
	CHECK(HIThemeGetTrackPartBounds(&sb, kAppearancePartDownButton, &r) == noErr && rect_is(r, 113, 20, 17, 15));
	CHECK(HIThemeGetTrackPartBounds(&sb, kAppearancePartPageUpArea, &r) == noErr && rect_is(r, 14, 20, 18, 15));
	CHECK(HIThemeGetTrackPartBounds(&sb, kAppearancePartPageDownArea, &r) == noErr && rect_is(r, 58, 20, 42, 15));
	CHECK(HIThemeGetTrackPartBounds(&sb, kAppearancePartIndicator, &r) == noErr && rect_is(r, 32, 20, 26, 15));
	UInt32 n = 0;
	ControlPartCode parts[8] = { 0 };
	CHECK(HIThemeGetTrackParts(&sb, &n, 8, parts) == noErr && n == 5);
	CHECK(parts[0] == kAppearancePartPageUpArea && parts[1] == kAppearancePartIndicator && parts[2] == kAppearancePartPageDownArea
		&& parts[3] == kAppearancePartUpButton && parts[4] == kAppearancePartDownButton);

	ControlPartCode hit = 0;
	HIPoint p = CGPointMake(40, 25);
	CHECK(HIThemeHitTestTrack(&sb, &p, &hit) && hit == kAppearancePartIndicator);
	p = CGPointMake(20, 25);
	CHECK(HIThemeHitTestTrack(&sb, &p, &hit) && hit == kAppearancePartPageUpArea);
	p = CGPointMake(70, 25);
	CHECK(HIThemeHitTestTrack(&sb, &p, &hit) && hit == kAppearancePartPageDownArea);
	p = CGPointMake(120, 25);
	CHECK(HIThemeHitTestTrack(&sb, &p, &hit) && hit == kAppearancePartDownButton);
	p = CGPointMake(95, 25);
	CHECK(HIThemeHitTestTrack(&sb, &p, &hit) && hit == kAppearancePartUpButton);
	p = CGPointMake(200, 25);
	CHECK(!HIThemeHitTestTrack(&sb, &p, &hit) && hit == 0);

	// vertical bar: the bounds are outset by one pixel horizontally
	HIThemeTrackDrawInfo vsb = track_info(kThemeScrollBarMedium, CGRectMake(10, 10, 20, 80), 0, 30);
	CHECK(HIThemeGetTrackBounds(&vsb, &r) == noErr && rect_is(r, 9, 10, 17, 80));
	// small scroll bar
	HIThemeTrackDrawInfo ssb = track_info(kThemeScrollBarSmall, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackBounds(&ssb, &r) == noErr && rect_is(r, 9, 20, 122, 11));
	CHECK(HIThemeGetTrackPartBounds(&ssb, kAppearancePartUpButton, &r) == noErr && rect_is(r, 96, 20, 21, 11));
	CHECK(HIThemeGetTrackPartBounds(&ssb, kAppearancePartDownButton, &r) == noErr && rect_is(r, 117, 20, 13, 11));
	CHECK(HIThemeGetTrackPartBounds(&ssb, kAppearancePartPageUpArea, &r) == noErr && rect_is(r, 14, 20, 22, 11));
	CHECK(HIThemeGetTrackPartBounds(&ssb, kAppearancePartIndicator, &r) == noErr && rect_is(r, 36, 20, 20, 11));

	// the thumb moves with the value and stays inside the track
	CGFloat previous = -1;
	for (SInt32 v = 0; v <= 100; v += 10)
	{
		HIThemeTrackDrawInfo t = track_info(kThemeScrollBarMedium, CGRectMake(10, 20, 120, 20), 1, v);
		HIRect thumb, trackRect;
		CHECK(HIThemeGetTrackPartBounds(&t, kAppearancePartIndicator, &thumb) == noErr);
		CHECK(HIThemeGetScrollBarTrackRect(&t.bounds, &(HIScrollBarTrackInfo){ 0, kThemeTrackActive, 0, 25 }, true, &trackRect) == noErr);
		CHECK(thumb.origin.x >= previous);
		CHECK(thumb.origin.x >= trackRect.origin.x && thumb.origin.x + thumb.size.width <= trackRect.origin.x + trackRect.size.width);
		previous = thumb.origin.x;
		// the offset of the thumb leads back to the value
		CGFloat pos = -1;
		SInt32 live = -1;
		CHECK(HIThemeGetTrackThumbPositionFromBounds(&t, &thumb, &pos) == noErr);
		CHECK(HIThemeGetTrackLiveValue(&t, pos, &live) == noErr);
		if (live < v - 1 || live > v + 1)
		{
			printf("FAIL live value %d for value %d\n", (int) live, (int) v);
			g_failures++;
		}
	}

	// medium horizontal slider, thumb at 30
	HIThemeTrackDrawInfo sl = track_info(kThemeSliderMedium, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackBounds(&sl, &r) == noErr && rect_is(r, 7, 20, 126, 21));
	HIShapeRef shape = NULL;
	CHECK(HIThemeGetTrackThumbShape(&sl, &shape) == noErr && shape != NULL);
	if (shape != NULL)
	{
		CGRect b;
		HIShapeGetBounds(shape, &b);
		CHECK(rect_is(b, 38, 20, 23, 21));
		CFRelease(shape);
	}
	HIThemeTrackDrawInfo sls = track_info(kThemeSliderSmall, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackBounds(&sls, &r) == noErr && rect_is(r, 8, 21, 124, 16));
	HIThemeTrackDrawInfo slm = track_info(kThemeSliderMini, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackBounds(&slm, &r) == noErr && rect_is(r, 9, 20, 122, 12));

	// progress bars
	HIThemeTrackDrawInfo pb = track_info(kThemeProgressBarMedium, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackBounds(&pb, &r) == noErr && rect_is(r, 9, 20, 122, 12));
	HIThemeTrackDrawInfo ib = track_info(kThemeIndeterminateBarMedium, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackBounds(&ib, &r) == noErr && rect_is(r, 9, 20, 122, 12));

	// kinds that do not exist as tracks
	HIThemeTrackDrawInfo bad = track_info(kThemeScrollBarMini, CGRectMake(10, 10, 20, 80), 0, 30);
	CHECK(HIThemeGetTrackBounds(&bad, &r) == paramErr);
	bad.kind = kThemeProgressBarMini;
	CHECK(HIThemeGetTrackBounds(&bad, &r) == paramErr);
	bad.kind = kThemeIndeterminateBarMini;
	CHECK(HIThemeGetTrackBounds(&bad, &r) == paramErr);
	bad.kind = 77;
	CHECK(HIThemeGetTrackBounds(&bad, &r) == paramErr);
	CHECK(HIThemeGetTrackBounds(NULL, &r) == paramErr);
	CHECK(HIThemeGetTrackBounds(&sb, NULL) == paramErr);

	// only scroll bars list their parts
	CHECK(HIThemeGetTrackParts(&sl, &n, 8, parts) == paramErr);
	CHECK(HIThemeGetTrackParts(&sb, NULL, 8, parts) == paramErr);
}

static void hitheme_quirk_unknown_scrollbar_part_is_whole_bar(void)
{
	HIThemeTrackDrawInfo sb = track_info(kThemeScrollBarMedium, CGRectMake(10, 20, 120, 20), 1, 30);
	HIRect r = CGRectZero;
	CHECK(HIThemeGetTrackPartBounds(&sb, 5, &r) == noErr);
	CHECK(rect_is(r, 10, 20, 120, 15));
	HIThemeTrackDrawInfo sl = track_info(kThemeSliderMedium, CGRectMake(10, 20, 120, 20), 1, 30);
	CHECK(HIThemeGetTrackPartBounds(&sl, 5, &r) == paramErr);
}

int main(void)
{
	test_button_bounds();
	test_button_errors();
	test_tab_menu_grow_shapes();
	test_window_shapes();
	test_tracks();
	hitheme_quirk_unknown_scrollbar_part_is_whole_bar();
	printf("button rows not reproduced (listed in the table): %d\n", g_known_deviations);
	if (g_failures != 0)
	{
		printf("hitheme_geometry: %d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS hitheme_geometry\n");
	return 0;
}
