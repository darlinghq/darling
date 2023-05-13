/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
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

#import <objc/NSObject.h>

@interface NSString : NSObject
@end

#ifdef __LP64__
	typedef double CGFloat;
#else
	typedef float CGFloat;
#endif

typedef struct {
	CGFloat m11, m12, m13, m14;
	CGFloat m21, m22, m23, m24;
	CGFloat m31, m32, m33, m34;
	CGFloat m41, m42, m43, m44;
} CATransform3D;

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf

const CATransform3D CATransform3DIdentity = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
NSString *const kCAAlignmentCenter = @"kCAAlignmentCenter";
NSString *const kCAAlignmentLeft = @"kCAAlignmentLeft";
NSString *const kCAAlignmentRight = @"kCAAlignmentRight";
NSString *const kCAAnimationCubic = @"kCAAnimationCubic";
NSString *const kCAAnimationCubicPaced = @"kCAAnimationCubicPaced";
NSString *const kCAAnimationLinear = @"kCAAnimationLinear";
NSString *const kCAFillModeBoth = @"kCAFillModeBoth";
NSString *const kCAFillModeForwards = @"kCAFillModeForwards";
NSString *const kCAFillRuleEvenOdd = @"kCAFillRuleEvenOdd";
NSString *const kCAFilterColorMatrix = @"kCAFilterColorMatrix";
NSString *const kCAFilterLinear = @"kCAFilterLinear";
NSString *const kCAFilterNearest = @"kCAFilterNearest";
NSString *const kCAFilterTrilinear = @"kCAFilterTrilinear";
NSString *const kCAGravityCenter = @"kCAGravityCenter";
NSString *const kCAGravityLeft = @"kCAGravityLeft";
NSString *const kCAGravityResize = @"kCAGravityResize";
NSString *const kCAGravityResizeAspect = @"kCAGravityResizeAspect";
NSString *const kCAGravityResizeAspectFill = @"kCAGravityResizeAspectFill";
NSString *const kCAGravityTopLeft = @"kCAGravityTopLeft";
NSString *const kCALineCapSquare = @"kCALineCapSquare";
NSString *const kCAMediaTimingFunctionEaseIn = @"kCAMediaTimingFunctionEaseIn";
NSString *const kCAMediaTimingFunctionEaseInEaseOut = @"kCAMediaTimingFunctionEaseInEaseOut";
NSString *const kCAMediaTimingFunctionEaseOut = @"kCAMediaTimingFunctionEaseOut";
NSString *const kCAMediaTimingFunctionLinear = @"kCAMediaTimingFunctionLinear";
NSString *const kCARendererColorSpace = @"kCARendererColorSpace";
NSString *const kCATransactionDisableActions = @"kCATransactionDisableActions";
NSString *const kCATransitionFade = @"kCATransitionFade";
NSString *const kCATransitionFromBottom = @"kCATransitionFromBottom";
NSString *const kCATransitionFromLeft = @"kCATransitionFromLeft";
NSString *const kCATransitionFromRight = @"kCATransitionFromRight";
NSString *const kCATransitionFromTop = @"kCATransitionFromTop";
NSString *const kCATransitionMoveIn = @"kCATransitionMoveIn";
NSString *const kCATransitionPush = @"kCATransitionPush";
NSString *const kCATransitionReveal = @"kCATransitionReveal";
NSString *const kCATruncationEnd = @"kCATruncationEnd";
NSString *const kCATruncationMiddle = @"kCATruncationMiddle";

void* CACurrentMediaTime(void) {
	if (verbose) LOG_FUNC("STUB: CACurrentMediaTime called\n");
	return NULL;
};

void* CATransform3DConcat(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DConcat called\n");
	return NULL;
};

void* CATransform3DInvert(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DInvert called\n");
	return NULL;
};

void* CATransform3DMakeRotation(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DMakeRotation called\n");
	return NULL;
};

void* CATransform3DMakeScale(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DMakeScale called\n");
	return NULL;
};

void* CATransform3DMakeTranslation(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DMakeTranslation called\n");
	return NULL;
};

void* CATransform3DRotate(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DRotate called\n");
	return NULL;
};

void* CATransform3DScale(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DScale called\n");
	return NULL;
};

void* CATransform3DTranslate(void) {
	if (verbose) LOG_FUNC("STUB: CATransform3DTranslate called\n");
	return NULL;
};


