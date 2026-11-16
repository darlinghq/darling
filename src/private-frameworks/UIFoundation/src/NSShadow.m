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

#import <AppKit/NSColor.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSRaise.h>
#import <AppKit/NSShadow.h>
#import <ApplicationServices/ApplicationServices.h>

@interface NSColor (NSAppKitPrivate)
- (CGColorRef) CGColorRef;
@end

@implementation NSShadow

- init {
    _offset = NSMakeSize(0, 0); // what is the default?
    _color = [[NSColor colorWithCalibratedWhite: 0 alpha: 1.0 / 3.0] retain];
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
    CGContextRef context = [[NSGraphicsContext currentContext] graphicsPort];
    CGColorRef color = [_color CGColorRef];

    CGContextSetShadowWithColor(context, _offset, _blurRadius, color);

    CGColorRelease(color);
}

@end
