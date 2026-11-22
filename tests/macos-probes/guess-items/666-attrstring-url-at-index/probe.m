// Probe 666: -[NSAttributedString URLAtIndex:effectiveRange:] (AppKit, deprecated) with NSLinkAttributeName values.
// Prints the URL (or nil), the effective range (a sentinel 777,777 means the range was not written), and any
// exception, for indexes at, before, between, after and past the end of link runs.
#import "probe_common.h"
#import <AppKit/AppKit.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

// Public, documented selector; declared again here so the probe builds even where an SDK header omits it.
@interface NSAttributedString (Probe666Decl)
- (NSURL *)URLAtIndex:(NSUInteger)location effectiveRange:(NSRangePointer)effectiveRange;
@end

static const NSRange SENT = {777, 777};

static NSString *rs(NSRange r) {
    return (r.location == SENT.location && r.length == SENT.length) ? @"range-untouched" : PF(@"{%lu,%lu}", (unsigned long)r.location, (unsigned long)r.length);
}

static void at(const char *id, NSString *desc, NSAttributedString *s, NSInteger idx) {
    __block NSRange r = SENT;
    __block NSURL *u = nil;
    NSString *exc = probe_try(^{ u = [s URLAtIndex:(NSUInteger)idx effectiveRange:&r]; });
    probe_line(id, PF(@"%@ ; length %lu ; index %ld", desc, (unsigned long)s.length, (long)idx),
               PF(@"url=%@ ; effectiveRange=%@ ; %@", u ? [u absoluteString] : @"nil", rs(r), exc));
}

static void atNullRange(const char *id, NSString *desc, NSAttributedString *s, NSUInteger idx) {
    __block NSURL *u = nil;
    NSString *exc = probe_try(^{ u = [s URLAtIndex:idx effectiveRange:NULL]; });
    probe_line(id, PF(@"%@ ; index %lu ; effectiveRange NULL", desc, (unsigned long)idx),
               PF(@"url=%@ ; %@", u ? [u absoluteString] : @"nil", exc));
}

static NSAttributedString *build(NSArray *parts) { // parts: @[ @[text, linkValueOrNull, otherAttrsOrNull], ... ]
    NSMutableAttributedString *m = [[NSMutableAttributedString alloc] init];
    for (NSArray *p in parts) {
        NSMutableDictionary *a = [NSMutableDictionary dictionary];
        if (p[1] != [NSNull null]) a[NSLinkAttributeName] = p[1];
        if (p.count > 2 && p[2] != [NSNull null]) [a addEntriesFromDictionary:p[2]];
        [m appendAttributedString:[[NSAttributedString alloc] initWithString:p[0] attributes:a]];
    }
    return m;
}

int main(void) {
    @autoreleasepool {
        probe_env("666");
        NSNull *N = [NSNull null];
        NSURL *u1 = [NSURL URLWithString:@"http://example.com/one"], *u2 = [NSURL URLWithString:@"http://example.com/two"];

        // A. One link in the middle: "plain" 0-4, "link" 5-8, "tail" 9-12 ("plain link tail").
        NSAttributedString *a = build(@[@[@"plain ", N], @[@"link", u1], @[@" tail", N]]);
        for (NSInteger i = 0; i <= 15; i++) at("666.A.middle-link", @"plain[0,6) link[6,10) tail[10,15)", a, i);
        atNullRange("666.A.null-range", @"link at 6", a, 6);
        atNullRange("666.A.null-range", @"no link at 0", a, 0);

        // B. No link at all.
        NSAttributedString *none = [[NSAttributedString alloc] initWithString:@"no link here"];
        at("666.B.no-link", @"plain string", none, 0);
        at("666.B.no-link", @"plain string", none, 5);
        at("666.B.no-link", @"plain string", none, 11);
        at("666.B.no-link", @"plain string", none, 12);
        at("666.B.no-link", @"plain string", none, 13);
        NSAttributedString *bold = build(@[@[@"ab", N, @{NSFontAttributeName: [NSFont boldSystemFontOfSize:12]}],
                                           @[@"cd", N], @[@"ef", N, @{NSForegroundColorAttributeName: [NSColor redColor]}]]);
        at("666.B.attr-runs", @"no link; runs differ in font/colour only", bold, 3);

        // C. Empty string and far out of range.
        NSAttributedString *empty = [[NSAttributedString alloc] initWithString:@""];
        at("666.C.empty", @"empty string", empty, 0);
        at("666.C.empty", @"empty string", empty, 1);
        at("666.C.far", @"link string", a, 1000);
        at("666.C.far", @"link string", a, (NSInteger)NSNotFound);
        at("666.C.far", @"link string", a, -1);

        // D. Adjacent runs: different URLs, the same URL object, equal URLs, link value types, mixed other attributes.
        NSAttributedString *adj = build(@[@[@"aaa", u1], @[@"bbb", u2], @[@"ccc", u2]]);
        for (NSInteger i = 0; i < 9; i += 2) at("666.D.adjacent-different", @"aaa=u1 bbb=u2 ccc=u2 (equal neighbours)", adj, i);
        NSAttributedString *sameObj = build(@[@[@"aaa", u1], @[@"bbb", u1]]);
        at("666.D.same-url", @"aaa=u1 bbb=u1 (same object)", sameObj, 1);
        at("666.D.same-url", @"aaa=u1 bbb=u1 (same object)", sameObj, 4);
        NSAttributedString *equalURLs = build(@[@[@"aaa", [NSURL URLWithString:@"http://example.com/x"]],
                                                @[@"bbb", [NSURL URLWithString:@"http://example.com/x"]]]);
        at("666.D.equal-urls", @"aaa, bbb: two NSURL objects, equal values", equalURLs, 1);
        NSAttributedString *urlThenString = build(@[@[@"aaa", [NSURL URLWithString:@"http://example.com/x"]],
                                                    @[@"bbb", @"http://example.com/x"]]);
        at("666.D.url-vs-string", @"aaa=NSURL, bbb=NSString with the same address", urlThenString, 1);
        at("666.D.url-vs-string", @"aaa=NSURL, bbb=NSString with the same address", urlThenString, 4);
        NSAttributedString *mixedAttrs = build(@[
            @[@"aaa", u1, @{NSFontAttributeName: [NSFont boldSystemFontOfSize:12]}],
            @[@"bbb", u1, @{NSForegroundColorAttributeName: [NSColor redColor]}],
            @[@"ccc", u1]]);
        at("666.D.same-link-other-attrs", @"aaa bbb ccc all link u1, other attributes differ per run", mixedAttrs, 1);
        at("666.D.same-link-other-attrs", @"aaa bbb ccc all link u1, other attributes differ per run", mixedAttrs, 4);
        at("666.D.same-link-other-attrs", @"aaa bbb ccc all link u1, other attributes differ per run", mixedAttrs, 8);
        NSAttributedString *twoLinksGap = build(@[@[@"aaa", u1], @[@"--", N], @[@"bbb", u1]]);
        at("666.D.gap", @"aaa=u1 --(no link) bbb=u1: gap between equal links", twoLinksGap, 3);
        at("666.D.gap", @"aaa=u1 --(no link) bbb=u1: gap between equal links", twoLinksGap, 1);
        at("666.D.gap", @"aaa=u1 --(no link) bbb=u1: gap between equal links", twoLinksGap, 6);

        // E. Link values that are strings: valid, invalid, empty, relative, with spaces, non-ASCII, percent-escaped.
        NSArray *strings = @[@"http://example.com/s", @"not a url with spaces", @"", @"relative/path", @"/abs/file/path",
                             @"mailto:me@example.com", @"http://example.com/ünï", @"http://example.com/a%20b",
                             @"ht!tp://bad", @"://", @"file:///tmp/x y"];
        for (NSString *v in strings) {
            NSAttributedString *s = build(@[@[@"pre ", N], @[@"LINK", v], @[@" post", N]]);
            at("666.E.string-link", PF(@"link value is an NSString: \"%@\"", v), s, 5);
        }
        // Other value types.
        at("666.E.other-type", @"link value is NSNumber 1", build(@[@[@"xxLINKxx", @1]]), 3);
        at("666.E.other-type", @"link value is NSArray", build(@[@[@"xxLINKxx", @[u1]]]), 3);
        at("666.E.other-type", @"link value is NSDictionary", build(@[@[@"xxLINKxx", @{@"k": @"v"}]]), 3);
        at("666.E.other-type", @"link value is NSData", build(@[@[@"xxLINKxx", [@"http://example.com" dataUsingEncoding:NSUTF8StringEncoding]]]), 3);
        // Invalid string run next to a valid URL run, effective range of the invalid one.
        NSAttributedString *badNext = build(@[@[@"aaa", @"not a url"], @[@"bbb", u1], @[@"ccc", @"not a url"], @[@"ddd", @"also bad"]]);
        for (NSInteger i = 0; i < 12; i += 3) at("666.E.invalid-runs", @"aaa=bad string, bbb=URL, ccc=bad string, ddd=different bad string", badNext, i);

        // F. A link covering the whole string, and the last character; link at the very start.
        NSAttributedString *whole = build(@[@[@"whole", u1]]);
        at("666.F.whole", @"whole string is one link", whole, 0);
        at("666.F.whole", @"whole string is one link", whole, 4);
        at("666.F.whole", @"whole string is one link", whole, 5);
        NSAttributedString *startLink = build(@[@[@"ab", u1], @[@"cd", N]]);
        at("666.F.start", @"link at the start", startLink, 0);
        at("666.F.start", @"link at the start", startLink, 2);
        NSAttributedString *endLink = build(@[@[@"ab", N], @[@"cd", u1]]);
        at("666.F.end", @"link at the end", endLink, 3);
        at("666.F.end", @"link at the end", endLink, 4);

        // G. Mutable string with a link added by addAttribute: same answer?
        NSMutableAttributedString *m = [[NSMutableAttributedString alloc] initWithString:@"abcdefgh"];
        [m addAttribute:NSLinkAttributeName value:u2 range:NSMakeRange(2, 3)];
        at("666.G.mutable", @"NSMutableAttributedString, link on [2,5)", m, 3);
        at("666.G.mutable", @"NSMutableAttributedString, link on [2,5)", m, 6);
        // Compare with the generic attribute accessor for the same index (what the docs say it is built on).
        NSRange er = SENT;
        id v = [m attribute:NSLinkAttributeName atIndex:3 longestEffectiveRange:&er inRange:NSMakeRange(0, m.length)];
        probe_line("666.G.compare", @"attribute:NSLinkAttributeName atIndex:3 longestEffectiveRange (same string)",
                   PF(@"value=%@ ; range=%@", v ? [v description] : @"nil", rs(er)));
        er = SENT;
        v = [m attribute:NSLinkAttributeName atIndex:6 longestEffectiveRange:&er inRange:NSMakeRange(0, m.length)];
        probe_line("666.G.compare", @"attribute:NSLinkAttributeName atIndex:6 longestEffectiveRange (same string)",
                   PF(@"value=%@ ; range=%@", v ? [v description] : @"nil", rs(er)));
    }
    return 0;
}
