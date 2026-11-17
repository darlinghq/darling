/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// HIShape holds a region as a set of rectangles. This implementation keeps a
// single rectangle, which covers every shape the system frameworks hand out
// (control and window geometry); an empty shape is a rectangle of zero area.

#include <HIServices/HIShape.h>
#include <CoreFoundation/CFRuntime.h>
#include <pthread.h>
#include <stddef.h>

struct __HIShape {
	CFRuntimeBase base;
	CGRect rect;
};

static const CFRuntimeClass hiShapeClass = {
	0,
	"HIShape",
	NULL, // init
	NULL, // copy
	NULL, // finalize
	NULL, // equal
	NULL, // hash
	NULL, // copyFormattingDesc
	NULL, // copyDebugDesc
};

static CFTypeID hiShapeTypeID;
static pthread_once_t hiShapeOnce = PTHREAD_ONCE_INIT;

static void registerShapeClass(void)
{
	hiShapeTypeID = _CFRuntimeRegisterClass(&hiShapeClass);
}

CFTypeID HIShapeGetTypeID(void)
{
	pthread_once(&hiShapeOnce, registerShapeClass);
	return hiShapeTypeID;
}

static HIShapeRef createShape(CGRect rect)
{
	struct __HIShape *shape = (struct __HIShape *) _CFRuntimeCreateInstance(kCFAllocatorDefault, HIShapeGetTypeID(),
		sizeof(struct __HIShape) - sizeof(CFRuntimeBase), NULL);
	if (shape != NULL)
		shape->rect = rect;
	return shape;
}

HIShapeRef HIShapeCreateEmpty(void)
{
	return createShape(CGRectZero);
}

HIShapeRef HIShapeCreateWithRect(const CGRect *inRect)
{
	if (inRect == NULL)
		return NULL;
	CGRect r = *inRect;
	if (CGRectIsNull(r) || CGRectIsEmpty(r))
		r = CGRectZero;
	else
		r = CGRectStandardize(r);
	return createShape(r);
}

HIShapeRef HIShapeCreateCopy(HIShapeRef inShape)
{
	if (inShape == NULL)
		return NULL;
	return createShape(inShape->rect);
}

Boolean HIShapeIsEmpty(HIShapeRef inShape)
{
	return inShape == NULL || CGRectIsEmpty(inShape->rect);
}

Boolean HIShapeIsRectangular(HIShapeRef inShape)
{
	return inShape != NULL;
}

CGRect *HIShapeGetBounds(HIShapeRef inShape, CGRect *outRect)
{
	if (outRect == NULL)
		return NULL;
	*outRect = inShape != NULL ? inShape->rect : CGRectZero;
	return outRect;
}

Boolean HIShapeContainsPoint(HIShapeRef inShape, const CGPoint *inPoint)
{
	if (inShape == NULL || inPoint == NULL || CGRectIsEmpty(inShape->rect))
		return false;
	return CGRectContainsPoint(inShape->rect, *inPoint);
}

Boolean HIShapeIntersectsRect(HIShapeRef inShape, const CGRect *inRect)
{
	if (inShape == NULL || inRect == NULL || CGRectIsEmpty(inShape->rect) || CGRectIsEmpty(*inRect))
		return false;
	return CGRectIntersectsRect(inShape->rect, *inRect);
}
