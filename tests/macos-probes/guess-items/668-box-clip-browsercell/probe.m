// Probe 668: three controls the PR fills in.
//   Part A, NSBox -sizeToFit: for boxes with different titles, title positions, box types, content margins and
//          content views (two visible subviews, a hidden third, none, negative origins) prints the box frame, the
//          content view frame and its subviews' frames before and after -sizeToFit, -titleRect, -borderRect,
//          -contentViewMargins, and the title cell's size. Shows: do subviews move or the content view; are hidden
//          subviews in the union; how the width follows the title; whether the box origin stays; whether a content
//          view without subviews is left alone.
//   Part B, NSClipView -documentRect: with no document view, a smaller, an exactly fitting and a larger one,
//          flipped and not, negative origin, after scrolling, with contentInsets (when available), and the
//          related -documentVisibleRect and -constrainBoundsRect:.
//   Part C, NSBrowserCell -setImage: image and alternateImage getters, type, cellSize, imageRectForBounds:,
//          titleRectForBounds:, copy, and how many pixels of the image's colour a drawn cell shows for leaf and
//          non-leaf, normal and highlighted, with and without image and alternate image.
// Window server: yes (views in a window that is never ordered in; the application is never activated).
#define PROBE_APPKIT 1
#import "probe_common.h"

static NSString *frames(NSView *v) {
    NSMutableArray *a = [NSMutableArray array];
    for (NSView *s in [v subviews]) [a addObject:PF(@"%@%@", probe_rect([s frame]), [s isHidden] ? @"(hidden)" : @"")];
    return [a count] ? [a componentsJoinedByString:@" "] : @"(no subviews)";
}

static NSBox *makeBox(NSString *title, NSTitlePosition pos, NSBoxType type, NSSize margins, int kind) {
    NSBox *b = [[NSBox alloc] initWithFrame:NSMakeRect(50, 60, 300, 200)];
    [b setBoxType:type];
    [b setTitle:title];
    [b setTitlePosition:pos];
    [b setContentViewMargins:margins];
    NSView *cv = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 200, 100)];
    if (kind == 0 || kind == 1 || kind == 3) {
        [cv addSubview:[[NSView alloc] initWithFrame:NSMakeRect(10, 10, 50, 20)]];
        [cv addSubview:[[NSView alloc] initWithFrame:NSMakeRect(100, 40, 30, 30)]];
    }
    if (kind == 1) { NSView *h = [[NSView alloc] initWithFrame:NSMakeRect(500, 500, 10, 10)]; [h setHidden:YES]; [cv addSubview:h]; }
    if (kind == 3) [cv addSubview:[[NSView alloc] initWithFrame:NSMakeRect(-20, -10, 10, 10)]];
    [b setContentView:cv];
    return b;
}

static void boxCase(NSString *name, NSBox *b) {
    NSView *cv = [b contentView];
    NSString *beforeBox = probe_rect([b frame]), *beforeCv = probe_rect([cv frame]), *beforeSub = frames(cv);
    NSString *geo = PF(@"margins=%@ ; titleRect=%@ ; borderRect=%@ ; titleCell size=%@ ; title=\"%@\"", probe_size([b contentViewMargins]), probe_rect([b titleRect]), probe_rect([b borderRect]), probe_size([[b titleCell] cellSize]), [b title]);
    NSString *ex = probe_try(^{ [b sizeToFit]; });
    probe_line("668.box", name, PF(@"%@ ; BEFORE box=%@ content=%@ subviews=%@ ; geometry before=[%@] ; AFTER box=%@ content=%@ subviews=%@ ; geometry after=[margins=%@ titleRect=%@ borderRect=%@] ; content view same object=%d",
        ex, beforeBox, beforeCv, beforeSub, geo, probe_rect([b frame]), probe_rect([[b contentView] frame]), frames([b contentView]), probe_size([b contentViewMargins]), probe_rect([b titleRect]), probe_rect([b borderRect]), (int)([b contentView] == cv)));
}

static void partA(void) {
    boxCase(@"A1 primary, title at top, two subviews, default margins", makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(5, 5), 0));
    boxCase(@"A2 same with a long title (wider than the content)", makeBox(@"A considerably longer box title than the content is wide, to test the width", NSAtTop, NSBoxPrimary, NSMakeSize(5, 5), 0));
    boxCase(@"A3 same, no title", makeBox(@"", NSNoTitle, NSBoxPrimary, NSMakeSize(5, 5), 0));
    boxCase(@"A4 same, title below the box (NSBelowBottom)", makeBox(@"Box title", NSBelowBottom, NSBoxPrimary, NSMakeSize(5, 5), 0));
    boxCase(@"A5 same, title above the top (NSAboveTop)", makeBox(@"Box title", NSAboveTop, NSBoxPrimary, NSMakeSize(5, 5), 0));
    boxCase(@"A6 a hidden third subview far away", makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(5, 5), 1));
    boxCase(@"A7 content view without subviews", makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(5, 5), 2));
    boxCase(@"A8 a subview with a negative origin", makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(5, 5), 3));
    boxCase(@"A9 margins 20x20", makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(20, 20), 0));
    boxCase(@"A10 margins 0x0", makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(0, 0), 0));
    boxCase(@"A11 custom box type", makeBox(@"Box title", NSAtTop, NSBoxCustom, NSMakeSize(5, 5), 0));
    boxCase(@"A12 secondary box type", makeBox(@"Box title", NSAtTop, NSBoxSecondary, NSMakeSize(5, 5), 0));
    NSBox *b = makeBox(@"Box title", NSAtTop, NSBoxPrimary, NSMakeSize(5, 5), 0);
    [b sizeToFit];
    NSString *first = PF(@"%@ %@", probe_rect([b frame]), frames([b contentView]));
    [b sizeToFit];
    probe_line("668.box", @"A13 sizeToFit twice (idempotent?)", PF(@"first=%@ ; second=%@ %@", first, probe_rect([b frame]), frames([b contentView])));
    NSBox *nb = [[NSBox alloc] initWithFrame:NSMakeRect(5, 5, 300, 200)];
    NSString *ex = probe_try(^{ [nb sizeToFit]; });
    probe_line("668.box", @"A14 a box with the default content view and no subviews", PF(@"%@ ; box=%@ content=%@", ex, probe_rect([nb frame]), probe_rect([[nb contentView] frame])));
}

static void clipCase(NSString *name, NSView *doc, BOOL scroll, BOOL insets) {
    NSClipView *c = [[NSClipView alloc] initWithFrame:NSMakeRect(0, 0, 100, 80)];
    if (doc) [c setDocumentView:doc];
    if (insets && [c respondsToSelector:NSSelectorFromString(@"setContentInsets:")]) [c setValue:[NSValue valueWithEdgeInsets:NSEdgeInsetsMake(10, 20, 30, 40)] forKey:@"contentInsets"];
    if (scroll) [c scrollToPoint:NSMakePoint(30, 40)];
    probe_line("668.clip", name, PF(@"documentRect=%@ ; documentVisibleRect=%@ ; bounds=%@ ; constrainBoundsRect(-500,-500,100,80)=%@ ; constrainBoundsRect(900,900,100,80)=%@ ; flipped=%d",
        probe_rect([c documentRect]), probe_rect([c documentVisibleRect]), probe_rect([c bounds]), probe_rect([c constrainBoundsRect:NSMakeRect(-500, -500, 100, 80)]), probe_rect([c constrainBoundsRect:NSMakeRect(900, 900, 100, 80)]), (int)[c isFlipped]));
}

@interface FlipView : NSView
@end
@implementation FlipView
- (BOOL)isFlipped { return YES; }
@end

static void partB(void) {
    clipCase(@"B1 no document view", nil, NO, NO);
    clipCase(@"B2 document 50x40 at origin", [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 50, 40)], NO, NO);
    clipCase(@"B3 document exactly the clip size 100x80", [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 100, 80)], NO, NO);
    clipCase(@"B4 document 300x400", [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 400)], NO, NO);
    clipCase(@"B5 document 300x400, flipped", [[FlipView alloc] initWithFrame:NSMakeRect(0, 0, 300, 400)], NO, NO);
    clipCase(@"B6 document 300x400 at origin (-50,-60)", [[NSView alloc] initWithFrame:NSMakeRect(-50, -60, 300, 400)], NO, NO);
    clipCase(@"B7 document 300x400 at origin (70,90)", [[NSView alloc] initWithFrame:NSMakeRect(70, 90, 300, 400)], NO, NO);
    clipCase(@"B8 document 300x400 after scrollToPoint (30,40)", [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 400)], YES, NO);
    clipCase(@"B9 document 300x400, contentInsets 10/20/30/40", [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 400)], NO, YES);
    clipCase(@"B10 document 50x40, contentInsets 10/20/30/40", [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 50, 40)], NO, YES);
    NSClipView *c = [[NSClipView alloc] initWithFrame:NSMakeRect(0, 0, 100, 80)];
    NSView *d = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 400)];
    [c setDocumentView:d];
    [c setDocumentView:nil];
    probe_line("668.clip", @"B11 document view set then removed", PF(@"documentRect=%@", probe_rect([c documentRect])));
    NSView *d2 = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 400)];
    [c setDocumentView:d2];
    [d2 setFrame:NSMakeRect(0, 0, 500, 600)];
    probe_line("668.clip", @"B12 document frame changed after being set", PF(@"documentRect=%@", probe_rect([c documentRect])));
}

@interface CellHost : NSView
@property (strong) NSCell *cell;
@end
@implementation CellHost
- (void)drawRect:(NSRect)r { [[NSColor whiteColor] set]; NSRectFill(r); [self.cell drawWithFrame:[self bounds] inView:self]; }
@end

static NSImage *solid(NSColor *c, NSSize s) {
    NSImage *i = [[NSImage alloc] initWithSize:s];
    [i lockFocus]; [c set]; NSRectFill(NSMakeRect(0, 0, s.width, s.height)); [i unlockFocus];
    return i;
}
static NSString *count(NSBrowserCell *c, BOOL leaf, BOOL highlighted, NSWindow *w) {
    [c setLeaf:leaf];
    [c setHighlighted:highlighted];
    CellHost *h = [[CellHost alloc] initWithFrame:NSMakeRect(0, 0, 200, 20)];
    h.cell = c;
    [[w contentView] addSubview:h];
    NSBitmapImageRep *rep = probe_snapshot(h);
    [h removeFromSuperview];
    int red = 0, blue = 0;
    for (NSInteger y = 0; y < [rep pixelsHigh]; y++) for (NSInteger x = 0; x < [rep pixelsWide]; x++) {
        NSString *p = probe_pixel(rep, x, y);
        if ([p hasPrefix:@"ff0000"]) red++; else if ([p hasPrefix:@"0000ff"]) blue++;
    }
    return PF(@"pure red pixels=%d pure blue pixels=%d", red, blue);
}

static void partC(NSWindow *w) {
    NSBrowserCell *c = [[NSBrowserCell alloc] initTextCell:@"Item"];
    probe_line("668.cell", @"C1 fresh cell", PF(@"image=%@ alternateImage=%@ type=%lu leaf=%d cellSize=%@ imageRect=%@ titleRect=%@", [c image] ? @"non-nil" : @"nil", [c alternateImage] ? @"non-nil" : @"nil", (unsigned long)[c type], (int)[c isLeaf], probe_size([c cellSize]), probe_rect([c imageRectForBounds:NSMakeRect(0, 0, 200, 20)]), probe_rect([c titleRectForBounds:NSMakeRect(0, 0, 200, 20)])));
    probe_line("668.cell", @"C2 drawn without images (leaf / non-leaf, normal / highlighted)", PF(@"leaf: %@ ; nonleaf: %@ ; leaf highlighted: %@", count(c, YES, NO, w), count(c, NO, NO, w), count(c, YES, YES, w)));
    NSImage *red = solid([NSColor colorWithSRGBRed:1 green:0 blue:0 alpha:1], NSMakeSize(12, 12)), *blue = solid([NSColor colorWithSRGBRed:0 green:0 blue:1 alpha:1], NSMakeSize(12, 12));
    [c setImage:red];
    probe_line("668.cell", @"C3 after setImage:(red 12x12)", PF(@"image same object=%d alternateImage=%@ type=%lu cellSize=%@ imageRect=%@ titleRect=%@ ; objectValue=%@", (int)([c image] == red), [c alternateImage] ? @"non-nil" : @"nil", (unsigned long)[c type], probe_size([c cellSize]), probe_rect([c imageRectForBounds:NSMakeRect(0, 0, 200, 20)]), probe_rect([c titleRectForBounds:NSMakeRect(0, 0, 200, 20)]), [c objectValue]));
    probe_line("668.cell", @"C4 drawn with the image", PF(@"leaf: %@ ; nonleaf: %@ ; leaf highlighted: %@ ; nonleaf highlighted: %@", count(c, YES, NO, w), count(c, NO, NO, w), count(c, YES, YES, w), count(c, NO, YES, w)));
    [c setAlternateImage:blue];
    probe_line("668.cell", @"C5 after setAlternateImage:(blue) ; drawn", PF(@"alternateImage same=%d ; leaf normal: %@ ; leaf highlighted: %@", (int)([c alternateImage] == blue), count(c, YES, NO, w), count(c, YES, YES, w)));
    NSBrowserCell *cp = [c copy];
    probe_line("668.cell", @"C6 copy of the cell", PF(@"image same=%d alternateImage same=%d", (int)([cp image] == red), (int)([cp alternateImage] == blue)));
    [c setImage:solid([NSColor colorWithSRGBRed:1 green:0 blue:0 alpha:1], NSMakeSize(40, 40))];
    probe_line("668.cell", @"C7 setImage: 40x40", PF(@"cellSize=%@ imageRect=%@ titleRect=%@", probe_size([c cellSize]), probe_rect([c imageRectForBounds:NSMakeRect(0, 0, 200, 20)]), probe_rect([c titleRectForBounds:NSMakeRect(0, 0, 200, 20)])));
    [c setImage:nil];
    probe_line("668.cell", @"C8 after setImage:nil", PF(@"image=%@ type=%lu cellSize=%@ ; drawn: %@", [c image] ? @"non-nil" : @"nil", (unsigned long)[c type], probe_size([c cellSize]), count(c, YES, NO, w)));
    NSBrowserCell *ic = [[NSBrowserCell alloc] initImageCell:red];
    probe_line("668.cell", @"C9 initImageCell:(red)", PF(@"image same=%d type=%lu cellSize=%@", (int)([ic image] == red), (unsigned long)[ic type], probe_size([ic cellSize])));
    [ic setImage:blue];
    probe_line("668.cell", @"C10 then setImage:(blue)", PF(@"image same=%d type=%lu", (int)([ic image] == blue), (unsigned long)[ic type]));
    NSBrowserCell *tc = [[NSBrowserCell alloc] initTextCell:@"Item"];
    NSString *ex = probe_try(^{ [tc setImage:(NSImage *)@"not an image"]; });
    probe_line("668.cell", @"C11 setImage: with an object that is not an NSImage", ex);
}

int main(void) {
    @autoreleasepool {
        probe_env("668");
        probe_app();
        NSWindow *w = probe_window(NSMakeRect(-20000, -20000, 300, 100), NO);
        [w setAlphaValue:0];
        partA();
        partB();
        partC(w);
    }
    return 0;
}
