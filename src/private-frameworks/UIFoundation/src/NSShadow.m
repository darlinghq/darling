/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSShadow.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/NSRaise.h>
#import <UIFoundation/UIFoundationGraphicsContext.h>

// The shadow colour is an AppKit colour object, which gives its colour as a
// CGColor (owned by the caller).
@protocol _UIFoundationShadowColor
- (CGColorRef) CGColorRef;
@end

@implementation NSShadow

- init {
    _offset = NSMakeSize(0, 0); // what is the default?
    // Black with one third alpha, supplied by AppKit.
    _color = [_UIFoundationTextColorForRole(_UIFoundationTextColorRoleShadow) retain];
    _blurRadius = 0;
    return self;
}

- (void) dealloc {
    [_color release];
    [super dealloc];
}

- initWithCoder: (NSCoder *) coder {
    NSUnimplementedMethod();
    return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
    NSUnimplementedMethod();
}

- copyWithZone: (NSZone *) zone {
    NSShadow *copy = NSCopyObject(self, 0, zone);

    copy->_color = [_color copy];

    return copy;
}

- (NSSize) shadowOffset {
    return _offset;
}

- (NSColor *) shadowColor {
    return _color;
}

- (CGFloat) shadowBlurRadius {
    return _blurRadius;
}

- (void) setShadowOffset: (NSSize) offset {
    _offset = offset;
}

- (void) setShadowColor: (NSColor *) color {
    color = [color copy];
    [_color release];
    _color = color;
}

- (void) setShadowBlurRadius: (CGFloat) radius {
    _blurRadius = radius;
}

- (void) set {
    CGContextRef context = _UIFoundationCurrentCGContext();
    if (context == NULL)
        return;

    CGColorRef color = [(id<_UIFoundationShadowColor>) _color CGColorRef];

    CGContextSetShadowWithColor(context, _offset, _blurRadius, color);

    CGColorRelease(color);
}

@end
