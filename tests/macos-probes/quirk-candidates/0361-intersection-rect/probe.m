// Probe 0361: what NSIntersectionRect, NSIntersectsRect and CGRectIntersection answer for rectangles that are
// ordinary, touching, empty, and unbounded (largest double, infinity, not-a-number edges).
#import "probe_common.h"
#include <math.h>
#include <float.h>

static NSString *R(NSRect r) { return probe_rect(r); }

int main(void) {
    @autoreleasepool {
        probe_env("0361");
        const CGFloat M = CGFLOAT_MAX, INF = INFINITY, NaN = NAN;
        struct { const char *name; NSRect a, b; } cases[] = {
            {"overlap",             {{0, 0}, {10, 10}},        {{5, 5}, {10, 10}}},
            {"contained",           {{0, 0}, {10, 10}},        {{2, 2}, {3, 3}}},
            {"touching-edge",       {{0, 0}, {10, 10}},        {{10, 0}, {10, 10}}},
            {"touching-corner",     {{0, 0}, {10, 10}},        {{10, 10}, {5, 5}}},
            {"disjoint",            {{0, 0}, {10, 10}},        {{20, 20}, {5, 5}}},
            {"zero-size-inside",    {{0, 0}, {10, 10}},        {{5, 5}, {0, 0}}},
            {"zero-width-inside",   {{0, 0}, {10, 10}},        {{5, 0}, {0, 10}}},
            {"negative-size",       {{10, 10}, {-10, -10}},    {{5, 5}, {10, 10}}},
            {"max-size-vs-small",   {{0, 0}, {M, M}},          {{10, 20}, {30, 40}}},
            {"max-size-both",       {{0, 0}, {M, M}},          {{10, 10}, {M, M}}},
            {"max-origin-max-size", {{M, M}, {M, M}},          {{10, 20}, {30, 40}}},
            {"inf-size-vs-small",   {{0, 0}, {INF, INF}},      {{10, 20}, {30, 40}}},
            {"inf-origin-inf-size", {{-INF, -INF}, {INF, INF}}, {{10, 20}, {30, 40}}},
            {"min-max-size",        {{-M, -M}, {M, M}},        {{10, 20}, {30, 40}}},
            {"nan-origin",          {{NaN, NaN}, {10, 10}},    {{5, 5}, {10, 10}}},
            {"nan-size",            {{0, 0}, {NaN, NaN}},      {{5, 5}, {10, 10}}},
            {"both-max-no-overlap", {{M, M}, {10, 10}},        {{0, 0}, {10, 10}}},
        };
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
            NSRect a = cases[i].a, b = cases[i].b;
            NSRect r1 = NSIntersectionRect(a, b);
            NSRect r2 = NSIntersectionRect(b, a);
            CGRect c = CGRectIntersection(*(CGRect *)&a, *(CGRect *)&b);
            NSString *in = PF(@"%s a=%@ b=%@", cases[i].name, R(a), R(b));
            probe_line("0361.NSIntersectionRect", in, R(r1));
            probe_line("0361.NSIntersectionRect-swapped", in, R(r2));
            probe_line("0361.NSIntersectsRect", in, PF(@"%d", (int)NSIntersectsRect(a, b)));
            probe_line("0361.CGRectIntersection", in, R(*(NSRect *)&c));
            probe_line("0361.NSIsEmptyRect-of-result", in, PF(@"%d", (int)NSIsEmptyRect(r1)));
        }
        probe_line("0361.CGFLOAT_MAX", @"constant", PF(@"%g", (double)CGFLOAT_MAX));
    }
    return 0;
}
