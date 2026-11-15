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

// QuickDraw rectangles and regions: area, bounds, set operations and the
// containment queries. Expected values come from the documented behaviour
// and were checked black-box on Mac OS X 10.14.4 (i386 and x86_64).

#include <QD/QD.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(const char* what, int ok)
{
	if (!ok) {
		printf("FAIL: %s\n", what);
		failures++;
	}
}

static int boundsAre(RgnHandle rgn, short top, short left, short bottom, short right)
{
	Rect r = {-1, -1, -1, -1};
	GetRegionBounds(rgn, &r);
	return r.top == top && r.left == left && r.bottom == bottom && r.right == right;
}

static Point pt(short h, short v)
{
	Point p;
	p.h = h;
	p.v = v;
	return p;
}

int main(void)
{
	Rect r;
	Rect q;
	Rect out;

	// Rectangles: SetRect takes left, top, right, bottom.
	SetRect(&r, 1, 2, 3, 4);
	check("SetRect order", r.left == 1 && r.top == 2 && r.right == 3 && r.bottom == 4);
	SetRect(&r, 30, 40, 10, 20);
	check("SetRect does not normalize", r.left == 30 && r.top == 40 && r.right == 10 && r.bottom == 20);
	check("inverted rect is empty", EmptyRect(&r));
	SetRect(&r, 0, 0, 10, 10);
	check("rect has area", !EmptyRect(&r));
	check("PtInRect top-left inside", PtInRect(pt(0, 0), &r));
	check("PtInRect bottom-right outside", !PtInRect(pt(10, 10), &r));
	check("PtInRect interior", PtInRect(pt(9, 9), &r));
	SetRect(&q, 5, 5, 15, 15);
	check("SectRect overlap", SectRect(&r, &q, &out) && out.left == 5 && out.top == 5 && out.right == 10 && out.bottom == 10);
	SetRect(&q, 20, 20, 30, 30);
	check("SectRect disjoint gives false and an empty rect",
		!SectRect(&r, &q, &out) && out.left == 0 && out.top == 0 && out.right == 0 && out.bottom == 0);
	UnionRect(&r, &q, &out);
	check("UnionRect", out.left == 0 && out.top == 0 && out.right == 30 && out.bottom == 30);
	SetRect(&r, 0, 0, 0, 0);
	UnionRect(&r, &q, &out);
	check("UnionRect ignores an empty rect", EqualRect(&out, &q));
	SetRect(&r, 0, 0, 10, 10);
	OffsetRect(&r, 5, -3);
	check("OffsetRect", r.left == 5 && r.top == -3 && r.right == 15 && r.bottom == 7);
	InsetRect(&r, 2, 1);
	check("InsetRect", r.left == 7 && r.top == -2 && r.right == 13 && r.bottom == 6);

	// A new region is empty, with all-zero bounds.
	RgnHandle a = NewRgn(), c = NewRgn(), d = NewRgn();
	check("NewRgn", a && c && d);
	check("new region is empty", EmptyRgn(a) && boundsAre(a, 0, 0, 0, 0));

	// SetRectRgn takes left, top, right, bottom.
	SetRectRgn(a, 10, 20, 30, 40);
	check("SetRectRgn bounds", !EmptyRgn(a) && boundsAre(a, 20, 10, 40, 30));
	SetRect(&r, 15, 25, 25, 35);
	check("RectInRgn inside", RectInRgn(&r, a));
	SetRect(&r, 0, 0, 100, 100);
	check("RectInRgn enclosing", RectInRgn(&r, a));
	SetRect(&r, 0, 0, 10, 20);
	check("RectInRgn outside", !RectInRgn(&r, a));
	SetRect(&r, 30, 20, 40, 40);
	check("RectInRgn touching the right edge", !RectInRgn(&r, a));
	SetRect(&r, 0, 20, 10, 40);
	check("RectInRgn touching the left edge", !RectInRgn(&r, a));
	SetRect(&r, 25, 15, 25, 25);
	check("RectInRgn empty rect", !RectInRgn(&r, a));
	SetRect(&r, 35, 25, 25, 15);
	check("RectInRgn inverted rect", !RectInRgn(&r, a));
	check("PtInRgn inside", PtInRgn(pt(15, 25), a));
	check("PtInRgn top-left", PtInRgn(pt(10, 20), a));
	check("PtInRgn last pixel", PtInRgn(pt(29, 39), a));
	check("PtInRgn right edge", !PtInRgn(pt(30, 25), a));
	check("PtInRgn bottom edge", !PtInRgn(pt(15, 40), a));

	// A rectangle with no area leaves the region empty.
	// quirk_empty_rect_keeps_bounds: the bounds are the given coordinates.
	SetRectRgn(a, 30, 40, 10, 20);
	check("quirk_empty_rect_keeps_bounds: inverted", EmptyRgn(a) && boundsAre(a, 40, 30, 20, 10));
	SetRectRgn(a, 10, 20, 10, 40);
	check("quirk_empty_rect_keeps_bounds: zero width", EmptyRgn(a) && boundsAre(a, 20, 10, 40, 10));
	SetRectRgn(a, 10, 20, 30, 40);
	SetEmptyRgn(a);
	check("SetEmptyRgn", EmptyRgn(a) && boundsAre(a, 0, 0, 0, 0));

	// Union of two separate rectangles: the gap is not in the region.
	SetRectRgn(a, 0, 0, 10, 10);
	SetRectRgn(c, 20, 20, 30, 30);
	UnionRgn(a, c, d);
	check("union bounds", !EmptyRgn(d) && boundsAre(d, 0, 0, 30, 30));
	SetRect(&r, 10, 10, 20, 20);
	check("RectInRgn in the gap", !RectInRgn(&r, d));
	SetRect(&r, 5, 5, 15, 15);
	check("RectInRgn over one part", RectInRgn(&r, d));
	SetRect(&r, 12, 12, 18, 18);
	check("RectInRgn inside the gap", !RectInRgn(&r, d));
	SetRect(&r, 0, 0, 10, 10);
	check("RectInRgn the first part", RectInRgn(&r, d));
	SetRect(&r, 20, 20, 30, 30);
	check("RectInRgn the second part", RectInRgn(&r, d));
	check("PtInRgn in the gap", !PtInRgn(pt(15, 15), d));
	check("PtInRgn in either part", PtInRgn(pt(5, 5), d) && PtInRgn(pt(25, 25), d));
	check("EqualRgn with itself", EqualRgn(d, d));
	check("EqualRgn of different regions", !EqualRgn(a, c));

	// Touching rectangles union to the same area as the enclosing rectangle.
	SetRectRgn(a, 0, 0, 10, 10);
	SetRectRgn(c, 10, 0, 20, 10);
	UnionRgn(a, c, d);
	SetRectRgn(a, 0, 0, 20, 10);
	check("adjacent rects union to one", EqualRgn(a, d) && boundsAre(d, 0, 0, 10, 20));

	// Overlapping rectangles: intersection, difference, exclusive or.
	SetRectRgn(a, 0, 0, 20, 20);
	SetRectRgn(c, 10, 10, 30, 30);
	SectRgn(a, c, d);
	check("SectRgn bounds", boundsAre(d, 10, 10, 20, 20));
	check("SectRgn area", PtInRgn(pt(15, 15), d) && !PtInRgn(pt(5, 5), d) && !PtInRgn(pt(25, 25), d));
	DiffRgn(a, c, d);
	check("DiffRgn bounds", boundsAre(d, 0, 0, 20, 20));
	check("DiffRgn area", PtInRgn(pt(5, 5), d) && PtInRgn(pt(15, 5), d) && PtInRgn(pt(5, 15), d) &&
		!PtInRgn(pt(15, 15), d) && !PtInRgn(pt(25, 25), d));
	XorRgn(a, c, d);
	check("XorRgn bounds", boundsAre(d, 0, 0, 30, 30));
	check("XorRgn area", PtInRgn(pt(5, 5), d) && PtInRgn(pt(25, 25), d) && !PtInRgn(pt(15, 15), d));
	// The result may be one of the sources.
	SectRgn(a, c, a);
	check("SectRgn into a source", boundsAre(a, 10, 10, 20, 20));
	SetRectRgn(d, 0, 0, 10, 10);
	SectRgn(d, c, d);
	check("disjoint SectRgn is empty with zero bounds", EmptyRgn(d) && boundsAre(d, 0, 0, 0, 0));
	SetRectRgn(a, 0, 0, 20, 20);
	UnionRgn(a, a, a);
	check("UnionRgn with itself", boundsAre(a, 0, 0, 20, 20) && !EmptyRgn(a));

	// Copy, offset and inset.
	CopyRgn(a, d);
	check("CopyRgn", EqualRgn(a, d) && boundsAre(d, 0, 0, 20, 20));
	OffsetRgn(d, 5, -3);
	check("OffsetRgn", boundsAre(d, -3, 5, 17, 25) && PtInRgn(pt(5, -3), d) && !PtInRgn(pt(4, -3), d));
	check("OffsetRgn leaves the copy's source alone", boundsAre(a, 0, 0, 20, 20));
	InsetRgn(d, 2, 2);
	check("InsetRgn shrinks", boundsAre(d, -1, 7, 15, 23));
	InsetRgn(d, -4, -4);
	check("InsetRgn with negatives grows", boundsAre(d, -5, 3, 19, 27));
	// quirk_inset_to_nothing_collapses_bounds
	InsetRgn(d, 100, 100);
	check("quirk_inset_to_nothing_collapses_bounds", EmptyRgn(d) && boundsAre(d, 7, 15, 7, 15));
	RectRgn(a, &(Rect){1, 2, 3, 4});
	check("RectRgn", boundsAre(a, 1, 2, 3, 4));

	// An L-shaped region shrinks away from its notch.
	SetRectRgn(a, 0, 0, 30, 30);
	SetRectRgn(c, 10, 10, 30, 30);
	DiffRgn(a, c, d);
	InsetRgn(d, 2, 2);
	check("InsetRgn of an L shape", PtInRgn(pt(2, 2), d) && PtInRgn(pt(2, 27), d) && PtInRgn(pt(27, 2), d) &&
		!PtInRgn(pt(1, 5), d) && !PtInRgn(pt(15, 15), d) && !PtInRgn(pt(10, 10), d) && !PtInRgn(pt(12, 12), d));

	DisposeRgn(a);
	DisposeRgn(c);
	DisposeRgn(d);

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
