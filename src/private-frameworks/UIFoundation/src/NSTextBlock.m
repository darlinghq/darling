/* Copyright (c) 2007 Christopher J. W. Lloyd

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

#import "UIFoundation-ExtraIvars.h"
#import <Foundation/NSRaise.h>
#import <AppKit/NSTextBlock.h>
#import <Foundation/NSArray.h>

@implementation NSTextBlock

- init {
    NSUnimplementedMethod();
    return self;
}

- (NSColor *) backgroundColor {
    return UIF_EXTRA_NSTextBlock(self)->_backgroundColor;
}

- (NSColor *) borderColorForEdge: (NSRectEdge) edge {
    return [UIF_EXTRA_NSTextBlock(self)->_borderColors objectAtIndex: edge];
}

- (NSTextBlockVerticalAlignment) verticalAlignment {
    return UIF_EXTRA_NSTextBlock(self)->_verticalAlignment;
}

- (CGFloat) contentWidth {
    return UIF_EXTRA_NSTextBlock(self)->_contentWidth;
}

- (NSTextBlockValueType) contentWidthValueType {
    return UIF_EXTRA_NSTextBlock(self)->_contentWidthValueType;
}

- (CGFloat) valueForDimension: (NSTextBlockDimension) dimension {
    return UIF_EXTRA_NSTextBlock(self)->_dimensionValues[dimension];
}

- (NSTextBlockValueType) valueTypeForDimension: (NSTextBlockDimension) dimension
{
    NSUnimplementedMethod();
    return 0;
}

- (CGFloat) widthForLayer: (NSTextBlockLayer) layer edge: (NSRectEdge) edge {
    return UIF_EXTRA_NSTextBlock(self)->_layerWidths[layer][edge];
}

- (NSTextBlockValueType) widthValueTypeForLayer: (NSTextBlockLayer) layer
                                           edge: (NSRectEdge) edge
{
    return UIF_EXTRA_NSTextBlock(self)->_layerValueTypes[layer][edge];
}

- (void) setBackgroundColor: (NSColor *) color {
    color = [color copy];
    [UIF_EXTRA_NSTextBlock(self)->_backgroundColor release];
    UIF_EXTRA_NSTextBlock(self)->_backgroundColor = color;
}

- (void) setBorderColor: (NSColor *) color {
    int i, count = [UIF_EXTRA_NSTextBlock(self)->_borderColors count];

    for (i = 0; i < count; i++)
        [UIF_EXTRA_NSTextBlock(self)->_borderColors replaceObjectAtIndex: i withObject: color];
}

- (void) setBorderColor: (NSColor *) color forEdge: (NSRectEdge) edge {
    [UIF_EXTRA_NSTextBlock(self)->_borderColors replaceObjectAtIndex: edge withObject: color];
}

- (void) setVerticalAlignment: (NSTextBlockVerticalAlignment) alignment {
    UIF_EXTRA_NSTextBlock(self)->_verticalAlignment = alignment;
}

- (void) setContentWidth: (CGFloat) width type: (NSTextBlockValueType) type {
    UIF_EXTRA_NSTextBlock(self)->_contentWidth = width;
    UIF_EXTRA_NSTextBlock(self)->_contentWidthValueType = type;
}

- (void) setValue: (CGFloat) value
                type: (NSTextBlockValueType) type
        forDimension: (NSTextBlockDimension) dimension
{
    UIF_EXTRA_NSTextBlock(self)->_dimensionValues[dimension] = value;
    UIF_EXTRA_NSTextBlock(self)->_dimensionValueTypes[dimension] = type;
}

- (void) setWidth: (CGFloat) value
             type: (NSTextBlockValueType) type
         forLayer: (NSTextBlockLayer) layer
{
    int i;

    for (i = 0; i <= NSMaxYEdge; i++) {
        UIF_EXTRA_NSTextBlock(self)->_layerWidths[layer][i] = value;
        UIF_EXTRA_NSTextBlock(self)->_layerValueTypes[layer][i] = type;
    }
}

- (void) setWidth: (CGFloat) value
             type: (NSTextBlockValueType) type
         forLayer: (NSTextBlockLayer) layer
             edge: (NSRectEdge) edge
{
    UIF_EXTRA_NSTextBlock(self)->_layerWidths[layer][edge] = value;
    UIF_EXTRA_NSTextBlock(self)->_layerValueTypes[layer][edge] = type;
}

- (NSRect) rectForLayoutAtPoint: (NSPoint) point
                         inRect: (NSRect) rect
                  textContainer: (NSTextContainer *) textContainer
                 characterRange: (NSRange) range
{
    NSUnimplementedMethod();
    return NSZeroRect;
}

- (NSRect) boundsRectForContentRect: (NSRect) contentRect
                             inRect: (NSRect) rect
                      textContainer: (NSTextContainer *) textContainer
                     characterRange: (NSRange) range
{
    NSUnimplementedMethod();
    return NSZeroRect;
}

- (void) drawBackgroundWithFrame: (NSRect) frame
                          inView: (NSView *) view
                  characterRange: (NSRange) range
                   layoutManager: (NSLayoutManager *) layoutManager
{
    NSUnimplementedMethod();
}

UIF_EXTRA_DEALLOC(NSTextBlock)

@end
