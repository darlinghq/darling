// Probe 663: NSTextField maximumNumberOfLines. (1) The stored value for negative, zero, one, equal-to-the-line-count
// and larger numbers; (2) the size the cell and the field report (cellSizeForBounds:, intrinsicContentSize with
// preferredMaxLayoutWidth) for wrapping and non-wrapping fields; (3) where the capped text is drawn when the
// frame is taller than the text, in an unflipped field and in a field subclass that answers isFlipped YES,
// observed as the first and last inked pixel rows and the number of text bands in a rendered bitmap.
// Window server needed (text drawing and window); alpha 0, never key, no activation.
#define PROBE_APPKIT 1
#import "probe_common.h"

@interface FlippedField : NSTextField
@end
@implementation FlippedField
- (BOOL)isFlipped { return YES; }
@end

static NSString *kText = @"one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen";

static NSTextField *mk(Class cls, NSString *text, BOOL wraps, NSLineBreakMode mode, NSRect frame) {
    NSTextField *f = [[cls alloc] initWithFrame:frame];
    [f setEditable:NO]; [f setSelectable:NO]; [f setBezeled:NO]; [f setBordered:NO];
    [f setDrawsBackground:YES]; [f setBackgroundColor:[NSColor whiteColor]]; [f setTextColor:[NSColor blackColor]];
    [f setFont:[NSFont systemFontOfSize:13]];
    [f setStringValue:text];
    [[f cell] setWraps:wraps]; [[f cell] setLineBreakMode:mode];
    return f;
}

static NSString *sizes(NSTextField *f, double w) {
    NSSize cs = [[f cell] cellSizeForBounds:NSMakeRect(0, 0, w, 10000)];
    NSSize cs2 = [[f cell] cellSizeForBounds:NSMakeRect(0, 0, 10000, 10000)];
    [f setPreferredMaxLayoutWidth:w];
    NSSize ic = [f intrinsicContentSize];
    return PF(@"max=%ld cellSizeForBounds(w=%g)=%@ cellSizeForBounds(w=10000)=%@ intrinsic(preferredMaxLayoutWidth=%g)=%@",
              (long)[f maximumNumberOfLines], w, probe_size(cs), probe_size(cs2), w, probe_size(ic));
}

// Rendered bitmap rows with ink (dark pixels): first, last, number of bands.
static NSString *ink(NSTextField *f) {
    NSWindow *win = [[NSWindow alloc] initWithContentRect:[f frame] styleMask:NSWindowStyleMaskBorderless backing:NSBackingStoreBuffered defer:NO];
    [win setReleasedWhenClosed:NO]; [win setAlphaValue:0];
    NSView *host = [[NSView alloc] initWithFrame:[f frame]];
    [f setFrameOrigin:NSZeroPoint]; [host addSubview:f]; [win setContentView:host]; [win orderFront:nil];
    [f display];
    NSBitmapImageRep *rep = probe_snapshot(f);
    if (rep == nil) return @"no bitmap";
    NSInteger W = [rep pixelsWide], H = [rep pixelsHigh], first = -1, last = -1, bands = 0; BOOL in = NO;
    double scale = (double)H / NSHeight([f bounds]);
    for (NSInteger y = 0; y < H; y++) {
        BOOL any = NO;
        for (NSInteger x = 0; x < W && !any; x++) {
            NSColor *c = [[rep colorAtX:x y:y] colorUsingColorSpace:[NSColorSpace genericGrayColorSpace]];
            if (c != nil && [c whiteComponent] < 0.5) any = YES;
        }
        if (any) { if (first < 0) first = y; last = y; if (!in) bands++; }
        in = any;
    }
    [f removeFromSuperview];
    return PF(@"bitmap %ldx%ld (scale %g) inkFirstRow=%ld inkLastRow=%ld bands=%ld ; in view points from top: first=%g last=%g of height %g",
              (long)W, (long)H, scale, (long)first, (long)last, (long)bands, first < 0 ? -1.0 : first / scale, last < 0 ? -1.0 : (last + 1) / scale, (double)NSHeight([f bounds]));
}

int main(void) {
    @autoreleasepool {
        probe_env("663");
        probe_app();
        const double W = 120;
        // Line height and natural line count.
        NSTextField *one = mk([NSTextField class], @"Wg", YES, NSLineBreakByWordWrapping, NSMakeRect(0, 0, W, 20));
        double lh = [[one cell] cellSizeForBounds:NSMakeRect(0, 0, W, 10000)].height;
        NSTextField *nat = mk([NSTextField class], kText, YES, NSLineBreakByWordWrapping, NSMakeRect(0, 0, W, 20));
        double nh = [[nat cell] cellSizeForBounds:NSMakeRect(0, 0, W, 10000)].height;
        long n0 = lround(nh / lh);
        probe_line("663.0", @"one line height; natural height of the sample text at width 120; lines = ratio", PF(@"lineHeight=%g naturalHeight=%g lines=%ld", lh, nh, n0));
        probe_line("663.0b", @"default maximumNumberOfLines of a new NSTextField", PF(@"%ld", (long)[nat maximumNumberOfLines]));

        long maxes[] = {-1000, -5, -2, -1, 0, 1, 2, 3, n0 - 1, n0, n0 + 1, n0 + 5, 1000, 100000, (long)NSIntegerMax};
        struct { const char *n; NSString *t; BOOL wraps; NSLineBreakMode mode; } vars[] = {
            {"wrapping, word wrap, long text", nil, YES, NSLineBreakByWordWrapping},
            {"wrapping, char wrap, long text", nil, YES, NSLineBreakByCharWrapping},
            {"wrapping, truncate tail, long text", nil, YES, NSLineBreakByTruncatingTail},
            {"non-wrapping (wraps NO), truncate tail, long text", nil, NO, NSLineBreakByTruncatingTail},
            {"wrapping, word wrap, text with 5 explicit newlines (\"l1\\nl2\\nl3\\nl4\\nl5\\nl6\")", @"l1\nl2\nl3\nl4\nl5\nl6", YES, NSLineBreakByWordWrapping},
            {"non-wrapping, clipping, text with 6 explicit lines", @"l1\nl2\nl3\nl4\nl5\nl6", NO, NSLineBreakByClipping},
        };
        for (int v = 0; v < 6; v++)
            for (int m = 0; m < 15; m++) {
                NSTextField *f = mk([NSTextField class], vars[v].t ?: kText, vars[v].wraps, vars[v].mode, NSMakeRect(0, 0, W, 20));
                [f setMaximumNumberOfLines:maxes[m]];
                probe_line("663.1", PF(@"%s; setMaximumNumberOfLines:%ld", vars[v].n, maxes[m]), sizes(f, W));
            }

        // Frame sizes through the field-level APIs.
        {
            NSTextField *f = mk([NSTextField class], kText, YES, NSLineBreakByWordWrapping, NSMakeRect(0, 0, W, 20));
            long ms[] = {0, 1, 3, n0 + 5};
            for (int i = 0; i < 4; i++) {
                [f setMaximumNumberOfLines:ms[i]];
                NSSize st = [f sizeThatFits:NSMakeSize(W, 10000)];
                probe_line("663.2", PF(@"wrapping field; max=%ld; sizeThatFits:{120,10000}", ms[i]), probe_size(st));
            }
        }

        // Drawing anchor.
        long dm[] = {0, 2, 3, n0 - 1, n0 + 3};
        for (int flip = 0; flip < 2; flip++)
            for (int m = 0; m < 5; m++)
                for (int tall = 0; tall < 2; tall++) {
                    double H = tall ? 220 : lh * 2.5;
                    NSTextField *f = mk(flip ? [FlippedField class] : [NSTextField class], kText, YES, NSLineBreakByWordWrapping, NSMakeRect(0, 0, W, H));
                    [f setMaximumNumberOfLines:dm[m]];
                    probe_line("663.3", PF(@"%s field, wrapping, max=%ld, frame 120x%g (%s than the capped text); render and find ink rows", flip ? "FLIPPED (isFlipped YES)" : "unflipped", dm[m], H, tall ? "taller" : "shorter"), ink(f));
                }
        // Single short line in a tall frame, as anchor reference
        for (int flip = 0; flip < 2; flip++) {
            NSTextField *f = mk(flip ? [FlippedField class] : [NSTextField class], @"Wg", YES, NSLineBreakByWordWrapping, NSMakeRect(0, 0, W, 220));
            probe_line("663.4", PF(@"%s field, wrapping, one-line text 'Wg', max=0, frame 120x220", flip ? "FLIPPED" : "unflipped"), ink(f));
        }
    }
    return 0;
}
