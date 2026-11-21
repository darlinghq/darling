/*
 * Copyright (C) 2026 Lubos Dolezel
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// QuickDraw regions. A region is a handle to a block that starts with the
// documented header (size and bounding box) and continues with the region's
// area as a list of non-overlapping rectangles in horizontal bands (top to
// bottom, left to right within a band). Bands that touch and cover the same
// spans are merged, so equal areas have equal lists. All coordinates are
// computed in 32 bits and stored back as the shorts QuickDraw uses.
#include <QD/QD.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int32_t top, left, bottom, right;
} IRect;

#pragma pack(push, 2)
typedef struct {
	UInt16 rgnSize;
	Rect rgnBBox;
} RegionHeader;
#pragma pack(pop)

typedef struct {
	RegionHeader header;
	uint16_t reserved;
	int32_t count;
	int32_t capacity;
	IRect rects[];
} RegionBlock;

typedef enum {
	OpUnion,
	OpSect,
	OpDiff,
	OpXor
} RegionOp;

static short clampShort(int32_t v) {
	return v > 32767 ? 32767 : (v < -32768 ? -32768 : (short) v);
}

static RegionBlock **handleOf(RgnHandle rgn) {
	return (RegionBlock **) rgn;
}

// Replaces the area of the region with the list (which the caller owns) and
// recomputes the header. Returns false when memory ran out, leaving the
// region unchanged.
static int store(RgnHandle rgn, const IRect *rects, int32_t count) {
	RegionBlock **h = handleOf(rgn);
	RegionBlock *block = *h;
	if (block->capacity < count) {
		int32_t capacity = count < 8 ? 8 : count;
		block = realloc(block, sizeof(RegionBlock) + (size_t) capacity * sizeof(IRect));
		if (block == NULL)
			return 0;
		block->capacity = capacity;
		*h = block;
	}
	if (count > 0)
		memmove(block->rects, rects, (size_t) count * sizeof(IRect));
	block->count = count;

	if (count == 0) {
		block->header.rgnBBox.top = block->header.rgnBBox.left = 0;
		block->header.rgnBBox.bottom = block->header.rgnBBox.right = 0;
		block->header.rgnSize = 10;
	} else {
		IRect b = rects[0];
		for (int32_t i = 1; i < count; i++) {
			if (rects[i].top < b.top) b.top = rects[i].top;
			if (rects[i].left < b.left) b.left = rects[i].left;
			if (rects[i].bottom > b.bottom) b.bottom = rects[i].bottom;
			if (rects[i].right > b.right) b.right = rects[i].right;
		}
		block->header.rgnBBox.top = clampShort(b.top);
		block->header.rgnBBox.left = clampShort(b.left);
		block->header.rgnBBox.bottom = clampShort(b.bottom);
		block->header.rgnBBox.right = clampShort(b.right);
		size_t size = count <= 1 ? 10 : 10 + (size_t) count * 8;
		block->header.rgnSize = size > 0xFFFF ? 0xFFFF : (UInt16) size;
	}
	return 1;
}

RgnHandle NewRgn(void) {
	RegionBlock **h = malloc(sizeof(RegionBlock *));
	if (h == NULL)
		return NULL;
	RegionBlock *block = calloc(1, sizeof(RegionBlock) + 8 * sizeof(IRect));
	if (block == NULL) {
		free(h);
		return NULL;
	}
	block->capacity = 8;
	block->header.rgnSize = 10;
	*h = block;
	return (RgnHandle) h;
}

void DisposeRgn(RgnHandle rgn) {
	if (rgn == NULL)
		return;
	RegionBlock **h = handleOf(rgn);
	free(*h);
	free(h);
}

void SetEmptyRgn(RgnHandle rgn) {
	if (rgn == NULL)
		return;
	store(rgn, NULL, 0);
}

// QUIRK: a rectangle with no area (right <= left or bottom <= top) leaves the
// region empty but keeps the given coordinates, unmodified, as its bounds,
// whereas SetEmptyRgn and every region operation give the all-zero bounds.
// Observed black-box on Mac OS X 10.14.4 (i386 and x86_64, tier 2): after
// SetRectRgn(rgn, 30, 40, 10, 20) GetRegionBounds returns top 40, left 30,
// bottom 20, right 10 and EmptyRgn is true.
// Test: tests/src/qd_regions.c (quirk_empty_rect_keeps_bounds)
void SetRectRgn(RgnHandle rgn, short left, short top, short right, short bottom) {
	if (rgn == NULL)
		return;
	if (right <= left || bottom <= top) {
		store(rgn, NULL, 0);
		RegionHeader *header = &(*handleOf(rgn))->header;
		header->rgnBBox.top = top;
		header->rgnBBox.left = left;
		header->rgnBBox.bottom = bottom;
		header->rgnBBox.right = right;
		return;
	}
	IRect r = {top, left, bottom, right};
	store(rgn, &r, 1);
}

void RectRgn(RgnHandle rgn, const Rect *r) {
	SetRectRgn(rgn, r->left, r->top, r->right, r->bottom);
}

void CopyRgn(RgnHandle srcRgn, RgnHandle dstRgn) {
	if (srcRgn == NULL || dstRgn == NULL || srcRgn == dstRgn)
		return;
	RegionBlock *src = *handleOf(srcRgn);
	store(dstRgn, src->rects, src->count);
	(*handleOf(dstRgn))->header.rgnBBox = src->header.rgnBBox;
}

void OffsetRgn(RgnHandle rgn, short dh, short dv) {
	if (rgn == NULL)
		return;
	RegionBlock *block = *handleOf(rgn);
	for (int32_t i = 0; i < block->count; i++) {
		block->rects[i].left += dh;
		block->rects[i].right += dh;
		block->rects[i].top += dv;
		block->rects[i].bottom += dv;
	}
	block->header.rgnBBox.left += dh;
	block->header.rgnBBox.right += dh;
	block->header.rgnBBox.top += dv;
	block->header.rgnBBox.bottom += dv;
}

#pragma mark - Set operations on rectangle lists

typedef struct {
	int32_t *x; // pairs: start, end
	int32_t n;  // number of spans
} Spans;

static int compareInt(const void *a, const void *b) {
	int32_t x = *(const int32_t *) a, y = *(const int32_t *) b;
	return x < y ? -1 : (x > y);
}

// The merged horizontal spans of the rectangles covering the band [y0, y1).
static void spansForBand(const IRect *rects, int32_t count, int32_t y0, int32_t y1, Spans *out) {
	int32_t *pairs = malloc((size_t) (count > 0 ? count : 1) * 2 * sizeof(int32_t));
	int32_t n = 0;
	out->x = pairs;
	out->n = 0;
	if (pairs == NULL)
		return;
	for (int32_t i = 0; i < count; i++) {
		if (rects[i].top <= y0 && rects[i].bottom >= y1 && rects[i].right > rects[i].left) {
			// insertion sort by start
			int32_t j = n++;
			while (j > 0 && pairs[2 * (j - 1)] > rects[i].left) {
				pairs[2 * j] = pairs[2 * (j - 1)];
				pairs[2 * j + 1] = pairs[2 * (j - 1) + 1];
				j--;
			}
			pairs[2 * j] = rects[i].left;
			pairs[2 * j + 1] = rects[i].right;
		}
	}
	int32_t merged = 0;
	for (int32_t i = 0; i < n; i++) {
		if (merged > 0 && pairs[2 * i] <= pairs[2 * (merged - 1) + 1]) {
			if (pairs[2 * i + 1] > pairs[2 * (merged - 1) + 1])
				pairs[2 * (merged - 1) + 1] = pairs[2 * i + 1];
		} else {
			pairs[2 * merged] = pairs[2 * i];
			pairs[2 * merged + 1] = pairs[2 * i + 1];
			merged++;
		}
	}
	out->n = merged;
}

static int inSpans(const Spans *s, int32_t x) {
	for (int32_t i = 0; i < s->n; i++) {
		if (x >= s->x[2 * i] && x < s->x[2 * i + 1])
			return 1;
	}
	return 0;
}

static int wanted(RegionOp op, int inA, int inB) {
	switch (op) {
	case OpUnion: return inA || inB;
	case OpSect: return inA && inB;
	case OpDiff: return inA && !inB;
	case OpXor: return inA != inB;
	}
	return 0;
}

static int sameSpans(const IRect *band, int32_t bandCount, const int32_t *x, int32_t n) {
	if (bandCount != n)
		return 0;
	for (int32_t i = 0; i < n; i++) {
		if (band[i].left != x[2 * i] || band[i].right != x[2 * i + 1])
			return 0;
	}
	return 1;
}

// The result of op on two canonical rectangle lists: a canonical list in
// *out (malloc'ed, possibly NULL when empty) of *outCount rectangles. False
// when memory ran out.
static int combine(const IRect *a, int32_t na, const IRect *b, int32_t nb, RegionOp op, IRect **out, int32_t *outCount) {
	int32_t ny = 0;
	int32_t *ys = malloc((size_t) (2 * (na + nb) + 1) * sizeof(int32_t));
	IRect *result = NULL;
	int32_t count = 0, capacity = 0;
	*out = NULL;
	*outCount = 0;
	if (ys == NULL)
		return 0;
	for (int32_t i = 0; i < na; i++) {
		ys[ny++] = a[i].top;
		ys[ny++] = a[i].bottom;
	}
	for (int32_t i = 0; i < nb; i++) {
		ys[ny++] = b[i].top;
		ys[ny++] = b[i].bottom;
	}
	qsort(ys, (size_t) ny, sizeof(int32_t), compareInt);

	int32_t prevStart = 0, prevCount = 0, prevBottom = 0;
	int ok = 1;
	for (int32_t k = 0; ok && k + 1 < ny; k++) {
		int32_t y0 = ys[k], y1 = ys[k + 1];
		if (y1 <= y0)
			continue;
		Spans sa, sb;
		spansForBand(a, na, y0, y1, &sa);
		spansForBand(b, nb, y0, y1, &sb);
		int32_t np = 2 * (sa.n + sb.n) + 1;
		int32_t *pts = malloc((size_t) np * sizeof(int32_t));
		int32_t *spans = malloc((size_t) (sa.n + sb.n + 1) * 2 * sizeof(int32_t));
		if (sa.x == NULL || sb.x == NULL || pts == NULL || spans == NULL) {
			ok = 0;
		} else {
			int32_t npts = 0, nspans = 0;
			for (int32_t i = 0; i < 2 * sa.n; i++) pts[npts++] = sa.x[i];
			for (int32_t i = 0; i < 2 * sb.n; i++) pts[npts++] = sb.x[i];
			qsort(pts, (size_t) npts, sizeof(int32_t), compareInt);
			// Each segment between neighbouring end points is wholly in or
			// out of each input; consecutive wanted segments make one span.
			for (int32_t i = 0; i + 1 < npts; i++) {
				if (pts[i + 1] == pts[i])
					continue;
				if (!wanted(op, inSpans(&sa, pts[i]), inSpans(&sb, pts[i])))
					continue;
				if (nspans > 0 && spans[2 * (nspans - 1) + 1] == pts[i]) {
					spans[2 * (nspans - 1) + 1] = pts[i + 1];
				} else {
					spans[2 * nspans] = pts[i];
					spans[2 * nspans + 1] = pts[i + 1];
					nspans++;
				}
			}

			if (nspans == 0) {
				prevCount = 0;
			} else {
				// A touching band with the same spans extends the previous one.
				IRect *prev = prevCount > 0 ? &result[prevStart] : NULL;
				if (prev != NULL && prevBottom == y0 && sameSpans(prev, prevCount, spans, nspans)) {
					for (int32_t i = 0; i < prevCount; i++)
						prev[i].bottom = y1;
					prevBottom = y1;
				} else {
					if (count + nspans > capacity) {
						int32_t newCapacity = capacity ? capacity * 2 : 16;
						while (newCapacity < count + nspans) newCapacity *= 2;
						IRect *grown = realloc(result, (size_t) newCapacity * sizeof(IRect));
						if (grown == NULL) {
							ok = 0;
						} else {
							result = grown;
							capacity = newCapacity;
						}
					}
					if (ok) {
						prevStart = count;
						prevCount = nspans;
						prevBottom = y1;
						for (int32_t i = 0; i < nspans; i++) {
							result[count].top = y0;
							result[count].bottom = y1;
							result[count].left = spans[2 * i];
							result[count].right = spans[2 * i + 1];
							count++;
						}
					}
				}
			}
		}
		free(sa.x);
		free(sb.x);
		free(pts);
		free(spans);
	}
	free(ys);
	if (!ok) {
		free(result);
		return 0;
	}
	*out = result;
	*outCount = count;
	return 1;
}

static void operate(RgnHandle srcA, RgnHandle srcB, RgnHandle dst, RegionOp op) {
	if (srcA == NULL || srcB == NULL || dst == NULL)
		return;
	RegionBlock *a = *handleOf(srcA), *b = *handleOf(srcB);
	IRect *result;
	int32_t count;
	if (!combine(a->rects, a->count, b->rects, b->count, op, &result, &count))
		return;
	store(dst, result, count);
	free(result);
}

void SectRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) { operate(srcRgnA, srcRgnB, dstRgn, OpSect); }
void UnionRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) { operate(srcRgnA, srcRgnB, dstRgn, OpUnion); }
void DiffRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) { operate(srcRgnA, srcRgnB, dstRgn, OpDiff); }
void XorRgn(RgnHandle srcRgnA, RgnHandle srcRgnB, RgnHandle dstRgn) { operate(srcRgnA, srcRgnB, dstRgn, OpXor); }

#pragma mark - Insetting

// The union of every rectangle grown by (dh, dv) on each side.
static int dilate(const IRect *rects, int32_t count, int32_t dh, int32_t dv, IRect **out, int32_t *outCount) {
	IRect *acc = NULL;
	int32_t accCount = 0;
	for (int32_t i = 0; i < count; i++) {
		IRect grown = {rects[i].top - dv, rects[i].left - dh, rects[i].bottom + dv, rects[i].right + dh};
		IRect *next;
		int32_t nextCount;
		if (!combine(acc, accCount, &grown, 1, OpUnion, &next, &nextCount)) {
			free(acc);
			return 0;
		}
		free(acc);
		acc = next;
		accCount = nextCount;
	}
	*out = acc;
	*outCount = accCount;
	return 1;
}

// Moves the region's edges inward by dh and dv (outward when negative).
//
// QUIRK: when the area vanishes, the bounds collapse to the center of the old
// bounds instead of becoming the all-zero rectangle: insetting a region
// bounded by top -5, left 3, bottom 19, right 27 by 100 leaves it empty with
// bounds top 7, left 15, bottom 7, right 15. Observed black-box on Mac OS X
// 10.14.4 (i386 and x86_64, tier 2).
// Test: tests/src/qd_regions.c (quirk_inset_to_nothing_collapses_bounds)
void InsetRgn(RgnHandle rgn, short dh, short dv) {
	if (rgn == NULL)
		return;
	RegionBlock *block = *handleOf(rgn);
	if (block->count == 0 || (dh == 0 && dv == 0))
		return;
	const RegionHeader oldHeader = block->header;
	IRect *current = malloc((size_t) block->count * sizeof(IRect));
	if (current == NULL)
		return;
	memcpy(current, block->rects, (size_t) block->count * sizeof(IRect));
	int32_t count = block->count;

	int32_t growH = dh < 0 ? -dh : 0, growV = dv < 0 ? -dv : 0;
	int32_t shrinkH = dh > 0 ? dh : 0, shrinkV = dv > 0 ? dv : 0;
	int ok = 1;

	if (growH > 0 || growV > 0) {
		IRect *grown;
		int32_t grownCount;
		if (dilate(current, count, growH, growV, &grown, &grownCount)) {
			free(current);
			current = grown;
			count = grownCount;
		} else {
			ok = 0;
		}
	}
	if (ok && (shrinkH > 0 || shrinkV > 0)) {
		// Erosion: what remains of the bounds, inset, once everything within
		// reach of the part of the bounds that is not in the region is taken away.
		IRect box = {oldHeader.rgnBBox.top, oldHeader.rgnBBox.left, oldHeader.rgnBBox.bottom, oldHeader.rgnBBox.right};
		IRect *outside = NULL, *outsideGrown = NULL, *eroded = NULL;
		int32_t outsideCount = 0, outsideGrownCount = 0, erodedCount = 0;
		IRect inner = {box.top + shrinkV, box.left + shrinkH, box.bottom - shrinkV, box.right - shrinkH};
		ok = combine(&box, 1, current, count, OpDiff, &outside, &outsideCount) &&
		     dilate(outside, outsideCount, shrinkH, shrinkV, &outsideGrown, &outsideGrownCount) &&
		     combine(&inner, inner.bottom > inner.top && inner.right > inner.left ? 1 : 0,
		             outsideGrown, outsideGrownCount, OpDiff, &eroded, &erodedCount);
		free(outside);
		free(outsideGrown);
		free(current);
		current = eroded;
		count = erodedCount;
	}
	if (ok) {
		store(rgn, current, count);
		if (count == 0) {
			RegionHeader *header = &(*handleOf(rgn))->header;
			short v = (short) ((oldHeader.rgnBBox.top + oldHeader.rgnBBox.bottom) / 2);
			short h = (short) ((oldHeader.rgnBBox.left + oldHeader.rgnBBox.right) / 2);
			header->rgnBBox.top = header->rgnBBox.bottom = v;
			header->rgnBBox.left = header->rgnBBox.right = h;
		}
	}
	free(current);
}

#pragma mark - Queries

Boolean EmptyRgn(RgnHandle rgn) {
	return rgn == NULL || (*handleOf(rgn))->count == 0;
}

Boolean EqualRgn(RgnHandle rgnA, RgnHandle rgnB) {
	if (rgnA == NULL || rgnB == NULL)
		return rgnA == rgnB;
	RegionBlock *a = *handleOf(rgnA), *b = *handleOf(rgnB);
	return a->count == b->count && memcmp(a->rects, b->rects, (size_t) a->count * sizeof(IRect)) == 0;
}

Boolean PtInRgn(Point pt, RgnHandle rgn) {
	if (rgn == NULL)
		return false;
	RegionBlock *block = *handleOf(rgn);
	for (int32_t i = 0; i < block->count; i++) {
		const IRect *r = &block->rects[i];
		if (pt.h >= r->left && pt.h < r->right && pt.v >= r->top && pt.v < r->bottom)
			return true;
	}
	return false;
}

// True when the rectangle shares area with the region itself, not just with
// its bounds.
Boolean RectInRgn(const Rect *r, RgnHandle rgn) {
	if (rgn == NULL || EmptyRect(r))
		return false;
	RegionBlock *block = *handleOf(rgn);
	for (int32_t i = 0; i < block->count; i++) {
		const IRect *c = &block->rects[i];
		if (r->left < c->right && r->right > c->left && r->top < c->bottom && r->bottom > c->top)
			return true;
	}
	return false;
}

Rect *GetRegionBounds(RgnHandle region, Rect *bounds) {
	if (region == NULL) {
		SetRect(bounds, 0, 0, 0, 0);
	} else {
		*bounds = (*handleOf(region))->header.rgnBBox;
	}
	return bounds;
}
