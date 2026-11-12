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
#import <darling-stub-forwarding.h>

@interface NSATSTypesetter : NSObject
@end

@implementation NSATSTypesetter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAccessibilityCustomAction : NSObject
@end

@implementation NSAccessibilityCustomAction

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAccessibilityCustomChooser : NSObject
@end

@implementation NSAccessibilityCustomChooser

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAccessibilityCustomChooserItemResult : NSObject
@end

@implementation NSAccessibilityCustomChooserItemResult

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAccessibilityCustomRotor : NSObject
@end

@implementation NSAccessibilityCustomRotor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAccessibilityCustomRotorItemResult : NSObject
@end

@implementation NSAccessibilityCustomRotorItemResult

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAccessibilityElement : NSObject
@end

@implementation NSAccessibilityElement

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSActionCell : NSObject
@end

@implementation NSActionCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAlert : NSObject
@end

@implementation NSAlert

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAlignmentFeedbackFilter : NSObject
@end

@implementation NSAlignmentFeedbackFilter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAnimation : NSObject
@end

@implementation NSAnimation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAnimationContext : NSObject
@end

@implementation NSAnimationContext

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSAppearance : NSObject
@end

@implementation NSAppearance

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSApplication : NSObject
@end

@implementation NSApplication

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSArrayController : NSObject
@end

@implementation NSArrayController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSBezierPath : NSObject
@end

@implementation NSBezierPath

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSBitmapImageRep : NSObject
@end

@implementation NSBitmapImageRep

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSBox : NSObject
@end

@implementation NSBox

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSBrowser : NSObject
@end

@implementation NSBrowser

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSButton : NSObject
@end

@implementation NSButton

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSButtonCell : NSObject
@end

@implementation NSButtonCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSButtonGroupTouchBarItem : NSObject
@end

@implementation NSButtonGroupTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCIImageRep : NSObject
@end

@implementation NSCIImageRep

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCachedImageRep : NSObject
@end

@implementation NSCachedImageRep

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCarbonMenuImpl : NSObject
@end

@implementation NSCarbonMenuImpl

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCatalogColor : NSObject
@end

@implementation NSCatalogColor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCell : NSObject
@end

@implementation NSCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSClassSwapper : NSObject
@end

@implementation NSClassSwapper

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSClickGestureRecognizer : NSObject
@end

@implementation NSClickGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSClipView : NSObject
@end

@implementation NSClipView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionView : NSObject
@end

@implementation NSCollectionView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewFlowLayout : NSObject
@end

@implementation NSCollectionViewFlowLayout

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewFlowLayoutInvalidationContext : NSObject
@end

@implementation NSCollectionViewFlowLayoutInvalidationContext

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewGridLayout : NSObject
@end

@implementation NSCollectionViewGridLayout

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewItem : NSObject
@end

@implementation NSCollectionViewItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewLayout : NSObject
@end

@implementation NSCollectionViewLayout

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewLayoutAttributes : NSObject
@end

@implementation NSCollectionViewLayoutAttributes

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCollectionViewLayoutInvalidationContext : NSObject
@end

@implementation NSCollectionViewLayoutInvalidationContext

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColor : NSObject
@end

@implementation NSColor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColorList : NSObject
@end

@implementation NSColorList

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColorPanel : NSObject
@end

@implementation NSColorPanel

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColorPickerTouchBarItem : NSObject
@end

@implementation NSColorPickerTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColorSampler : NSObject
@end

@implementation NSColorSampler

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColorSpace : NSObject
@end

@implementation NSColorSpace

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSColorWell : NSObject
@end

@implementation NSColorWell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

- (id)activate: (id)activate
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}

- (id)deactivate
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}

@end

@interface NSComboBox : NSObject
@end

@implementation NSComboBox

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSComboBoxCell : NSObject
@end

@implementation NSComboBoxCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSConcreteTextStorage : NSObject
@end

@implementation NSConcreteTextStorage

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSControl : NSObject
@end

@implementation NSControl

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSController : NSObject
@end

@implementation NSController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCoreTypesetter : NSObject
@end

@implementation NSCoreTypesetter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCursor : NSObject
@end

@implementation NSCursor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCustomImageRep : NSObject
@end

@implementation NSCustomImageRep

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCustomObject : NSObject
@end

@implementation NSCustomObject

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCustomResource : NSObject
@end

@implementation NSCustomResource

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCustomTouchBarItem : NSObject
@end

@implementation NSCustomTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSCustomView : NSObject
@end

@implementation NSCustomView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDataAsset : NSObject
@end

@implementation NSDataAsset

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDatePicker : NSObject
@end

@implementation NSDatePicker

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDatePickerCell : NSObject
@end

@implementation NSDatePickerCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDictionaryController : NSObject
@end

@implementation NSDictionaryController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDocument : NSObject
@end

@implementation NSDocument

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDocumentController : NSObject
@end

@implementation NSDocumentController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDraggingItem : NSObject
@end

@implementation NSDraggingItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDrawer : NSObject
@end

@implementation NSDrawer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSDynamicNamedColor : NSObject
@end

@implementation NSDynamicNamedColor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSEvent : NSObject
@end

@implementation NSEvent

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSFont : NSObject
@end

@implementation NSFont

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSFontDescriptor : NSObject
@end

@implementation NSFontDescriptor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSFontManager : NSObject
@end

@implementation NSFontManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSFontPanel : NSObject
@end

@implementation NSFontPanel

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSForm : NSObject
@end

@implementation NSForm

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSFormCell : NSObject
@end

@implementation NSFormCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGestureRecognizer : NSObject
@end

@implementation NSGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGlyphGenerator : NSObject
@end

@implementation NSGlyphGenerator

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGradient : NSObject
@end

@implementation NSGradient

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGraphicsContext : NSObject
@end

@implementation NSGraphicsContext

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGridCell : NSObject
@end

@implementation NSGridCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGridColumn : NSObject
@end

@implementation NSGridColumn

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGridRow : NSObject
@end

@implementation NSGridRow

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGridView : NSObject
@end

@implementation NSGridView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSGroupTouchBarItem : NSObject
@end

@implementation NSGroupTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSHapticFeedbackManager : NSObject
@end

@implementation NSHapticFeedbackManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSHelpManager : NSObject
@end

@implementation NSHelpManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSIBHelpConnector : NSObject
@end

@implementation NSIBHelpConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSIBObjectData : NSObject
@end

@implementation NSIBObjectData

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSIBUserDefinedRuntimeAttributesConnector : NSObject
@end

@implementation NSIBUserDefinedRuntimeAttributesConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSImage : NSObject
@end

@implementation NSImage

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSImageCell : NSObject
@end

@implementation NSImageCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSImageRep : NSObject
@end

@implementation NSImageRep

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSImageView : NSObject
@end

@implementation NSImageView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSImmediateActionGestureRecognizer : NSObject
@end

@implementation NSImmediateActionGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSKeyBindingManager : NSObject
@end

@implementation NSKeyBindingManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSLayoutGuide : NSObject
@end

@implementation NSLayoutGuide

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSLayoutManager : NSObject
@end

@implementation NSLayoutManager

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSLevelIndicator : NSObject
@end

@implementation NSLevelIndicator

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSLevelIndicatorCell : NSObject
@end

@implementation NSLevelIndicatorCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSMagnificationGestureRecognizer : NSObject
@end

@implementation NSMagnificationGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSMatrix : NSObject
@end

@implementation NSMatrix

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSMenu : NSObject
@end

@implementation NSMenu

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSMenuItem : NSObject
@end

@implementation NSMenuItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSMenuItemCell : NSObject
@end

@implementation NSMenuItemCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSMutableParagraphStyle : NSObject
@end

@implementation NSMutableParagraphStyle

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNib : NSObject
@end

@implementation NSNib

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibAXAttributeConnector : NSObject
@end

@implementation NSNibAXAttributeConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibAXRelationshipConnector : NSObject
@end

@implementation NSNibAXRelationshipConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibAuxiliaryActionConnector : NSObject
@end

@implementation NSNibAuxiliaryActionConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibBindingConnector : NSObject
@end

@implementation NSNibBindingConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibControlConnector : NSObject
@end

@implementation NSNibControlConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibExternalObjectPlaceholder : NSObject
@end

@implementation NSNibExternalObjectPlaceholder

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSNibOutletConnector : NSObject
@end

@implementation NSNibOutletConnector

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSObjectController : NSObject
@end

@implementation NSObjectController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSOpenGLContext : NSObject
@end

@implementation NSOpenGLContext

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSOpenGLPixelFormat : NSObject
@end

@implementation NSOpenGLPixelFormat

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSOpenGLView : NSObject
@end

@implementation NSOpenGLView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSOpenPanel : NSObject
@end

@implementation NSOpenPanel

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSOutlineView : NSObject
@end

@implementation NSOutlineView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPDFImageRep : NSObject
@end

@implementation NSPDFImageRep

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPageController : NSObject
@end

@implementation NSPageController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPanGestureRecognizer : NSObject
@end

@implementation NSPanGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPanel : NSObject
@end

@implementation NSPanel

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSParagraphStyle : NSObject
@end

@implementation NSParagraphStyle

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPasteboard : NSObject
@end

@implementation NSPasteboard

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPasteboardItem : NSObject
@end

@implementation NSPasteboardItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPathCell : NSObject
@end

@implementation NSPathCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPathComponentCell : NSObject
@end

@implementation NSPathComponentCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPathControl : NSObject
@end

@implementation NSPathControl

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPopUpButton : NSObject
@end

@implementation NSPopUpButton

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPopUpButtonCell : NSObject
@end

@implementation NSPopUpButtonCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPopover : NSObject
@end

@implementation NSPopover

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPopoverTouchBarItem : NSObject
@end

@implementation NSPopoverTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPredicateEditor : NSObject
@end

@implementation NSPredicateEditor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPredicateEditorRowTemplate : NSObject
@end

@implementation NSPredicateEditorRowTemplate

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPressGestureRecognizer : NSObject
@end

@implementation NSPressGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPressureConfiguration : NSObject
@end

@implementation NSPressureConfiguration

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPrintInfo : NSObject
@end

@implementation NSPrintInfo

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPrintOperation : NSObject
@end

@implementation NSPrintOperation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSProgressIndicator : NSObject
@end

@implementation NSProgressIndicator

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSPulseGestureRecognizer : NSObject
@end

@implementation NSPulseGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSResponder : NSObject
@end

@implementation NSResponder

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSRotationGestureRecognizer : NSObject
@end

@implementation NSRotationGestureRecognizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSRuleEditor : NSObject
@end

@implementation NSRuleEditor

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSRulerView : NSObject
@end

@implementation NSRulerView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSRunningApplication : NSObject
@end

@implementation NSRunningApplication

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSavePanel : NSObject
@end

@implementation NSSavePanel

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScreen : NSObject
@end

@implementation NSScreen

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScrollView : NSObject
@end

@implementation NSScrollView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScroller : NSObject
@end

@implementation NSScroller

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScrollerImpPair : NSObject
@end

@implementation NSScrollerImpPair

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScrubber : NSObject
@end

@implementation NSScrubber

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScrubberFlowLayout : NSObject
@end

@implementation NSScrubberFlowLayout

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScrubberLayout : NSObject
@end

@implementation NSScrubberLayout

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSScrubberProportionalLayout : NSObject
@end

@implementation NSScrubberProportionalLayout

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSearchField : NSObject
@end

@implementation NSSearchField

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSearchFieldCell : NSObject
@end

@implementation NSSearchFieldCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSecureTextField : NSObject
@end

@implementation NSSecureTextField

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSecureTextFieldCell : NSObject
@end

@implementation NSSecureTextFieldCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSegmentedCell : NSObject
@end

@implementation NSSegmentedCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSegmentedControl : NSObject
@end

@implementation NSSegmentedControl

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSShadow : NSObject
@end

@implementation NSShadow

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSharingService : NSObject
@end

@implementation NSSharingService

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSharingServicePicker : NSObject
@end

@implementation NSSharingServicePicker

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSharingServicePickerTouchBarItem : NSObject
@end

@implementation NSSharingServicePickerTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSlider : NSObject
@end

@implementation NSSlider

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSliderAccessory : NSObject
@end

@implementation NSSliderAccessory

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSliderAccessoryBehavior : NSObject
@end

@implementation NSSliderAccessoryBehavior

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSliderCell : NSObject
@end

@implementation NSSliderCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSliderTouchBarItem : NSObject
@end

@implementation NSSliderTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSound : NSObject
@end

@implementation NSSound

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSpaceTouchBarItem : NSObject
@end

@implementation NSSpaceTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSpeechSynthesizer : NSObject
@end

@implementation NSSpeechSynthesizer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSpellChecker : NSObject
@end

@implementation NSSpellChecker

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSplitView : NSObject
@end

@implementation NSSplitView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSplitViewController : NSObject
@end

@implementation NSSplitViewController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSplitViewItem : NSObject
@end

@implementation NSSplitViewItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSStackView : NSObject
@end

@implementation NSStackView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSStatusBar : NSObject
@end

@implementation NSStatusBar

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSStepper : NSObject
@end

@implementation NSStepper

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSStepperCell : NSObject
@end

@implementation NSStepperCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSStoryboard : NSObject
@end

@implementation NSStoryboard

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSStoryboardControllerPlaceholder : NSObject
@end

@implementation NSStoryboardControllerPlaceholder

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSSwitch : NSObject
@end

@implementation NSSwitch

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTabView : NSObject
@end

@implementation NSTabView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTabViewController : NSObject
@end

@implementation NSTabViewController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTabViewItem : NSObject
@end

@implementation NSTabViewItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableCellView : NSObject
@end

@implementation NSTableCellView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableColumn : NSObject
@end

@implementation NSTableColumn

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableHeaderCell : NSObject
@end

@implementation NSTableHeaderCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableHeaderView : NSObject
@end

@implementation NSTableHeaderView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableRowView : NSObject
@end

@implementation NSTableRowView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableView : NSObject
@end

@implementation NSTableView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTableViewRowAction : NSObject
@end

@implementation NSTableViewRowAction

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSText : NSObject
@end

@implementation NSText

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextAttachment : NSObject
@end

@implementation NSTextAttachment

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextAttachmentCell : NSObject
@end

@implementation NSTextAttachmentCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextContainer : NSObject
@end

@implementation NSTextContainer

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextField : NSObject
@end

@implementation NSTextField

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextFieldCell : NSObject
@end

@implementation NSTextFieldCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextFinder : NSObject
@end

@implementation NSTextFinder

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextList : NSObject
@end

@implementation NSTextList

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextStorage : NSObject
@end

@implementation NSTextStorage

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextTab : NSObject
@end

@implementation NSTextTab

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTextView : NSObject
@end

@implementation NSTextView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTitlebarAccessoryViewController : NSObject
@end

@implementation NSTitlebarAccessoryViewController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTokenAttachment : NSObject
@end

@implementation NSTokenAttachment

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTokenAttachmentCell : NSObject
@end

@implementation NSTokenAttachmentCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTokenField : NSObject
@end

@implementation NSTokenField

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTokenFieldCell : NSObject
@end

@implementation NSTokenFieldCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSToolTipPanel : NSObject
@end

@implementation NSToolTipPanel

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSToolbar : NSObject
@end

@implementation NSToolbar

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSToolbarItem : NSObject
@end

@implementation NSToolbarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTouchBar : NSObject
@end

@implementation NSTouchBar

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTouchBarItem : NSObject
@end

@implementation NSTouchBarItem

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTouchBarItemContainerView : NSObject
@end

@implementation NSTouchBarItemContainerView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTouchBarView : NSObject
@end

@implementation NSTouchBarView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTouchDevice : NSObject
@end

@implementation NSTouchDevice

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTrackingArea : NSObject
@end

@implementation NSTrackingArea

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTreeController : NSObject
@end

@implementation NSTreeController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTreeNode : NSObject
@end

@implementation NSTreeNode

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSTypesetter : NSObject
@end

@implementation NSTypesetter

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSUndoReplaceCharacters : NSObject
@end

@implementation NSUndoReplaceCharacters

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSUndoTextOperation : NSObject
@end

@implementation NSUndoTextOperation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSUndoTyping : NSObject
@end

@implementation NSUndoTyping

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSUserDefaultsController : NSObject
@end

@implementation NSUserDefaultsController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSView : NSObject
@end

@implementation NSView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSViewAnimation : NSObject
@end

@implementation NSViewAnimation

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSViewController : NSObject
@end

@implementation NSViewController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSViewTextAttachmentCell : NSObject
@end

@implementation NSViewTextAttachmentCell

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSVisualEffectView : NSObject
@end

@implementation NSVisualEffectView

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSWindow : NSObject
@end

@implementation NSWindow

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSWindowController : NSObject
@end

@implementation NSWindowController

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSWindowTemplate : NSObject
@end

@implementation NSWindowTemplate

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSWorkspace : NSObject
@end

@implementation NSWorkspace

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface UINibEncoder : NSObject
@end

@implementation UINibEncoder

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
	DarlingStubForwardInvocation(self, anInvocation);
}

@end

@interface NSObject (BindingSupport)
@end

@implementation NSObject (BindingSupport)

- (id)bind: (id)bind toObject: (id)toObject withKeyPath: (id)withKeyPath options: (id)options
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}

- (id)infoForBinding: (id)infoForBinding
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}

- (id)unbind: (id)unbind
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}

@end

@interface NSObject (NSKeyValueBindingCreation)
@end

@implementation NSObject (NSKeyValueBindingCreation)

- (id)exposedBindings
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
	return nil;
}

@end

