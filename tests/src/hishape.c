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

// HIShape: rectangular shapes created from a rectangle, an empty shape and
// copies, with the bounds, point and rectangle queries.

#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void test_hishape(void)
{
	CGRect r = CGRectMake(1, 2, 30, 40);
	HIShapeRef shape = HIShapeCreateWithRect(&r);
	CHECK(shape != NULL);
	if (shape == NULL)
		return;
	CHECK(CFGetTypeID(shape) == HIShapeGetTypeID());
	CGRect b;
	CHECK(HIShapeGetBounds(shape, &b) == &b && CGRectEqualToRect(b, r));
	CHECK(HIShapeIsRectangular(shape) && !HIShapeIsEmpty(shape));
	CGPoint in = CGPointMake(5, 5), out = CGPointMake(100, 5);
	CHECK(HIShapeContainsPoint(shape, &in) && !HIShapeContainsPoint(shape, &out));
	CGRect overlap = CGRectMake(20, 20, 100, 100), apart = CGRectMake(200, 200, 5, 5);
	CHECK(HIShapeIntersectsRect(shape, &overlap) && !HIShapeIntersectsRect(shape, &apart));
	HIShapeRef copy = HIShapeCreateCopy(shape);
	CHECK(copy != NULL && copy != shape);
	if (copy != NULL)
	{
		CHECK(HIShapeGetBounds(copy, &b) != NULL && CGRectEqualToRect(b, r));
		CFRelease(copy);
	}
	CFRelease(shape);

	HIShapeRef empty = HIShapeCreateEmpty();
	CHECK(empty != NULL && HIShapeIsEmpty(empty));
	if (empty != NULL)
	{
		CHECK(!HIShapeContainsPoint(empty, &in));
		CFRelease(empty);
	}
	CHECK(HIShapeCreateWithRect(NULL) == NULL);
	CHECK(HIShapeCreateCopy(NULL) == NULL);
}

int main(void)
{
	test_hishape();
	if (g_failures != 0)
	{
		printf("hishape: %d failure(s)\n", g_failures);
		return 1;
	}
	printf("PASS hishape\n");
	return 0;
}
