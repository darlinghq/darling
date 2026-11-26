// -[NSTextContainer size], -setSize: and -initWithSize:, the current (macOS 10.11) names for the extent of a text
// container, which AppKit's documentation lists the older containerSize / initWithContainerSize: as giving way to
// (developer.apple.com/documentation/appkit/nstextcontainer/size, .../containersize, .../init(size:)).
// The pages state: the size caps what the container hands back for a proposed line rectangle (non-positive values
// meaning no cap is documented and is not tested here), and -initWithSize: is the designated initializer. They say
// nothing about layout invalidation or about how the two spellings relate; checks resting on that are marked OBSERVED:
// they hold on macOS 26.7.3 (black-box run of this file, tier 2).
// No display needed. MRC. The calls go through a protocol so that the test compiles against headers without the
// declarations and reports FAIL (instead of failing to build or raising) when the selectors are missing.
#import <AppKit/AppKit.h>
#include <stdio.h>

@protocol SizeNames
- (instancetype)initWithSize:(NSSize)size;
- (NSSize)size;
- (void)setSize:(NSSize)size;
@end

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

// Counts the container's geometry-change messages to its layout manager. OBSERVED: the container sends
// -textContainerChangedGeometry: when its size changes (the page does not say).
static int geometryChanges;
@interface CountingLayoutManager : NSLayoutManager
@end
@implementation CountingLayoutManager
- (void)textContainerChangedGeometry:(NSTextContainer *)container
{
    geometryChanges++;
    [super textContainerChangedGeometry:container];
}
@end

static NSTextContainer *makeContainer(NSSize size)
{
    return [(NSTextContainer *)[(id<SizeNames>)[NSTextContainer alloc] initWithSize:size] autorelease];
}

static CGFloat usedHeight(NSLayoutManager *lm, NSTextContainer *tc)
{
    [lm ensureLayoutForTextContainer:tc];
    return [lm usedRectForTextContainer:tc].size.height;
}

int main(void)
{
    @autoreleasepool {
        CHECK([NSTextContainer instancesRespondToSelector:@selector(size)]);
        CHECK([NSTextContainer instancesRespondToSelector:@selector(setSize:)]);
        CHECK([NSTextContainer instancesRespondToSelector:@selector(initWithSize:)]);
        if (![NSTextContainer instancesRespondToSelector:@selector(size)] ||
            ![NSTextContainer instancesRespondToSelector:@selector(setSize:)] ||
            ![NSTextContainer instancesRespondToSelector:@selector(initWithSize:)]) {
            printf("FAIL size names missing\n");
            return 1;
        }

        // Documented: -initWithSize: sets the bounding size.
        NSTextContainer *tc = makeContainer(NSMakeSize(300, 1e7));
        CHECK(NSEqualSizes([(id<SizeNames>)tc size], NSMakeSize(300, 1e7)));
        // OBSERVED: the older name reads the same value (same property under two names).
        CHECK(NSEqualSizes([tc containerSize], NSMakeSize(300, 1e7)));

        // Documented: size is read-write.
        [(id<SizeNames>)tc setSize:NSMakeSize(120, 80)];
        CHECK(NSEqualSizes([(id<SizeNames>)tc size], NSMakeSize(120, 80)));
        CHECK(NSEqualSizes([tc containerSize], NSMakeSize(120, 80)));
        [tc setContainerSize:NSMakeSize(60, 40)];
        CHECK(NSEqualSizes([(id<SizeNames>)tc size], NSMakeSize(60, 40)));

        // A container made with the older initializer reads back through size.
        NSTextContainer *old = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(7, 9)] autorelease];
        CHECK(NSEqualSizes([(id<SizeNames>)old size], NSMakeSize(7, 9)));

        // Documented: size caps the rectangle that the container hands back for a proposed line rectangle.
        [(id<SizeNames>)tc setSize:NSMakeSize(100, 50)];
        NSRect remaining = NSZeroRect;
        NSRect fragment = [tc lineFragmentRectForProposedRect:NSMakeRect(0, 0, 500, 20)
                                               sweepDirection:NSLineSweepRight
                                            movementDirection:NSLineMovesDown
                                                remainingRect:&remaining];
        CHECK(fragment.size.width == 100);
        CHECK(fragment.size.height == 20);
        [(id<SizeNames>)tc setSize:NSMakeSize(250, 50)];
        fragment = [tc lineFragmentRectForProposedRect:NSMakeRect(0, 0, 500, 20)
                                        sweepDirection:NSLineSweepRight
                                     movementDirection:NSLineMovesDown
                                         remainingRect:&remaining];
        CHECK(fragment.size.width == 250);

        // OBSERVED: a change of size makes the layout manager lay the text out again at the new width (the page is
        // silent on invalidation; a layout area that is the container's size can only be honoured that way).
        NSTextStorage *ts = [[[NSTextStorage alloc] initWithString:
            @"a fairly long line of text that needs to wrap when the container becomes narrow"] autorelease];
        NSLayoutManager *lm = [[[NSLayoutManager alloc] init] autorelease];
        NSTextContainer *wrap = makeContainer(NSMakeSize(2000, 1e7));
        [lm addTextContainer:wrap];
        [ts addLayoutManager:lm];
        CGFloat wide = usedHeight(lm, wrap);
        [(id<SizeNames>)wrap setSize:NSMakeSize(60, 1e7)];
        CGFloat narrow = usedHeight(lm, wrap);
        CHECK(narrow > wide);
        [(id<SizeNames>)wrap setSize:NSMakeSize(2000, 1e7)];
        CHECK(usedHeight(lm, wrap) == wide);

        // OBSERVED: whichever name sets the size, the layout manager is told once per change and both names read the new value.
        CountingLayoutManager *clm = [[[CountingLayoutManager alloc] init] autorelease];
        NSTextContainer *counted = makeContainer(NSMakeSize(100, 100));
        [clm addTextContainer:counted];
        geometryChanges = 0;
        [(id<SizeNames>)counted setSize:NSMakeSize(150, 100)];
        CHECK(geometryChanges == 1);
        CHECK(NSEqualSizes([counted containerSize], NSMakeSize(150, 100)));
        [counted setContainerSize:NSMakeSize(180, 100)];
        CHECK(geometryChanges == 2);
        CHECK(NSEqualSizes([(id<SizeNames>)counted size], NSMakeSize(180, 100)));

        // A container made with -init (width and height not tracking) takes the size through either name too.
        NSTextContainer *plain = [[[NSTextContainer alloc] init] autorelease];
        [(id<SizeNames>)plain setSize:NSMakeSize(33, 44)];
        CHECK(NSEqualSizes([plain containerSize], NSMakeSize(33, 44)));
    }
    if (failures == 0)
        printf("PASS nstextcontainer-size\n");
    return failures != 0;
}
