// A new NSTextContainer follows neither the width nor the height of its text
// view: Apple documents NO as the default of widthTracksTextView and
// heightTracksTextView, for -init and for -initWithContainerSize: alike. A
// container that follows the view's height stops a vertically resizable view
// from growing with its text once the view's frame changes. The flags that a
// caller sets are kept, and a container that is attached to a view while
// following it still follows it. No display needed. MRC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;

static void expect(BOOL ok, const char *what)
{
    if (!ok) {
        fprintf(stderr, "FAIL: %s\n", what);
        failures++;
    }
}

int main(void)
{
    @autoreleasepool {
        NSTextContainer *plain = [[[NSTextContainer alloc] init] autorelease];
        expect(![plain widthTracksTextView] && ![plain heightTracksTextView], "-init follows neither axis");

        NSTextContainer *sized = [[[NSTextContainer alloc] initWithContainerSize: NSMakeSize(100, 1e7)] autorelease];
        expect(![sized widthTracksTextView] && ![sized heightTracksTextView],
               "-initWithContainerSize: follows neither axis");
        expect(NSEqualSizes([sized containerSize], NSMakeSize(100, 1e7)), "-initWithContainerSize: keeps the size");

        [sized setWidthTracksTextView: YES];
        expect([sized widthTracksTextView] && ![sized heightTracksTextView], "setting the width flag leaves the height flag");
        [sized setHeightTracksTextView: YES];
        [sized setWidthTracksTextView: NO];
        expect(![sized widthTracksTextView] && [sized heightTracksTextView], "setting the height flag is kept");

        // A container that follows the width of its view and not its height. The
        // view resizes on both axes, so only the container's flags move the container.
        NSTextStorage *storage = [[[NSTextStorage alloc] init] autorelease];
        NSLayoutManager *layoutManager = [[[NSLayoutManager alloc] init] autorelease];
        [storage addLayoutManager: layoutManager];
        NSTextContainer *container = [[[NSTextContainer alloc] initWithContainerSize: NSMakeSize(100, 1e7)] autorelease];
        [layoutManager addTextContainer: container];
        [container setWidthTracksTextView: YES];
        NSTextView *view = [[[NSTextView alloc] initWithFrame: NSMakeRect(0, 0, 100, 100) textContainer: container] autorelease];
        [view setHorizontallyResizable: YES];
        [view setVerticallyResizable: YES];
        [view setMaxSize: NSMakeSize(1e7, 1e7)];
        [view setFrameSize: NSMakeSize(160, 50)];
        expect([[view textContainer] containerSize].width == 160 - 2 * [view textContainerInset].width,
               "the container follows the width of its view");
        expect([[view textContainer] containerSize].height == 1e7, "and not its height");
    }
    if (failures)
        return 1;
    puts("text container tracking defaults: PASS");
    return 0;
}
