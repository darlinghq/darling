// Probe 0012: sizes, inner rectangles and a few rendered pixels of the standard controls, at the three control
// sizes, in light and dark appearance. Public API only: cell sizes, intrinsic sizes, title/drawing rectangles,
// slider and progress defaults, segmented control widths, and bitmap snapshots. Nothing is shown on screen.
#define PROBE_APPKIT 1
#import "probe_common.h"

static const NSControlSize kSizes[3] = {NSControlSizeRegular, NSControlSizeSmall, NSControlSizeMini};
static const char *kSizeName[3] = {"regular", "small", "mini"};

static void configure(NSControl *c, int s) {
    [c setControlSize:kSizes[s]];
    [[c cell] setControlSize:kSizes[s]];
    [c setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:kSizes[s]]]];
}

static void cellMetrics(const char *id, NSString *what, NSControl *c, NSRect boundsForRects) {
    NSCell *cell = [c cell];
    probe_line(id, what,
               PF(@"font=%@ ; cellSize=%@ ; intrinsicContentSize=%@ ; fittingSize=%@ ; titleRectForBounds(%@)=%@ ; drawingRectForBounds=%@ ; alignmentRectInsets=%@",
                  probe_font([c font]), probe_size([cell cellSize]), probe_size([c intrinsicContentSize]), probe_fitting(c),
                  probe_rect(boundsForRects), [cell isKindOfClass:[NSButtonCell class]] ? probe_rect([(NSButtonCell *)cell titleRectForBounds:boundsForRects]) : @"n/a",
                  probe_rect([cell drawingRectForBounds:boundsForRects]),
                  PF(@"{t%g l%g b%g r%g}", (double)[c alignmentRectInsets].top, (double)[c alignmentRectInsets].left,
                     (double)[c alignmentRectInsets].bottom, (double)[c alignmentRectInsets].right)));
    NSSize before = [c frame].size;
    [c sizeToFit];
    probe_line(id, PF(@"%@ ; -sizeToFit from frame %@", what, probe_size(before)), PF(@"frame size %@", probe_size([c frame].size)));
}

static void pixels(const char *id, NSString *what, NSView *v, int dark, NSArray<NSValue *> *points) {
    NSWindow *w = probe_window(NSMakeRect(0, 0, [v frame].size.width, [v frame].size.height), dark);
    [v setFrameOrigin:NSZeroPoint];
    [[w contentView] addSubview:v];
    NSBitmapImageRep *rep = probe_snapshot(v);
    NSRect b = [v bounds];
    double sx = rep ? [rep pixelsWide] / b.size.width : 1, sy = rep ? [rep pixelsHigh] / b.size.height : 1;
    NSMutableArray *out = [NSMutableArray array];
    for (NSValue *pv in points) {
        NSPoint p = [pv pointValue];   // points, y measured from the top
        [out addObject:PF(@"(%g,%g)=%@", (double)p.x, (double)p.y, probe_pixel(rep, (NSInteger)(p.x * sx), (NSInteger)(p.y * sy)))];
    }
    probe_line(id, PF(@"%@ ; %s appearance ; bitmap %ldx%ld for %@ ; non-premultiplied sRGB rrggbbaa ; window not key", what, dark ? "dark" : "light",
                      (long)[rep pixelsWide], (long)[rep pixelsHigh], probe_size(b.size)), [out componentsJoinedByString:@" "]);
    [v removeFromSuperview];
}

static NSArray<NSValue *> *grid(NSSize s) {
    NSMutableArray *a = [NSMutableArray array];
    double xs[] = {0.5, 2, 6, 12, 24, s.width / 2, s.width - 6, s.width - 2};
    double ys[] = {0.5, 2, 6, s.height / 2, s.height - 6, s.height - 2};
    for (size_t j = 0; j < sizeof ys / sizeof ys[0]; j++)
        for (size_t i = 0; i < sizeof xs / sizeof xs[0]; i++)
            if (xs[i] < s.width && ys[j] < s.height) [a addObject:[NSValue valueWithPoint:NSMakePoint(xs[i], ys[j])]];
    return a;
}

int main(void) {
    @autoreleasepool {
        probe_env("0012");
        probe_app();
        for (int s = 0; s < 3; s++) {
            NSButton *push = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [push setBezelStyle:NSBezelStyleRounded];
            [push setTitle:@"Title"];
            configure(push, s);
            cellMetrics("0012.push", PF(@"push button, rounded bezel, title Title, %s", kSizeName[s]), push, NSMakeRect(0, 0, 120, 40));

            NSButton *sq = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [sq setBezelStyle:NSBezelStyleRegularSquare];
            [sq setTitle:@"Title"];
            configure(sq, s);
            cellMetrics("0012.square", PF(@"push button, regular-square bezel, title Title, %s", kSizeName[s]), sq, NSMakeRect(0, 0, 120, 40));

            NSButton *cb = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [cb setButtonType:NSButtonTypeSwitch];
            [cb setTitle:@"Title"];
            configure(cb, s);
            cellMetrics("0012.checkbox", PF(@"checkbox, title Title, %s", kSizeName[s]), cb, NSMakeRect(0, 0, 120, 40));

            NSButton *rb = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [rb setButtonType:NSButtonTypeRadio];
            [rb setTitle:@"Title"];
            configure(rb, s);
            cellMetrics("0012.radio", PF(@"radio button, title Title, %s", kSizeName[s]), rb, NSMakeRect(0, 0, 120, 40));

            NSPopUpButton *pop = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40) pullsDown:NO];
            [pop addItemsWithTitles:@[@"Title", @"Other"]];
            configure(pop, s);
            cellMetrics("0012.popup", PF(@"pop-up button, items Title and Other, %s", kSizeName[s]), pop, NSMakeRect(0, 0, 120, 40));

            NSPopUpButton *pull = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40) pullsDown:YES];
            [pull addItemsWithTitles:@[@"Title", @"Other"]];
            configure(pull, s);
            cellMetrics("0012.pulldown", PF(@"pull-down button, items Title and Other, %s", kSizeName[s]), pull, NSMakeRect(0, 0, 120, 40));

            for (int vertical = 0; vertical <= 1; vertical++) {
                NSSlider *sl = [[NSSlider alloc] initWithFrame:vertical ? NSMakeRect(0, 0, 40, 120) : NSMakeRect(0, 0, 120, 40)];
                [sl setControlSize:kSizes[s]];
                [[sl cell] setControlSize:kSizes[s]];
                NSSliderCell *sc = [sl cell];
                probe_line("0012.slider", PF(@"slider %s, %s, fresh instance", vertical ? "vertical" : "horizontal", kSizeName[s]),
                           PF(@"min=%g max=%g value=%g ; knobThickness=%g ; cellSize=%@ ; intrinsicContentSize=%@ ; isVertical=%ld",
                              [sl minValue], [sl maxValue], [sl doubleValue], (double)[sc knobThickness], probe_size([sc cellSize]),
                              probe_size([sl intrinsicContentSize]), (long)[sl isVertical]));
                NSSize b0 = [sl frame].size;
                [sl sizeToFit];
                probe_line("0012.slider", PF(@"slider %s, %s, -sizeToFit from %@", vertical ? "vertical" : "horizontal", kSizeName[s], probe_size(b0)), PF(@"frame size %@", probe_size([sl frame].size)));
                [sl setMinValue:0]; [sl setMaxValue:100]; [sl setDoubleValue:25];
                NSWindow *w = probe_window(NSMakeRect(0, 0, 200, 200), NO);
                [[w contentView] addSubview:sl];
                [sl display];
                probe_line("0012.slider", PF(@"slider %s, %s, 0..100 value 25, after display", vertical ? "vertical" : "horizontal", kSizeName[s]),
                           PF(@"knobRect flipped=NO %@ ; knobRect flipped=YES %@ ; isVertical=%ld", probe_rect([sc knobRectFlipped:NO]), probe_rect([sc knobRectFlipped:YES]), (long)[sl isVertical]));
            }

            NSProgressIndicator *pi = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [pi setStyle:NSProgressIndicatorStyleBar];
            [pi setIndeterminate:NO];
            [pi setControlSize:kSizes[s]];
            NSSize p0 = [pi frame].size;
            [pi sizeToFit];
            probe_line("0012.progress", PF(@"progress bar, %s, -sizeToFit from %@", kSizeName[s], probe_size(p0)),
                       PF(@"frame size %@ ; intrinsicContentSize=%@ ; controlSize=%ld", probe_size([pi frame].size), probe_size([pi intrinsicContentSize]), (long)[pi controlSize]));

            for (int style = 0; style < 2; style++) {
                NSSegmentedControl *sg = [[NSSegmentedControl alloc] initWithFrame:NSMakeRect(0, 0, 300, 40)];
                [sg setSegmentCount:3];
                [sg setLabel:@"One" forSegment:0];
                [sg setLabel:@"Two" forSegment:1];
                [sg setLabel:@"Three" forSegment:2];
                [sg setSegmentStyle:style == 0 ? NSSegmentStyleRounded : NSSegmentStyleTexturedRounded];
                [sg setControlSize:kSizes[s]];
                [[sg cell] setControlSize:kSizes[s]];
                [sg setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:kSizes[s]]]];
                NSSize s0 = [sg frame].size;
                probe_line("0012.segmented", PF(@"segmented control, labels One Two Three, %s, style %s", kSizeName[s], style == 0 ? "rounded" : "texturedRounded"),
                           PF(@"cellSize=%@ ; intrinsicContentSize=%@ ; widthForSegment 0,1,2 = %g,%g,%g", probe_size([[sg cell] cellSize]), probe_size([sg intrinsicContentSize]),
                              [sg widthForSegment:0], [sg widthForSegment:1], [sg widthForSegment:2]));
                [sg sizeToFit];
                probe_line("0012.segmented", PF(@"segmented control, %s, style %s, -sizeToFit from %@", kSizeName[s], style == 0 ? "rounded" : "texturedRounded", probe_size(s0)),
                           PF(@"frame size %@", probe_size([sg frame].size)));
            }

            NSTextField *tf = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 100, 40)];
            [tf setStringValue:@"Text"];
            configure(tf, s);
            cellMetrics("0012.textfield", PF(@"bezeled editable text field, value Text, %s", kSizeName[s]), tf, NSMakeRect(0, 0, 100, 40));
            NSTextField *lb = [NSTextField labelWithString:@"Text"];
            [lb setFont:[NSFont systemFontOfSize:[NSFont systemFontSizeForControlSize:kSizes[s]]]];
            cellMetrics("0012.label", PF(@"label (labelWithString:), value Text, %s", kSizeName[s]), lb, NSMakeRect(0, 0, 100, 40));
        }

        // Text that is drawn into a rectangle lower than one line: is anything drawn?
        {
            NSFont *font = [NSFont systemFontOfSize:13];
            NSDictionary *attrs = @{NSFontAttributeName: font, NSForegroundColorAttributeName: [NSColor blackColor]};
            double heights[] = {4, 8, 12, 16, 24};
            for (size_t i = 0; i < sizeof heights / sizeof heights[0]; i++) {
                NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:100 pixelsHigh:40 bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:0 bitsPerPixel:0];
                NSGraphicsContext *g = [NSGraphicsContext graphicsContextWithBitmapImageRep:rep];
                [NSGraphicsContext saveGraphicsState];
                [NSGraphicsContext setCurrentContext:g];
                [@"Text" drawInRect:NSMakeRect(0, 10, 100, heights[i]) withAttributes:attrs];
                [NSGraphicsContext restoreGraphicsState];
                NSInteger touched = 0, rowMin = 40, rowMax = -1;
                for (NSInteger y = 0; y < 40; y++)
                    for (NSInteger x = 0; x < 100; x++)
                        if ([[rep colorAtX:x y:y] alphaComponent] > 0.01) { touched++; if (y < rowMin) rowMin = y; if (y > rowMax) rowMax = y; }
                probe_line("0012.drawInRect", PF(@"\"Text\" 13 pt drawInRect:{0,10,100,%g} withAttributes (rect lower than one line when < %g)", heights[i], (double)ceil([font ascender] - [font descender] + [font leading])),
                           PF(@"touched pixels=%ld ; rows touched %ld..%ld (bitmap rows from the top, rect spans rows %g..%g from the bottom edge: 10..%g)", (long)touched, (long)(touched ? rowMin : -1), (long)(touched ? rowMax : -1), 10.0, 10.0 + heights[i], 10.0 + heights[i]));
            }
        }

        // Rendered pixels.
        for (int dark = 0; dark <= 1; dark++) {
            NSButton *push = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [push setBezelStyle:NSBezelStyleRounded];
            [push setTitle:@""];
            pixels("0012.pixels", @"push button 120x40, empty title", push, dark, grid(NSMakeSize(120, 40)));
            NSButton *def = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 40)];
            [def setBezelStyle:NSBezelStyleRounded];
            [def setTitle:@""];
            [def setKeyEquivalent:@"\r"];
            pixels("0012.pixels", @"default push button (Return) 120x40, empty title", def, dark, grid(NSMakeSize(120, 40)));
            NSButton *cb = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 60, 24)];
            [cb setButtonType:NSButtonTypeSwitch];
            [cb setTitle:@""];
            [cb setState:NSControlStateValueOn];
            pixels("0012.pixels", @"checkbox on 60x24, empty title", cb, dark, grid(NSMakeSize(60, 24)));
            NSSlider *sl = [[NSSlider alloc] initWithFrame:NSMakeRect(0, 0, 120, 24)];
            [sl setMinValue:0]; [sl setMaxValue:100]; [sl setDoubleValue:50];
            pixels("0012.pixels", @"slider 120x24 at 50", sl, dark, grid(NSMakeSize(120, 24)));
            NSProgressIndicator *pi = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect(0, 0, 120, 20)];
            [pi setStyle:NSProgressIndicatorStyleBar];
            [pi setIndeterminate:NO];
            [pi setMinValue:0]; [pi setMaxValue:100]; [pi setDoubleValue:50];
            pixels("0012.pixels", @"progress bar 120x20 at 50", pi, dark, grid(NSMakeSize(120, 20)));
            NSTextField *tf = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 120, 24)];
            [tf setStringValue:@""];
            pixels("0012.pixels", @"empty bezeled text field 120x24", tf, dark, grid(NSMakeSize(120, 24)));
        }
    }
    return 0;
}
