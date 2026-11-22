// Probe 0122: which device pixels a clip touches when its edge is a hair away from a pixel boundary, and what
// CGContextGetClipBoundingBox reports. Public CoreGraphics only. Writes alpha coverage of the pixels around
// one clip edge, with anti-aliasing on and off, at CTM scale 1 and at a fractional scale.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>

#define W 24
#define H 24

static void env(void) {
    struct utsname u;
    uname(&u);
    char ver[64] = "?", build[64] = "?";
    size_t n = sizeof ver;
    sysctlbyname("kern.osproductversion", ver, &n, NULL, 0);
    n = sizeof build;
    sysctlbyname("kern.osversion", build, &n, NULL, 0);
    printf("PROBE 0122 env -> macOS %s (build %s) ; machine %s\n", ver, build, u.machine);
}

static int alphaAt(const unsigned char *px, int x, int yFromBottom) {
    int row = H - 1 - yFromBottom;
    return px[(row * W + x) * 4 + 3];
}

// kind 0: CGContextClipToRect; kind 1: add a rectangle path and CGContextClip; kind 2: CGContextClipToRects
static void run(int kind, int aa, double scale, double e, int edgeIsMax) {
    unsigned char *px = calloc(W * H * 4, 1);
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    CGContextRef c = CGBitmapContextCreate(px, W, H, 8, W * 4, cs, kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(cs);
    CGContextSetShouldAntialias(c, aa);
    CGContextScaleCTM(c, scale, scale);
    // Device-space rectangle 4..12 in both axes; the edge under test is moved by e device pixels.
    double x0 = 4, y0 = 4, x1 = 12, y1 = 12;
    if (edgeIsMax) { x1 += e; y1 += e; } else { x0 += e; y0 += e; }
    CGRect r = CGRectMake(x0 / scale, y0 / scale, (x1 - x0) / scale, (y1 - y0) / scale);
    if (kind == 0) CGContextClipToRect(c, r);
    else if (kind == 1) { CGContextBeginPath(c); CGContextAddRect(c, r); CGContextClip(c); }
    else CGContextClipToRects(c, &r, 1);
    CGRect bb = CGContextGetClipBoundingBox(c);
    CGContextSetRGBFillColor(c, 1, 1, 1, 1);
    CGContextFillRect(c, CGRectMake(-100, -100, 1000, 1000));
    const char *kn[] = {"ClipToRect", "AddRect+Clip", "ClipToRects"};
    // Pixels around the tested edge along a row/column well inside the other axis (index 8).
    int center = edgeIsMax ? 12 : 4;
    printf("PROBE 0122.clip %s aa=%d scale=%.6f edge=%s%+.4f -> clipBBox=[%.6f %.6f %.6f %.6f] alpha(px %d..%d, x-axis)=",
           kn[kind], aa, scale, edgeIsMax ? "max" : "min", e, bb.origin.x * scale, bb.origin.y * scale,
           (bb.origin.x + bb.size.width) * scale, (bb.origin.y + bb.size.height) * scale, center - 2, center + 1);
    for (int x = center - 2; x <= center + 1; x++) printf("%d%s", alphaAt(px, x, 8), x == center + 1 ? "" : ",");
    printf(" ; alpha(px %d..%d, y-axis)=", center - 2, center + 1);
    for (int y = center - 2; y <= center + 1; y++) printf("%d%s", alphaAt(px, 8, y), y == center + 1 ? "" : ",");
    printf("\n");
    fflush(stdout);
    CGContextRelease(c);
    free(px);
}

int main(void) {
    env();
    const double es[] = {-0.01, -0.002, -0.0005, -0.0001, 0.0, 0.0001, 0.0005, 0.002, 0.01, 0.25, 0.5};
    const double scales[] = {1.0, 719.0 / 479.0, 2.0};
    for (int kind = 0; kind < 3; kind++)
        for (int aa = 1; aa >= 0; aa--)
            for (size_t s = 0; s < sizeof scales / sizeof scales[0]; s++)
                for (int mx = 1; mx >= 0; mx--)
                    for (size_t i = 0; i < sizeof es / sizeof es[0]; i++) run(kind, aa, scales[s], es[i], mx);
    return 0;
}
