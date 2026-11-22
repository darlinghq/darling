// Probe 638: draw order of sibling CALayers with different zPosition, rendered headless by a CARenderer into a
// Metal texture (no window, no activation, no window server drawing). Prints which layer's colour is on top
// at the centre of the overlap, as the name of the layer (red A, blue B, green C) or the raw colour.
// If no Metal device exists, or CARenderer cannot be made, every line says so.
#import "probe_common.h"
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#include <math.h>

static const int W = 100, H = 100;
static id<MTLDevice> gDev;
static id<MTLCommandQueue> gQueue;

// Two or three 60x60 opaque layers whose rectangles overlap at the centre (50,50) of a 100x100 root.
static CALayer *makeLayer(CGFloat x, CGFloat y, CGColorRef c, CGFloat z) {
    CALayer *l = [CALayer layer];
    l.anchorPoint = CGPointMake(0, 0);
    l.frame = CGRectMake(x, y, 60, 60);
    l.backgroundColor = c;
    l.opaque = YES;
    l.zPosition = z;
    return l;
}
static CALayer *A, *B, *C;
static CALayer *root(void) {
    CGColorRef red = CGColorCreateGenericRGB(1, 0, 0, 1), blue = CGColorCreateGenericRGB(0, 0, 1, 1),
               green = CGColorCreateGenericRGB(0, 1, 0, 1);
    CALayer *r = [CALayer layer];
    r.anchorPoint = CGPointMake(0, 0);
    r.bounds = CGRectMake(0, 0, W, H);
    r.position = CGPointZero;
    r.backgroundColor = CGColorCreateGenericRGB(1, 1, 1, 1);
    A = makeLayer(10, 10, red, 0);
    B = makeLayer(30, 30, blue, 0);
    C = makeLayer(20, 20, green, 0);
    return r;
}

// Render the layer tree at time `t` into an offscreen texture; return the centre pixel as rrggbb.
static NSString *centrePixel(CALayer *rootLayer, CFTimeInterval t) {
    MTLTextureDescriptor *td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                   width:W height:H mipmapped:NO];
    td.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if defined(__arm64__)
    td.storageMode = MTLStorageModeShared;
#else
    td.storageMode = MTLStorageModeManaged;
#endif
    id<MTLTexture> tex = [gDev newTextureWithDescriptor:td];
    CARenderer *r = [CARenderer rendererWithMTLTexture:tex options:nil];
    if (r == nil) return @"CARenderer nil";
    r.layer = rootLayer;
    r.bounds = CGRectMake(0, 0, W, H);
    [CATransaction flush];
    [r beginFrameAtTime:t timeStamp:NULL];
    [r addUpdateRect:CGRectMake(0, 0, W, H)];
    [r render];
    [r endFrame];
    if (tex.storageMode == MTLStorageModeManaged) {
#if !defined(__arm64__)
        id<MTLCommandBuffer> cb = [gQueue commandBuffer];
        id<MTLBlitCommandEncoder> bl = [cb blitCommandEncoder];
        [bl synchronizeResource:tex];
        [bl endEncoding];
        [cb commit];
        [cb waitUntilCompleted];
#endif
    }
    uint8_t px[4] = {0};
    [tex getBytes:px bytesPerRow:4 fromRegion:MTLRegionMake2D(W / 2, H / 2, 1, 1) mipmapLevel:0];
    unsigned rr = px[2], gg = px[1], bb = px[0]; // BGRA
    if (rr > 200 && gg < 60 && bb < 60) return @"A(red)";
    if (rr < 60 && gg < 60 && bb > 200) return @"B(blue)";
    if (rr < 60 && gg > 200 && bb < 60) return @"C(green)";
    if (rr > 200 && gg > 200 && bb > 200) return @"white(background)";
    if (rr == 0 && gg == 0 && bb == 0 && px[3] == 0) return @"transparent-black(nothing drawn?)";
    return PF(@"other:%02x%02x%02x", rr, gg, bb);
}

// Add the layers in the order given (a string of A, B, C), set their z, render the centre pixel.
static void run(const char *id, NSString *desc, NSString *order, double za, double zb, double zc,
                void (^extra)(CALayer *rootLayer)) {
    @autoreleasepool {
        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        CALayer *r = root();
        A.zPosition = za; B.zPosition = zb; C.zPosition = zc;
        for (NSUInteger i = 0; i < order.length; i++) {
            unichar ch = [order characterAtIndex:i];
            [r addSublayer:ch == 'A' ? A : ch == 'B' ? B : C];
        }
        if (extra) extra(r);
        [CATransaction commit];
        probe_line(id, PF(@"%@ ; added in order %@ ; z A=%g B=%g C=%g", desc, order, za, zb, zc),
                   centrePixel(r, CACurrentMediaTime()));
    }
}

// An animation on the zPosition of layer `l` that is in progress (about half way) at the render time.
static void addAnim(CALayer *l, CAAnimation *a) {
    a.beginTime = [l convertTime:CACurrentMediaTime() fromLayer:nil] - 50;
    a.duration = 100;
    a.removedOnCompletion = NO;
    a.fillMode = kCAFillModeBoth;
    [l addAnimation:a forKey:@"z"];
}

int main(void) {
    @autoreleasepool {
        probe_env("638");
        gDev = MTLCreateSystemDefaultDevice();
        if (gDev == nil) { probe_line("638", @"MTLCreateSystemDefaultDevice", @"nil: no Metal device, nothing can be observed"); return 0; }
        gQueue = [gDev newCommandQueue];
        probe_line("638", @"Metal device", @"present");

        // 1. Plain model values.
        run("638.1.equal", @"equal zPosition: array order decides", @"AB", 0, 0, 0, nil);
        run("638.1.equal", @"equal zPosition: array order decides", @"BA", 0, 0, 0, nil);
        run("638.1.equal", @"equal non-zero zPosition", @"AB", 5, 5, 0, nil);
        run("638.1.higher-first", @"higher z added first", @"AB", 1, 0, 0, nil);
        run("638.1.higher-last", @"higher z added last", @"AB", 0, 1, 0, nil);
        run("638.1.negative-last", @"negative z added last", @"AB", 0, -1, 0, nil);
        run("638.1.negative-first", @"negative z added first", @"AB", -1, 0, 0, nil);
        run("638.1.both-negative", @"both negative", @"AB", -1, -2, 0, nil);
        run("638.1.tiny", @"tiny z difference", @"AB", 1e-6, 0, 0, nil);
        run("638.1.large", @"huge z values", @"AB", 1e30, 1e29, 0, nil);
        run("638.1.negzero", @"-0 against 0", @"AB", 0, -0.0, 0, nil);
        run("638.1.fraction", @"fractional z", @"AB", 0.2, 0.5, 0, nil);
        run("638.1.triple", @"three layers z 2,1,3", @"ABC", 2, 1, 3, nil);
        run("638.1.triple", @"three layers z 1,3,2", @"ABC", 1, 3, 2, nil);
        run("638.1.triple", @"three layers z 0,0,1", @"ABC", 0, 0, 1, nil);
        run("638.1.triple", @"three layers z 3,3,1 (tie between A and B)", @"ABC", 3, 3, 1, nil);
        run("638.1.triple", @"three layers z 3,3,1 (tie between A and B)", @"BAC", 3, 3, 1, nil);

        // 2. NaN and infinities. "NaN counts as 0" is the question: compare with the neighbours' values.
        double nan = NAN;
        run("638.2.nan", @"A NaN, B 1 (NaN as 0: B on top; array order: B on top)", @"AB", nan, 1, 0, nil);
        run("638.2.nan", @"A NaN, B -1, added AB (NaN as 0: A on top; array order: B on top)", @"AB", nan, -1, 0, nil);
        run("638.2.nan", @"A 1, B NaN, added AB (NaN as 0: A on top; array order: B on top)", @"AB", 1, nan, 0, nil);
        run("638.2.nan", @"A -1, B NaN, added AB (NaN as 0: B on top)", @"AB", -1, nan, 0, nil);
        run("638.2.nan", @"A 1, B NaN, added BA (NaN as 0: A on top)", @"BA", 1, nan, 0, nil);
        run("638.2.nan", @"A NaN, B NaN", @"AB", nan, nan, 0, nil);
        run("638.2.nan", @"A 2, B NaN, C 1, added ABC", @"ABC", 2, nan, 1, nil);
        run("638.2.nan", @"A 1, B NaN, C -1, added ABC", @"ABC", 1, nan, -1, nil);
        run("638.2.inf", @"A +inf, B 1e30", @"AB", INFINITY, 1e30, 0, nil);
        run("638.2.inf", @"A 0, B +inf", @"AB", 0, INFINITY, 0, nil);
        run("638.2.inf", @"A -inf, B -1e30, added AB", @"AB", -INFINITY, -1e30, 0, nil);
        run("638.2.inf", @"A -inf, B 0, added BA", @"BA", -INFINITY, 0, 0, nil);

        // 3. Depth only among siblings: a child's z does not lift it above a sibling of its parent.
        run("638.3.nested", @"B inside A (z of B child 10), C sibling of A z 1; C vs the child", @"AC", 0, 0, 1, ^(CALayer *r) {
            [B removeFromSuperlayer];
            B.frame = CGRectMake(20, 20, 60, 60);
            B.zPosition = 10;
            [A addSublayer:B];
        });

        // 4. Perspective on the parent: does ordering still follow zPosition?
        run("638.4.perspective", @"parent sublayerTransform m34=-1/500, A z 1, B 0, added AB", @"AB", 1, 0, 0, ^(CALayer *r) {
            CATransform3D t = CATransform3DIdentity; t.m34 = -1.0 / 500; r.sublayerTransform = t; });
        run("638.4.perspective", @"parent sublayerTransform m34=-1/500, A 0, B 1, added AB", @"AB", 0, 1, 0, ^(CALayer *r) {
            CATransform3D t = CATransform3DIdentity; t.m34 = -1.0 / 500; r.sublayerTransform = t; });

        // 5. Animated zPosition (animation in progress, about half way): model values order one way,
        // presentation values the other. A model z 0, B model z 5: model order puts B on top.
        CABasicAnimation *basic = [CABasicAnimation animationWithKeyPath:@"zPosition"];
        basic.fromValue = @0; basic.toValue = @100; // about 50 at render time
        run("638.5.basic", @"A animates z 0->100 (about 50 now), B model z 5; presented order puts A on top",
            @"AB", 0, 5, 0, ^(CALayer *r) { addAnim(A, [basic copy]); });
        CABasicAnimation *down = [CABasicAnimation animationWithKeyPath:@"zPosition"];
        down.fromValue = @10; down.toValue = @(-100);
        run("638.5.basic", @"A model z 6 animates 10->-100 (about -45 now), B model z 0: A should drop below B",
            @"AB", 6, 0, 0, ^(CALayer *r) { addAnim(A, [down copy]); });
        CAKeyframeAnimation *kf = [CAKeyframeAnimation animationWithKeyPath:@"zPosition"];
        kf.values = @[@0, @100, @100]; kf.keyTimes = @[@0, @0.5, @1];
        run("638.5.keyframe", @"A keyframe animation of z (0, 100, 100), B model z 5",
            @"AB", 0, 5, 0, ^(CALayer *r) { addAnim(A, [kf copy]); });
        CAAnimationGroup *group = [CAAnimationGroup animation];
        CABasicAnimation *inner = [CABasicAnimation animationWithKeyPath:@"zPosition"];
        inner.fromValue = @100; inner.toValue = @100;
        group.animations = @[inner];
        run("638.5.group", @"A group animation containing a z animation fixed at 100, B model z 5 (unhandled animation class?)",
            @"AB", 0, 5, 0, ^(CALayer *r) { addAnim(A, [group copy]); });
        run("638.5.group", @"same group on A, with A model z 9 and B model z 5",
            @"AB", 9, 5, 0, ^(CALayer *r) { addAnim(A, [group copy]); });
        CASpringAnimation *spring = [CASpringAnimation animationWithKeyPath:@"zPosition"];
        spring.fromValue = @100; spring.toValue = @100;
        run("638.5.spring", @"A spring animation of z fixed at 100, B model z 5",
            @"AB", 0, 5, 0, ^(CALayer *r) { addAnim(A, [spring copy]); });
        CABasicAnimation *add = [CABasicAnimation animationWithKeyPath:@"zPosition"];
        add.fromValue = @0; add.toValue = @100; add.additive = YES;
        run("638.5.additive", @"A additive z animation +0->+100 on model z 0 (about 50), B model z 5",
            @"AB", 0, 5, 0, ^(CALayer *r) { addAnim(A, [add copy]); });
        CABasicAnimation *byOnly = [CABasicAnimation animationWithKeyPath:@"zPosition"];
        byOnly.byValue = @100;
        run("638.5.byvalue", @"A basic animation with only byValue 100 (model 0 -> 100), B model z 5",
            @"AB", 0, 5, 0, ^(CALayer *r) { addAnim(A, [byOnly copy]); });
        CABasicAnimation *nanAnim = [CABasicAnimation animationWithKeyPath:@"zPosition"];
        nanAnim.fromValue = @(NAN); nanAnim.toValue = @(NAN);
        run("638.5.nan", @"A animation from NaN to NaN, B model z -1, added AB (NaN as 0: A on top)",
            @"AB", 0, -1, 0, ^(CALayer *r) { addAnim(A, [nanAnim copy]); });
        run("638.5.on-b", @"animation on the later sibling B (z 0->100), A model z 5, added AB",
            @"AB", 5, 0, 0, ^(CALayer *r) { addAnim(B, [basic copy]); });
        run("638.5.other-key", @"A animates opacity only (not z), B model z 5: z order unaffected",
            @"AB", 0, 5, 0, ^(CALayer *r) {
                CABasicAnimation *o = [CABasicAnimation animationWithKeyPath:@"opacity"];
                o.fromValue = @1; o.toValue = @1; addAnim(A, o); });
    }
    return 0;
}
