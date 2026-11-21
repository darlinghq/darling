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
#import "QTMoviePrivate.h"
#import <AVFoundation/AVFoundation.h>
#import <QuartzCore/QuartzCore.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSGraphics.h>
#import <objc/runtime.h>
#import <darling-stub-forwarding.h>

// The height of the controller strip QTMovieView reserves at the bottom of its bounds when the
// controller is visible. Only the space is kept: there is no controller drawn in it.
static const CGFloat QTMovieViewControllerHeight = 16.0;

// The state of a view. The view has no instance variables of its own: its instance layout
// is fixed, because a subclass compiled against it puts its own variables right after them.
@interface QTMovieViewState : NSObject {
@public
    QTMovie *movie;
    AVPlayerLayer *playerLayer;
    BOOL controllerVisible;
    BOOL preservesAspectRatio;
    BOOL editable;
    NSColor *fillColor;
}
@end

@implementation QTMovieViewState

- (instancetype)init
{
    if ((self = [super init])) {
        controllerVisible = YES;
        fillColor = [[NSColor blackColor] retain];
    }
    return self;
}

- (void)dealloc
{
    [playerLayer removeFromSuperlayer];
    playerLayer.player = nil;
    [playerLayer release];
    [movie release];
    [fillColor release];
    [super dealloc];
}

@end

static char QTMovieViewStateKey;

@implementation QTMovieView

- (QTMovieViewState *)_state
{
    QTMovieViewState *state = objc_getAssociatedObject(self, &QTMovieViewStateKey);
    if (!state) {
        state = [[[QTMovieViewState alloc] init] autorelease];
        objc_setAssociatedObject(self, &QTMovieViewStateKey, state, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    return state;
}

- (void)_layoutPlayerLayer
{
    CGRect frame = NSRectToCGRect([self movieBounds]);
    AVPlayerLayer *layer = [self _state]->playerLayer;
    if (layer && !CGRectEqualToRect(layer.frame, frame)) {
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        layer.frame = frame;
        [CATransaction commit];
    }
}

- (QTMovie *)movie
{
    return [self _state]->movie;
}

- (void)setMovie:(QTMovie *)movie
{
    QTMovieViewState *state = [self _state];
    if (state->movie == movie)
        return;

    [state->playerLayer removeFromSuperlayer];
    state->playerLayer.player = nil;
    [state->playerLayer release];
    state->playerLayer = nil;
    [state->movie release];
    state->movie = [movie retain];

    if (movie) {
        [self setWantsLayer:YES];
        AVPlayerLayer *layer = [AVPlayerLayer playerLayerWithPlayer:[movie _player]];
        layer.videoGravity = state->preservesAspectRatio ? AVLayerVideoGravityResizeAspect : AVLayerVideoGravityResize;
        state->playerLayer = [layer retain];
        [self.layer addSublayer:layer];
        [self _layoutPlayerLayer];
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
    return [self _state]->preservesAspectRatio;
}

- (void)setPreservesAspectRatio:(BOOL)preserves
{
    QTMovieViewState *state = [self _state];
    state->preservesAspectRatio = preserves;
    state->playerLayer.videoGravity = preserves ? AVLayerVideoGravityResizeAspect : AVLayerVideoGravityResize;
}

- (NSColor *)fillColor
{
    return [self _state]->fillColor;
}

- (void)setFillColor:(NSColor *)fillColor
{
    QTMovieViewState *state = [self _state];
    if (state->fillColor != fillColor) {
        [state->fillColor release];
        state->fillColor = [fillColor retain];
    }
    [self setNeedsDisplay:YES];
}

- (BOOL)isControllerVisible
{
    return [self _state]->controllerVisible;
}

- (void)setControllerVisible:(BOOL)visible
{
    [self _state]->controllerVisible = visible;
    [self _layoutPlayerLayer];
    [self setNeedsDisplay:YES];
}

- (BOOL)isEditable
{
    return [self _state]->editable;
}

- (void)setEditable:(BOOL)editable
{
    [self _state]->editable = editable;
}

- (NSRect)movieBounds
{
    NSRect bounds = [self bounds];
    if ([self isControllerVisible]) {
        CGFloat strip = MIN(QTMovieViewControllerHeight, NSHeight(bounds));
        bounds.origin.y += strip;
        bounds.size.height -= strip;
    }
    return bounds;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSColor *fill = [self fillColor];
    if (fill) {
        [fill setFill];
        NSRectFill(dirtyRect);
    }
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldSize
{
    [super resizeSubviewsWithOldSize:oldSize];
    [self _layoutPlayerLayer];
}

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    [self _layoutPlayerLayer];
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
