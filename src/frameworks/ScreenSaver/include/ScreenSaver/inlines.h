#ifndef _SCREENSAVER_INLINES_H_
#define _SCREENSAVER_INLINES_H_

#import <Foundation/Foundation.h>
#import <CoreGraphics/CGBase.h>

__BEGIN_DECLS

static NSRect SSCenteredRectInRect(NSRect inner, NSRect outer) {
#if CGFLOAT_IS_DOUBLE
	#define SSCenteredRectInRect_floor floor
#else
	#define SSCenteredRectInRect_floor floorf
#endif
	inner.origin.x = outer.origin.x + SSCenteredRectInRect_floor((outer.size.width  - inner.size.width ) / 2);
	inner.origin.y = outer.origin.y + SSCenteredRectInRect_floor((outer.size.height - inner.size.height) / 2);
	return inner;
};

static int SSRandomIntBetween(int a, int b) {
	int range = (a > b) ? (a - b) : (b - a);
	int result = range * ((float)random() / (float)RAND_MAX);
	return a + result;
};

static CGFloat SSRandomFloatBetween(CGFloat a, CGFloat b) {
	CGFloat range = (a > b) ? (a - b) : (b - a);
	CGFloat result = range * ((CGFloat)random() / (CGFloat)RAND_MAX);
	return a + result;
};

static NSPoint SSRandomPointForSizeWithinRect(NSSize size, NSRect rect) {
	return NSMakePoint(
		SSRandomFloatBetween(rect.origin.x, rect.origin.x + rect.size.width - size.width),
		SSRandomFloatBetween(rect.origin.y, rect.origin.y + rect.size.height - size.height)
	);
};

__END_DECLS

#endif // _SCREENSAVER_INLINES_H_
