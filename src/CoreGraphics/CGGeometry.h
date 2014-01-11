#ifndef CGGEOMETRY_H_
#define CGGEOMETRY_H_
#include "CGBase.h"
#include <float.h>
#include <stdbool.h>
#include <CoreFoundation/CFDictionary.h>

BEGIN_EXTERN_C

struct CGPoint
{
	CGFloat x;
	CGFloat y;
};
typedef struct CGPoint CGPoint;

struct CGSize
{
	CGFloat width;
	CGFloat height;
};
typedef struct CGSize CGSize;

struct CGVector
{
	CGFloat dx;
	CGFloat dy;
};
typedef struct CGVector CGVector;

struct CGRect
{
	CGPoint origin;
	CGSize size;
};
typedef struct CGRect CGRect;

enum CGRectEdge
{
	CGRectMinXEdge,
	CGRectMinYEdge,
	CGRectMaxXEdge,
	CGRectMaxYEdge
};
typedef enum CGRectEdge CGRectEdge;

extern const CGRect CGRectInfinite;
extern const CGPoint CGPointZero;
extern const CGRect CGRectZero;
extern const CGSize CGSizeZero;
extern const CGRect CGRectNull;

CGPoint CGPointMake(CGFloat x, CGFloat y);
CGSize CGSizeMake(CGFloat width, CGFloat height);
CGRect CGRectMake(CGFloat x, CGFloat y, CGFloat width, CGFloat height);
CGVector CGVectorMake(CGFloat dx, CGFloat dy);

bool CGSizeEqualToSize(CGSize size1, CGSize size2);
bool CGPointEqualToPoint(CGPoint point1, CGPoint point2);
bool CGRectEqualToRect(CGRect rect1, CGRect rect2);

CFDictionaryRef CGPointCreateDictionaryRepresentation(CGPoint point);
CFDictionaryRef CGSizeCreateDictionaryRepresentation(CGSize size);
CFDictionaryRef CGRectCreateDictionaryRepresentation(CGRect rect);

bool CGPointMakeWithDictionaryRepresentation(CFDictionaryRef dict, CGPoint *point);
bool CGSizeMakeWithDictionaryRepresentation(CFDictionaryRef dict, CGSize *size);
bool CGRectMakeWithDictionaryRepresentation(CFDictionaryRef dict, CGRect *rect);

CGFloat CGRectGetHeight(CGRect rect);
CGFloat CGRectGetWidth(CGRect rect);
CGFloat CGRectGetMaxX(CGRect rect);
CGFloat CGRectGetMaxY(CGRect rect);
CGFloat CGRectGetMinX(CGRect rect);
CGFloat CGRectGetMinY(CGRect rect);
CGFloat CGRectGetMidX(CGRect rect);
CGFloat CGRectGetMidY(CGRect rect);
bool CGRectIsNull(CGRect rect);
bool CGRectIsInfinite(CGRect rect);
bool CGRectContainsPoint(CGRect rect, CGPoint point);
bool CGRectContainsRect(CGRect rect1, CGRect rect2);

CGRect CGRectStandardize(CGRect rect);
CGRect CGRectOffset(CGRect rect, CGFloat dx, CGFloat dy);
CGRect CGRectIntegral(CGRect rect);
CGRect CGRectIntersection(CGRect r1, CGRect r2);
CGRect CGRectInset(CGRect rect, CGFloat dx, CGFloat dy);
CGRect CGRectUnion(CGRect r1, CGRect r2);
void CGRectDivide(CGRect rect, CGRect* slice, CGRect* remainder, CGFloat amount, CGRectEdge edge);

END_EXTERN_C

#endif

