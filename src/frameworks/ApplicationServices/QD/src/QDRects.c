/*
 * Copyright (C) 2026 Lubos Dolezel
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// QuickDraw rectangles: shorts with the top-left corner inside and the
// bottom-right corner outside. A rectangle with bottom <= top or right <= left
// is empty. Rects are never normalized.
#include <QD/QD.h>

void SetRect(Rect *r, short left, short top, short right, short bottom) {
	r->top = top;
	r->left = left;
	r->bottom = bottom;
	r->right = right;
}

void OffsetRect(Rect *r, short dh, short dv) {
	r->left += dh;
	r->right += dh;
	r->top += dv;
	r->bottom += dv;
}

void InsetRect(Rect *r, short dh, short dv) {
	r->left += dh;
	r->right -= dh;
	r->top += dv;
	r->bottom -= dv;
}

Boolean EmptyRect(const Rect *r) {
	return r->bottom <= r->top || r->right <= r->left;
}

Boolean EqualRect(const Rect *rect1, const Rect *rect2) {
	return rect1->top == rect2->top && rect1->left == rect2->left &&
	       rect1->bottom == rect2->bottom && rect1->right == rect2->right;
}

Boolean PtInRect(Point pt, const Rect *r) {
	return pt.h >= r->left && pt.h < r->right && pt.v >= r->top && pt.v < r->bottom;
}

// The result is the all-zero rectangle, and the return value false, when the
// rectangles do not overlap.
Boolean SectRect(const Rect *src1, const Rect *src2, Rect *dstRect) {
	Rect result;
	result.top = src1->top > src2->top ? src1->top : src2->top;
	result.left = src1->left > src2->left ? src1->left : src2->left;
	result.bottom = src1->bottom < src2->bottom ? src1->bottom : src2->bottom;
	result.right = src1->right < src2->right ? src1->right : src2->right;
	if (EmptyRect(&result)) {
		SetRect(dstRect, 0, 0, 0, 0);
		return false;
	}
	*dstRect = result;
	return true;
}

// An empty source rectangle does not take part in the union; with both empty
// the result is the all-zero rectangle.
void UnionRect(const Rect *src1, const Rect *src2, Rect *dstRect) {
	Rect result;
	Boolean empty1 = EmptyRect(src1), empty2 = EmptyRect(src2);
	if (empty1 && empty2) {
		SetRect(dstRect, 0, 0, 0, 0);
		return;
	}
	if (empty1) {
		result = *src2;
	} else if (empty2) {
		result = *src1;
	} else {
		result.top = src1->top < src2->top ? src1->top : src2->top;
		result.left = src1->left < src2->left ? src1->left : src2->left;
		result.bottom = src1->bottom > src2->bottom ? src1->bottom : src2->bottom;
		result.right = src1->right > src2->right ? src1->right : src2->right;
	}
	*dstRect = result;
}

#ifndef __LP64__
void AddPt(Point src, Point *dst) {
	dst->h += src.h;
	dst->v += src.v;
}

void SubPt(Point src, Point *dst) {
	dst->h -= src.h;
	dst->v -= src.v;
}
#endif
