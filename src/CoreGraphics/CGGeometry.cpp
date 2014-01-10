#include "CGGeometry.h"
#include <limits>
#include <cmath>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>

const CGRect CGRectInfinite = { CGPoint { -CGFLOAT_MAX / 2, -CGFLOAT_MAX / 2 }, CGSize { CGFLOAT_MAX, CGFLOAT_MAX } };
const CGPoint CGPointZero = { 0, 0 };
const CGSize CGSizeZero = { 0, 0 };
const CGRect CGRectZero = { CGPointZero, CGSizeZero };
const CGRect CGRectNull = { { std::numeric_limits<CGFloat>::infinity(), std::numeric_limits<CGFloat>::infinity() }, CGSizeZero };

inline CGFloat cgabs(CGFloat a) { return a > 0 ? a : -a; }

CGPoint CGPointMake(CGFloat x, CGFloat y)
{
	return CGPoint { x, y };
}

CGSize CGSizeMake(CGFloat width, CGFloat height)
{
	return CGSize { width, height };
}

CGRect CGRectMake(CGFloat x, CGFloat y, CGFloat width, CGFloat height)
{
	return CGRect { CGPoint { x, y }, CGSize { width, height } };
}

CGVector CGVectorMake(CGFloat dx, CGFloat dy)
{
	return CGVector { dx, dy };
}

bool CGSizeEqualToSize(CGSize size1, CGSize size2)
{
	return cgabs(size1.width - size2.width) < CGFLOAT_EPSILON
		&& cgabs(size1.height - size2.height) < CGFLOAT_EPSILON;
}

bool CGPointEqualToPoint(CGPoint point1, CGPoint point2)
{
	return cgabs(point1.x - point2.x) < CGFLOAT_EPSILON
		&& cgabs(point1.y - point2.y) < CGFLOAT_EPSILON;
}

bool CGRectEqualToRect(CGRect rect1, CGRect rect2)
{
	return CGSizeEqualToSize(rect1.size, rect2.size)
		&& CGPointEqualToPoint(rect1.origin, rect2.origin);
}

CGFloat CGRectGetHeight(CGRect rect)
{
	return cgabs(rect.size.height);
}

CGFloat CGRectGetWidth(CGRect rect)
{
	return cgabs(rect.size.width);
}

CGFloat CGRectGetMaxX(CGRect rect)
{
	return (rect.size.width > 0) ? (rect.origin.x + rect.size.width) : (rect.origin.x);
}

CGFloat CGRectGetMaxY(CGRect rect)
{
	return (rect.size.height > 0) ? (rect.origin.y + rect.size.height) : (rect.origin.y);
}

CGFloat CGRectGetMidY(CGRect rect)
{
	return rect.origin.y + rect.size.height/2;
}

CGFloat CGRectGetMidX(CGRect rect)
{
	return rect.origin.x + rect.size.width/2;
}

CGFloat CGRectGetMinX(CGRect rect)
{
	return (rect.size.width > 0) ? (rect.origin.x) : (rect.origin.x + rect.size.width);
}

CGFloat CGRectGetMinY(CGRect rect)
{
	return (rect.size.height > 0) ? (rect.origin.y) : (rect.origin.y + rect.size.height);
}

bool CGRectIsNull(CGRect rect)
{
	return rect.origin.x == std::numeric_limits<CGFloat>::infinity()
		&& rect.origin.y == std::numeric_limits<CGFloat>::infinity();
}

bool CGRectIsInfinite(CGRect rect)
{
	return rect.origin.x == CGRectInfinite.origin.x
		&& rect.origin.y == CGRectInfinite.origin.y
		&& rect.size.width == CGRectInfinite.size.width
		&& rect.size.height == CGRectInfinite.size.height;
}

bool CGRectIsEmpty(CGRect rect)
{
	return rect.size.height == 0 && rect.size.width == 0;
}

bool CGRectContainsPoint(CGRect rect, CGPoint point)
{
	return point.x >= rect.origin.x && point.x < rect.origin.x+rect.size.width
		&& point.y >= rect.origin.y && point.y < rect.origin.y+rect.size.height;
}

bool CGRectContainsRect(CGRect rect1, CGRect rect2)
{
	return CGRectContainsPoint(rect1, rect2.origin)
		&& CGRectContainsPoint(rect1, CGPoint { rect2.origin.x+rect2.size.width, rect2.origin.y+rect2.size.height });
}

CGRect CGRectStandardize(CGRect rect)
{
	if (rect.size.width < 0)
	{
		rect.origin.x += rect.size.width;
		rect.size.width = -rect.size.width;
	}
	
	if (rect.size.height < 0)
	{
		rect.origin.y += rect.size.height;
		rect.size.height = -rect.size.height;
	}
	
	return rect;
}

CGRect CGRectOffset(CGRect rect, CGFloat dx, CGFloat dy)
{
	return CGRect { CGPoint { rect.origin.x+dx, rect.origin.y+dy }, rect.size };
}

CGRect CGRectIntegral(CGRect rect)
{
	return CGRect {
		CGPoint { std::floor(rect.origin.x), std::floor(rect.origin.y) },
		CGSize { std::ceil(rect.size.width), std::ceil(rect.size.height) }
	};
}

CFDictionaryRef CGPointCreateDictionaryRepresentation(CGPoint point)
{
	CFNumberRef values[2];
	static const CFStringRef keys[] = { CFSTR("X"), CFSTR("Y") };
	CFDictionaryRef dict;
	
	values[0] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &point.x);
	values[1] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &point.y);
	
	dict = CFDictionaryCreate(nullptr, (const void**) keys, (const void**) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFRelease(values[0]);
	CFRelease(values[1]);
	
	return dict;
}

CFDictionaryRef CGSizeCreateDictionaryRepresentation(CGSize size)
{
	CFNumberRef values[2];
	static const CFStringRef keys[] = { CFSTR("Width"), CFSTR("Height") };
	CFDictionaryRef dict;
	
	values[0] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &size.width);
	values[1] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &size.height);
	
	dict = CFDictionaryCreate(nullptr, (const void**) keys, (const void**) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFRelease(values[0]);
	CFRelease(values[1]);
	
	return dict;
}

CFDictionaryRef CGRectCreateDictionaryRepresentation(CGRect rect)
{
	CFNumberRef values[4];
	static const CFStringRef keys[] = { CFSTR("X"), CFSTR("Y"), CFSTR("Width"), CFSTR("Height") };
	CFDictionaryRef dict;
	
	values[0] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &rect.origin.x);
	values[1] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &rect.origin.y);
	values[2] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &rect.size.width);
	values[3] = CFNumberCreate(nullptr, kCFNumberCGFloatType, &rect.size.height);
	
	dict = CFDictionaryCreate(nullptr, (const void**) keys, (const void**) values, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFRelease(values[0]);
	CFRelease(values[1]);
	CFRelease(values[2]);
	CFRelease(values[3]);
	
	return dict;
}

bool CGPointMakeWithDictionaryRepresentation(CFDictionaryRef dict, CGPoint *point)
{
	CFNumberRef x, y;
	
	if (!point)
		return false;
	
	x = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("X"));
	y = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("Y"));
	
	if (!x || !y)
		return false;
	
	if (CFGetTypeID(x) != CFNumberGetTypeID() || CFGetTypeID(y) != CFNumberGetTypeID())
		return false;
	
	return CFNumberGetValue(x, kCFNumberCGFloatType, &point->x)
		&& CFNumberGetValue(y, kCFNumberCGFloatType, &point->y);
}

bool CGSizeMakeWithDictionaryRepresentation(CFDictionaryRef dict, CGSize *size)
{
	CFNumberRef w, h;
	
	if (!size)
		return false;
	
	w = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("Width"));
	h = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("Height"));
	
	if (!w || !h)
		return false;
	
	if (CFGetTypeID(w) != CFNumberGetTypeID() || CFGetTypeID(h) != CFNumberGetTypeID())
		return false;
	
	return CFNumberGetValue(w, kCFNumberCGFloatType, &size->width)
		&& CFNumberGetValue(h, kCFNumberCGFloatType, &size->height);
}

bool CGRectMakeWithDictionaryRepresentation(CFDictionaryRef dict, CGRect *rect)
{
	CFNumberRef x, y, w, h;
	
	if (!rect)
		return false;
	
	x = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("X"));
	y = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("Y"));
	w = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("Width"));
	h = (CFNumberRef) CFDictionaryGetValue(dict, CFSTR("Height"));
	
	if (!x || !y || !w || !h)
		return false;
	
	if (CFGetTypeID(x) != CFNumberGetTypeID() || CFGetTypeID(y) != CFNumberGetTypeID()
		|| CFGetTypeID(w) != CFNumberGetTypeID() || CFGetTypeID(h) != CFNumberGetTypeID())
	{
		return false;
	}
	
	return CFNumberGetValue(x, kCFNumberCGFloatType, &rect->origin.x)
		&& CFNumberGetValue(y, kCFNumberCGFloatType, &rect->origin.y)
		&& CFNumberGetValue(w, kCFNumberCGFloatType, &rect->size.width)
		&& CFNumberGetValue(h, kCFNumberCGFloatType, &rect->size.height);
}


