/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import "SPGridView.h"

static const CGFloat kColumns = 8;
static const CGFloat kCellWidth = 80;
static const CGFloat kCellHeight = 78;
static const CGFloat kMargin = 14;
static const CGFloat kGroupGap = 10;
static const CGFloat kHeadingHeight = 16;

@interface SPGridView ()
- (void)iconClicked:(id)icon;
@end

@interface SPIconView : NSView
@property (readonly, strong) SPPane *pane;
@property (nonatomic, getter=isDimmed) BOOL dimmed;
@property (weak) SPGridView *grid;
- (instancetype)initWithPane:(SPPane *)pane frame:(NSRect)frame;
@end

@implementation SPIconView {
    BOOL _pressed;
}

- (instancetype)initWithPane:(SPPane *)pane frame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self != nil) {
        _pane = pane;
        [self setToolTip:pane.label];
    }
    return self;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)setDimmed:(BOOL)dimmed
{
    if (dimmed == _dimmed)
        return;
    _dimmed = dimmed;
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)dirty
{
    NSRect bounds = [self bounds];
    if (_pressed) {
        [[NSColor colorWithCalibratedWhite:0.0 alpha:0.12] set];
        [[NSBezierPath bezierPathWithRoundedRect:NSInsetRect(bounds, 2, 2) xRadius:5 yRadius:5] fill];
    }

    CGFloat alpha = _dimmed ? 0.25 : 1.0;
    NSImage *icon = _pane.icon;
    NSRect iconRect = NSMakeRect(NSMidX(bounds) - 16, 8, 32, 32);
    if (icon != nil) {
        [icon drawInRect:iconRect fromRect:NSZeroRect operation:NSCompositingOperationSourceOver fraction:alpha respectFlipped:YES hints:nil];
    } else {
        // a pane without an icon gets a plain tile with its first letter
        [[NSColor colorWithCalibratedWhite:0.62 alpha:alpha] set];
        [[NSBezierPath bezierPathWithRoundedRect:iconRect xRadius:7 yRadius:7] fill];
        NSString *letter = [_pane.label length] != 0 ? [[_pane.label substringToIndex:1] uppercaseString] : @"?";
        NSDictionary *letterAttributes = @{
            NSFontAttributeName: [NSFont boldSystemFontOfSize:18],
            NSForegroundColorAttributeName: [NSColor colorWithCalibratedWhite:1.0 alpha:alpha],
        };
        NSSize size = [letter sizeWithAttributes:letterAttributes];
        [letter drawAtPoint:NSMakePoint(NSMidX(iconRect) - size.width / 2, NSMidY(iconRect) - size.height / 2) withAttributes:letterAttributes];
    }

    NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSTextAlignmentCenter];
    [style setLineBreakMode:NSLineBreakByWordWrapping];
    NSDictionary *attributes = @{
        NSFontAttributeName: [NSFont systemFontOfSize:11],
        NSForegroundColorAttributeName: [[NSColor labelColor] colorWithAlphaComponent:alpha],
        NSParagraphStyleAttributeName: style,
    };
    [_pane.label drawInRect:NSMakeRect(2, 44, bounds.size.width - 4, bounds.size.height - 44) withAttributes:attributes];
}

- (void)mouseDown:(NSEvent *)event
{
    _pressed = YES;
    [self setNeedsDisplay:YES];
}

- (void)mouseUp:(NSEvent *)event
{
    BOOL inside = NSPointInRect([self convertPoint:[event locationInWindow] fromView:nil], [self bounds]);
    _pressed = NO;
    [self setNeedsDisplay:YES];
    if (inside)
        [_grid iconClicked:self];
}

@end

@implementation SPGridView {
    SPPaneCatalog *_catalog;
    CGFloat _height;
    NSMutableArray<NSNumber *> *_separators;   // y of each line between groups
    NSMutableArray *_headings;                 // @[ text, y ]
}

+ (CGFloat)gridWidth
{
    return kColumns * kCellWidth + 2 * kMargin;
}

- (instancetype)initWithCatalog:(SPPaneCatalog *)catalog
{
    self = [super initWithFrame:NSMakeRect(0, 0, [SPGridView gridWidth], 100)];
    if (self != nil) {
        _catalog = catalog;
        [self reloadData];
    }
    return self;
}

- (BOOL)isFlipped
{
    return YES;
}

- (CGFloat)preferredHeight
{
    return _height;
}

- (void)reloadData
{
    for (NSView *view in [[self subviews] copy])
        [view removeFromSuperview];
    _separators = [NSMutableArray array];
    _headings = [NSMutableArray array];

    CGFloat y = kGroupGap;
    BOOL first = YES;
    for (NSString *group in [_catalog groupIdentifiers]) {
        NSArray *panes = [_catalog panesInGroup:group];
        if (!first) {
            [_separators addObject:@(y - kGroupGap / 2)];
        }
        first = NO;
        if ([group isEqualToString:_catalog.catchAllGroupIdentifier]) {
            [_headings addObject:@[ NSLocalizedString(@"Other", @"heading of the third-party panes"), @(y) ]];
            y += kHeadingHeight;
        }
        NSUInteger index = 0;
        for (SPPane *pane in panes) {
            NSUInteger column = index % (NSUInteger)kColumns, row = index / (NSUInteger)kColumns;
            NSRect frame = NSMakeRect(kMargin + column * kCellWidth, y + row * kCellHeight, kCellWidth, kCellHeight);
            SPIconView *icon = [[SPIconView alloc] initWithPane:pane frame:frame];
            icon.grid = self;
            [self addSubview:icon];
            index++;
        }
        y += ceil((double)[panes count] / kColumns) * kCellHeight + kGroupGap;
    }
    _height = MAX(y, kCellHeight);
    [self setFrameSize:NSMakeSize([SPGridView gridWidth], _height)];
    [self setSearchText:_searchText];
    [self setNeedsDisplay:YES];
}

- (void)setSearchText:(NSString *)text
{
    _searchText = [text copy];
    for (SPIconView *icon in [self subviews])
        icon.dimmed = ![icon.pane matchesSearch:_searchText];
}

- (void)iconClicked:(SPIconView *)icon
{
    _clickedPane = icon.pane;
    if (_target != nil && _action != NULL)
        [NSApp sendAction:_action to:_target from:self];
}

- (void)drawRect:(NSRect)dirty
{
    [[NSColor colorWithCalibratedWhite:0.0 alpha:0.15] set];
    for (NSNumber *y in _separators)
        NSRectFill(NSMakeRect(kMargin, [y doubleValue], [self bounds].size.width - 2 * kMargin, 1));

    NSDictionary *attributes = @{
        NSFontAttributeName: [NSFont boldSystemFontOfSize:11],
        NSForegroundColorAttributeName: [NSColor secondaryLabelColor],
    };
    for (NSArray *heading in _headings)
        [heading[0] drawAtPoint:NSMakePoint(kMargin, [heading[1] doubleValue]) withAttributes:attributes];
}

@end
