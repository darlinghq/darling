/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
 Copyright (C) 2026 Darling Developers

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

#import <QTKit/QTMovieView.h>
#import <QTKit/QTMovie.h>
#import <AVFoundation/AVFoundation.h>
#import <QuartzCore/QuartzCore.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSLayoutConstraint.h>
#import <objc/runtime.h>
#import <darling-stub-forwarding.h>

@interface NSLayoutConstraint (QTKitWorkaround)
+ (void)deactivateConstraints:(NSArray *)constraints;
@end

@implementation NSLayoutConstraint (QTKitWorkaround)
+ (void)deactivateConstraints:(NSArray *)constraints
{
    for (id c in constraints) {
        if ([c respondsToSelector:@selector(setActive:)]) {
            [c setActive:NO];
        }
    }
}
@end

static char kQTMovieKey;
static char kQTPlayerLayerKey;
static char kQTPreservesAspectRatioKey;
static char kQTFillColorKey;
static char kQTControllerVisibleKey;
static char kQTEditableKey;

@implementation QTMovieView

- (QTMovie *)movie
{
    return objc_getAssociatedObject(self, &kQTMovieKey);
}

- (void)setMovie:(QTMovie *)movie
{
    AVPlayerLayer *oldLayer = objc_getAssociatedObject(self, &kQTPlayerLayerKey);
    if (oldLayer) {
        [oldLayer removeFromSuperlayer];
        oldLayer.player = nil;
        objc_setAssociatedObject(self, &kQTPlayerLayerKey, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }

    objc_setAssociatedObject(self, &kQTMovieKey, movie, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    if (movie != nil) {
        [self setWantsLayer:YES];
        AVPlayerLayer *layer = [AVPlayerLayer playerLayerWithPlayer:[movie _player]];
        layer.frame = NSRectToCGRect([self bounds]);
        layer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
        layer.videoGravity = [self preservesAspectRatio] ? AVLayerVideoGravityResizeAspect : AVLayerVideoGravityResize;
        [self.layer addSublayer:layer];
        objc_setAssociatedObject(self, &kQTPlayerLayerKey, layer, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    [self setNeedsDisplay:YES];
}

- (void)play:(id)sender
{
    [[self movie] play];
}

- (void)pause:(id)sender
{
    [[self movie] stop];
}

- (void)gotoBeginning:(id)sender
{
    [[self movie] gotoBeginning];
}

- (void)gotoEnd:(id)sender
{
    [[self movie] gotoEnd];
}

- (void)stepForward:(id)sender
{
    [[self movie] stepForward];
}

- (void)stepBackward:(id)sender
{
    [[self movie] stepBackward];
}

- (void)gotoPosterTime:(id)sender
{
    [[self movie] gotoBeginning];
}

- (BOOL)preservesAspectRatio
{
    NSNumber *val = objc_getAssociatedObject(self, &kQTPreservesAspectRatioKey);
    return val ? [val boolValue] : YES;
}

- (void)setPreservesAspectRatio:(BOOL)preserves
{
    objc_setAssociatedObject(self, &kQTPreservesAspectRatioKey, [NSNumber numberWithBool:preserves], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    AVPlayerLayer *layer = objc_getAssociatedObject(self, &kQTPlayerLayerKey);
    if (layer) {
        layer.videoGravity = preserves ? AVLayerVideoGravityResizeAspect : AVLayerVideoGravityResize;
    }
}

- (NSColor *)fillColor
{
    return objc_getAssociatedObject(self, &kQTFillColorKey);
}

- (void)setFillColor:(NSColor *)fillColor
{
    objc_setAssociatedObject(self, &kQTFillColorKey, fillColor, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [self setNeedsDisplay:YES];
}

- (BOOL)isControllerVisible
{
    NSNumber *val = objc_getAssociatedObject(self, &kQTControllerVisibleKey);
    return val ? [val boolValue] : NO;
}

- (void)setControllerVisible:(BOOL)visible
{
    objc_setAssociatedObject(self, &kQTControllerVisibleKey, [NSNumber numberWithBool:visible], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (BOOL)isEditable
{
    NSNumber *val = objc_getAssociatedObject(self, &kQTEditableKey);
    return val ? [val boolValue] : NO;
}

- (void)setEditable:(BOOL)editable
{
    objc_setAssociatedObject(self, &kQTEditableKey, [NSNumber numberWithBool:editable], OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (NSRect)movieBounds
{
    return [self bounds];
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSColor *fill = [self fillColor];
    if (fill) {
        [fill setFill];
        NSRectFill(dirtyRect);
    }
    if (![self wantsLayer]) {
        NSImage *img = [[self movie] currentFrameImage];
        if (img) {
            [img drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        }
    }
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldSize
{
    [super resizeSubviewsWithOldSize:oldSize];
    AVPlayerLayer *layer = objc_getAssociatedObject(self, &kQTPlayerLayerKey);
    if (layer) {
        layer.frame = NSRectToCGRect([self bounds]);
    }
}

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
