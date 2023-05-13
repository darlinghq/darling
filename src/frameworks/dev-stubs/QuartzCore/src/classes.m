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

#import <Foundation/Foundation.h>

@interface CAAnimation : NSObject
@end

@implementation CAAnimation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAAnimationGroup : NSObject
@end

@implementation CAAnimationGroup

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CABasicAnimation : NSObject
@end

@implementation CABasicAnimation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAConstraint : NSObject
@end

@implementation CAConstraint

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAConstraintLayoutManager : NSObject
@end

@implementation CAConstraintLayoutManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAFilter : NSObject
@end

@implementation CAFilter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAGradientLayer : NSObject
@end

@implementation CAGradientLayer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAKeyframeAnimation : NSObject
@end

@implementation CAKeyframeAnimation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CALayer : NSObject
@end

@implementation CALayer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CALayerHost : NSObject
@end

@implementation CALayerHost

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAMediaTimingFunction : NSObject
@end

@implementation CAMediaTimingFunction

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAMetalLayer : NSObject
@end

@implementation CAMetalLayer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CARenderer : NSObject
@end

@implementation CARenderer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CAShapeLayer : NSObject
@end

@implementation CAShapeLayer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CASpringAnimation : NSObject
@end

@implementation CASpringAnimation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CATextLayer : NSObject
@end

@implementation CATextLayer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CATransaction : NSObject
@end

@implementation CATransaction

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CATransformLayer : NSObject
@end

@implementation CATransformLayer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

@interface CATransition : NSObject
@end

@implementation CATransition

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end

