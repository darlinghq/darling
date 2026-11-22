// Item (a): does assigning an IOSurface to CALayer.contents change the surface's public use count, and does
// the count come back when the contents are replaced, cleared or the layer is released? Public API only:
// IOSurfaceGetUseCount, IOSurfaceIsInUse, CFGetRetainCount, CALayer.contents, -[CALayer renderInContext:].
#import "probe_common.h"
#import <IOSurface/IOSurface.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreGraphics/CoreGraphics.h>
#include <objc/message.h>

static IOSurfaceRef makeSurface(void) {
    NSDictionary *props = @{(__bridge id)kIOSurfaceWidth: @64, (__bridge id)kIOSurfaceHeight: @64,
                            (__bridge id)kIOSurfaceBytesPerElement: @4, (__bridge id)kIOSurfacePixelFormat: @((uint32_t)'BGRA')};
    return IOSurfaceCreate((__bridge CFDictionaryRef)props);
}

static NSString *st(IOSurfaceRef s) {
    return PF(@"useCount=%d isInUse=%d retainCount=%ld", (int)IOSurfaceGetUseCount(s), (int)IOSurfaceIsInUse(s), (long)CFGetRetainCount(s));
}

int main(void) {
    @autoreleasepool {
        probe_env("item-a");
        IOSurfaceRef s = makeSurface();
        if (s == NULL) { probe_line("item-a", @"IOSurfaceCreate", @"NULL"); return 1; }
        probe_line("item-a.1", @"after IOSurfaceCreate", st(s));
        CALayer *layer = [CALayer layer];
        layer.contents = (__bridge id)s;
        probe_line("item-a.2", @"after layer.contents = surface", st(s));
        probe_line("item-a.3", @"layer.contents read back is the same object", PF(@"%d", (int)((__bridge void *)layer.contents == (void *)s)));
        CALayer *l2 = [CALayer layer];
        l2.contents = (__bridge id)s;
        probe_line("item-a.4", @"after a second layer also gets the same surface", st(s));
        layer.contents = (__bridge id)s;
        probe_line("item-a.5", @"after assigning the same surface to the first layer again", st(s));
        layer.contents = nil;
        probe_line("item-a.6", @"after first layer.contents = nil", st(s));
        l2.contents = nil;
        probe_line("item-a.7", @"after second layer.contents = nil", st(s));

        IOSurfaceRef s2 = makeSurface();
        layer.contents = (__bridge id)s;
        layer.contents = (__bridge id)s2;
        probe_line("item-a.8", @"layer.contents replaced by a different surface: first surface", st(s));
        probe_line("item-a.8b", @"layer.contents replaced by a different surface: second surface", st(s2));
        layer.contents = nil;

        // Rendering.
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef bc = CGBitmapContextCreate(NULL, 64, 64, 8, 0, cs, kCGImageAlphaPremultipliedLast);
        layer.frame = CGRectMake(0, 0, 64, 64);
        layer.contents = (__bridge id)s;
        probe_line("item-a.9a", @"before -renderInContext:", st(s));
        if ([layer respondsToSelector:@selector(renderInContext:)])
            ((void (*)(id, SEL, CGContextRef))objc_msgSend)(layer, @selector(renderInContext:), bc);
        probe_line("item-a.9b", @"after -renderInContext:", st(s));
        CGContextRelease(bc);
        CGColorSpaceRelease(cs);

        // Explicit use count by the application, to see whether the layer adds or removes counts.
        IOSurfaceIncrementUseCount(s);
        probe_line("item-a.10a", @"after the application's own IOSurfaceIncrementUseCount (layer still holds the surface)", st(s));
        layer.contents = nil;
        probe_line("item-a.10b", @"after layer.contents = nil", st(s));
        IOSurfaceDecrementUseCount(s);
        probe_line("item-a.10c", @"after the application's own IOSurfaceDecrementUseCount", st(s));

        // Layer released.
        @autoreleasepool {
            CALayer *tmp = [CALayer layer];
            tmp.contents = (__bridge id)s;
            probe_line("item-a.11a", @"a temporary layer holds the surface", st(s));
            tmp = nil;
        }
        probe_line("item-a.11b", @"after the temporary layer is released", st(s));

        // A layer in a layer tree committed by CATransaction.
        CALayer *root = [CALayer layer];
        CALayer *child = [CALayer layer];
        [root addSublayer:child];
        child.contents = (__bridge id)s;
        [CATransaction flush];
        probe_line("item-a.12a", @"child layer with the surface inside a root layer after CATransaction flush", st(s));
        [child removeFromSuperlayer];
        child.contents = nil;
        [CATransaction flush];
        probe_line("item-a.12b", @"after removing it and clearing contents, flushed", st(s));
        CFRelease(s2);
        CFRelease(s);
    }
    return 0;
}
