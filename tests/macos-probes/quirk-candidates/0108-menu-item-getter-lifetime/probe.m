// Probe 0108: do NSMenuItem's object getters hand back an object that outlives the item, inside the same
// autorelease pool? Manual reference counting is used on purpose (build with -fno-objc-arc): the question is
// whether the caller can keep using what -title, -keyEquivalent, -image, -representedObject return after it
// released the item, without retaining it.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <objc/runtime.h>

static int gFreed;
static char kTrackerKey;

@interface Tracker : NSObject
@end
@implementation Tracker
- (void)dealloc {
    gFreed = 1;
    [super dealloc];
}
@end

typedef id (^Make)(void);
typedef void (^Set)(NSMenuItem *, id);
typedef id (^Get)(NSMenuItem *);

static void run(const char *prop, Make make, Set set, Get get, BOOL attachToValue) {
    for (int callGetter = 0; callGetter <= 1; callGetter++) {
        gFreed = 0;
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"x" action:NULL keyEquivalent:@""];
        id value = make();
        Tracker *t = [[Tracker alloc] init];
        if (attachToValue) {
            objc_setAssociatedObject(value, &kTrackerKey, t, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
            [t release];
        }
        set(item, value);
        id got = callGetter ? get(item) : nil;
        BOOL sameObject = (got == value);
        [value release];   // the caller's own reference to the value
        if (!attachToValue)
            [t release];
        [item release];    // the item goes away and releases what it holds
        int freedBeforeDrain = gFreed;
        [pool drain];
        probe_line("0108.getter", PF(@"%s ; getter called before the item was released: %@ ; returned object is the one that was set: %@",
                                      prop, callGetter ? @"yes" : @"no (control)", callGetter ? (sameObject ? @"yes" : @"no") : @"n/a"),
                   PF(@"value freed before the autorelease pool drained: %@ ; freed after drain: %@",
                      freedBeforeDrain ? @"yes" : @"no", gFreed ? @"yes" : @"no"));
    }
}

int main(void) {
    NSAutoreleasePool *outer = [[NSAutoreleasePool alloc] init];
    probe_env("0108");
    probe_app();
    static int n;
    run("title",
        ^id { return [[NSString alloc] initWithFormat:@"probe-title-%d-long-enough-for-the-heap", ++n]; },
        ^(NSMenuItem *i, id v) { [i setTitle:v]; }, ^id(NSMenuItem *i) { return [i title]; }, YES);
    run("keyEquivalent",
        ^id { return [[NSString alloc] initWithFormat:@"probe-key-%d-long-enough-for-the-heap", ++n]; },
        ^(NSMenuItem *i, id v) { [i setKeyEquivalent:v]; }, ^id(NSMenuItem *i) { return [i keyEquivalent]; }, YES);
    run("attributedTitle",
        ^id { return [[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@"probe-atitle-%d-long-enough-for-the-heap", ++n]]; },
        ^(NSMenuItem *i, id v) { [i setAttributedTitle:v]; }, ^id(NSMenuItem *i) { return [i attributedTitle]; }, YES);
    run("image",
        ^id { return [[NSImage alloc] initWithSize:NSMakeSize(4, 4)]; },
        ^(NSMenuItem *i, id v) { [i setImage:v]; }, ^id(NSMenuItem *i) { return [i image]; }, YES);
    run("representedObject",
        ^id { return [[NSObject alloc] init]; },
        ^(NSMenuItem *i, id v) { [i setRepresentedObject:v]; }, ^id(NSMenuItem *i) { return [i representedObject]; }, YES);
    run("toolTip",
        ^id { return [[NSString alloc] initWithFormat:@"probe-tip-%d-long-enough-for-the-heap", ++n]; },
        ^(NSMenuItem *i, id v) { [i setToolTip:v]; }, ^id(NSMenuItem *i) { return [i toolTip]; }, YES);
    [outer drain];
    return 0;
}
