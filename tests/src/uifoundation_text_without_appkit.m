// CFLAGS: -framework Foundation -framework CoreGraphics
// Copyright (C) 2026 Lubos Dolezel
//
// The text system works from UIFoundation alone. On macOS NSTextStorage,
// NSLayoutManager, NSTextContainer, NSShadow and NSTextAttachment are
// UIFoundation classes, and UIFoundation does not link against AppKit (Mac OS
// X 10.14.4 and macOS 26.7.3 load commands), so they work in a process that
// never loads AppKit. Darling's UIFoundation reaches AppKit only through a
// private provider that AppKit registers; this program links Foundation and
// CoreGraphics, opens UIFoundation with dlopen and checks:
//   - opening UIFoundation and using the text classes does not load AppKit;
//   - without a provider: a new shadow has no colour and -set does nothing, a
//     new attachment has no cell, drawing does nothing, and text without a
//     font attribute gets glyphs and its layout returns;
//   - layout with a font object of fixed metrics (every glyph 10 points
//     wide, lines 12 points high) gives the expected glyph ranges, line
//     fragment rects, used rects and glyph locations, for one line and for
//     wrapped lines;
//   - with a provider registered by the test, the layout manager fills text
//     backgrounds and strokes underlines into the provider's bitmap
//     CGContext, with the colour objects found in the attributes, and the
//     shadow and attachment take their colour and cell from the provider.
//
// With the argument --appkit, AppKit is opened first and only the layout
// checks run. They give the same results whichever framework defines the
// classes, so running this mode on a build where AppKit still defines them
// checks the expected values against that implementation.
//
// Prints PASS or FAIL lines, then ALL PASSED or FAILED n.
#import <Foundation/Foundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <math.h>
#include <objc/runtime.h>
#include <stdio.h>
#include <string.h>

#define APPKIT_PATH "/System/Library/Frameworks/AppKit.framework/AppKit"
#define UIFOUNDATION_PATH "/System/Library/PrivateFrameworks/UIFoundation.framework/UIFoundation"

static int failures;
static void expect(BOOL ok, const char *format, ...) __attribute__((format(printf, 2, 3)));
static void expect(BOOL ok, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    printf("%s ", ok ? "PASS" : "FAIL");
    vprintf(format, ap);
    printf("\n");
    va_end(ap);
    if (!ok)
        failures++;
}

static BOOL near(CGFloat a, CGFloat b)
{
    return fabs(a - b) < 0.01;
}

static BOOL rectIs(NSRect r, CGFloat x, CGFloat y, CGFloat w, CGFloat h)
{
    return near(r.origin.x, x) && near(r.origin.y, y) && near(r.size.width, w) && near(r.size.height, h);
}

static const char *rectString(NSRect r)
{
    return [NSStringFromRect(r) UTF8String];
}

static BOOL appKitLoaded(void)
{
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char *name = _dyld_get_image_name(i);
        if (name != NULL && strstr(name, "/AppKit.framework/") != NULL)
            return YES;
    }
    return NO;
}

// The AppKit headers are not used; only what is called is declared. NSGlyph
// is the type Darling's NSFont.h declares (NSUInteger; macOS uses unsigned
// int), so that the glyph arrays the text system hands to the test font
// have the element size it writes.
typedef NSUInteger NSGlyph;

@protocol TextSystemMethods
- (id)initWithString:(NSString *)string attributes:(NSDictionary *)attributes;
- (id)initWithContainerSize:(NSSize)size;
- (void)addLayoutManager:(id)layoutManager;
- (void)addTextContainer:(id)container;
- (void)setLineFragmentPadding:(CGFloat)padding;
- (void)ensureLayoutForTextContainer:(id)container;
- (NSUInteger)numberOfGlyphs;
- (NSRange)glyphRangeForTextContainer:(id)container;
- (NSRect)lineFragmentRectForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)range;
- (NSRect)lineFragmentUsedRectForGlyphAtIndex:(NSUInteger)glyphIndex effectiveRange:(NSRangePointer)range;
- (NSPoint)locationForGlyphAtIndex:(NSUInteger)glyphIndex;
- (NSRect)usedRectForTextContainer:(id)container;
- (NSRect)boundingRectForGlyphRange:(NSRange)range inTextContainer:(id)container;
- (void)drawBackgroundForGlyphRange:(NSRange)range atPoint:(NSPoint)origin;
- (void)drawGlyphsForGlyphRange:(NSRange)range atPoint:(NSPoint)origin;
- (void)drawUnderlineForGlyphRange:(NSRange)glyphRange
                     underlineType:(NSInteger)underlineType
                    baselineOffset:(CGFloat)baselineOffset
                  lineFragmentRect:(NSRect)lineRect
            lineFragmentGlyphRange:(NSRange)lineGlyphRange
                   containerOrigin:(NSPoint)containerOrigin;
- (id)shadowColor;
- (void)set;
- (id)attachmentCell;
- (void)setContents:(NSData *)contents;
- (id)imageForBounds:(CGRect)bounds attributes:(NSDictionary *)attributes location:(id)location textContainer:(id)container;
@end

// A font with fixed metrics: glyph = character, every glyph 10 points wide,
// ascender 8, line height 12. It answers the messages the text system sends
// to the font in the font attribute.
@interface TestBoxFont : NSObject
@end

@implementation TestBoxFont
- (CGFloat)ascender
{
    return 8;
}
- (CGFloat)descender
{
    return -4;
}
- (CGFloat)defaultLineHeightForFont
{
    return 12;
}
- (void)getGlyphs:(NSGlyph *)glyphs forCharacters:(const unichar *)characters length:(NSUInteger)length
{
    for (NSUInteger i = 0; i < length; i++)
        glyphs[i] = characters[i];
}
- (NSSize)advancementForGlyph:(NSGlyph)glyph
{
    return NSMakeSize(10, 0);
}
- (void)getAdvancements:(NSSize *)advancements forGlyphs:(const NSGlyph *)glyphs count:(NSUInteger)count
{
    for (NSUInteger i = 0; i < count; i++)
        advancements[i] = NSMakeSize(10, 0);
}
// Answered as Darling's NSFont answers it, which is what the text system is
// written against: the advance of `current`, and none for NSNullGlyph.
- (NSPoint)positionOfGlyph:(NSGlyph)current precededByGlyph:(NSGlyph)previous isNominal:(BOOL *)isNominal
{
    if (isNominal)
        *isNominal = YES;
    return NSMakePoint(current == 0 /* NSNullGlyph */ ? 0 : 10, 0);
}
- (void)set
{
}
// With AppKit loaded, the text storage's attribute fixing replaces a font
// that does not cover the characters of its range; this one covers them all.
- (NSCharacterSet *)coveredCharacterSet
{
    return [[NSCharacterSet characterSetWithCharactersInString: @""] invertedSet];
}
@end

// The drawing context the test provider reports.
static CGContextRef currentContext;

// A colour object as the layout manager uses one from an attribute.
@interface TestColor : NSObject {
@public
    CGFloat r, g, b, a;
}
@end

static BOOL shadowAskedForCGColor;

@implementation TestColor
+ (instancetype)r:(CGFloat)r g:(CGFloat)g b:(CGFloat)b a:(CGFloat)a
{
    TestColor *c = [[[self alloc] init] autorelease];
    c->r = r;
    c->g = g;
    c->b = b;
    c->a = a;
    return c;
}
- (void)setFill
{
    CGContextSetRGBFillColor(currentContext, r, g, b, a);
}
- (void)setStroke
{
    CGContextSetRGBStrokeColor(currentContext, r, g, b, a);
}
- (void)set
{
    [self setFill];
    [self setStroke];
}
- (CGColorRef)CGColorRef
{
    shadowAskedForCGColor = YES;
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGFloat components[4] = {r, g, b, a};
    CGColorRef color = CGColorCreate(space, components);
    CGColorSpaceRelease(space);
    return color;
}
@end

@interface TestCell : NSObject {
@public
    id attachment;
}
@end

@implementation TestCell
- (void)setAttachment:(id)value
{
    attachment = value;
}
@end

static id testImage;

// Stands in for AppKit: reports the test's bitmap context, unflipped, and
// hands out test colours, cells and images.
@interface TestProvider : NSObject
@end

@implementation TestProvider
- (CGContextRef)currentCGContext
{
    return currentContext;
}
- (BOOL)currentContextIsFlipped
{
    return NO;
}
- (BOOL)currentContextIsDrawingToScreen
{
    return NO;
}
- (id)textColorForRole:(NSInteger)role
{
    return [TestColor r: 0 g: 0 b: 0 a: 1];
}
- (id)defaultTextFont
{
    return nil;
}
- (id)imageWithData:(NSData *)data
{
    return testImage;
}
- (id)newTextAttachmentCell
{
    return [[TestCell alloc] init];
}
@end

static Class classNamed(const char *name)
{
    Class cls = objc_getClass(name);
    expect(cls != Nil, "%s exists", name);
    return cls;
}

// A storage with one layout manager and one container, padding 5.
static id makeLayout(NSString *string, NSDictionary *attributes, NSSize size, id *managerOut, id *containerOut)
{
    id storage = [[[classNamed("NSTextStorage") alloc] initWithString: string attributes: attributes] autorelease];
    id manager = [[[classNamed("NSLayoutManager") alloc] init] autorelease];
    id container = [[[classNamed("NSTextContainer") alloc] initWithContainerSize: size] autorelease];
    [container setLineFragmentPadding: 5];
    [manager addTextContainer: container];
    [storage addLayoutManager: manager];
    [manager ensureLayoutForTextContainer: container];
    *managerOut = manager;
    *containerOut = container;
    return storage;
}

static void checkLayout(void)
{
    NSDictionary *attributes = [NSDictionary dictionaryWithObject: [[[TestBoxFont alloc] init] autorelease]
                                                            forKey: @"NSFont"];
    id manager, container;

    // One line: five glyphs of 10 points after the 5 point padding.
    id storage = makeLayout(@"abcde", attributes, NSMakeSize(200, 100), &manager, &container);
    expect([manager numberOfGlyphs] == 5, "one line: 5 glyphs (%lu)", (unsigned long) [manager numberOfGlyphs]);
    NSRange range = [manager glyphRangeForTextContainer: container];
    expect(NSEqualRanges(range, NSMakeRange(0, 5)), "one line: all glyphs in the container (%s)",
          [NSStringFromRange(range) UTF8String]);
    NSRange lineRange;
    NSRect fragment = [manager lineFragmentRectForGlyphAtIndex: 0 effectiveRange: &lineRange];
    expect(rectIs(fragment, 0, 0, 200, 12) && NSEqualRanges(lineRange, NSMakeRange(0, 5)),
          "one line: fragment rect {0,0,200,12} for glyphs 0-4 (%s, %s)", rectString(fragment),
          [NSStringFromRange(lineRange) UTF8String]);
    NSRect used = [manager lineFragmentUsedRectForGlyphAtIndex: 0 effectiveRange: NULL];
    expect(rectIs(used, 0, 0, 60, 12), "one line: used rect {0,0,60,12} (%s)", rectString(used));
    NSPoint location = [manager locationForGlyphAtIndex: 0];
    expect(near(location.x, 5), "one line: first glyph at x 5 (%g)", location.x);
    NSRect bounds = [manager boundingRectForGlyphRange: NSMakeRange(0, 5) inTextContainer: container];
    expect(rectIs(bounds, 5, 0, 50, 12), "one line: glyph bounds {5,0,50,12} (%s)", rectString(bounds));
    NSRect all = [manager usedRectForTextContainer: container];
    expect(rectIs(all, 0, 0, 60, 12), "one line: container used rect {0,0,60,12} (%s)", rectString(all));
    (void) storage;

    // Wrapped: 55 points are left after the padding, so each "xxxx " word
    // (50 points) takes a line.
    storage = makeLayout(@"aaaa bbbb cccc", attributes, NSMakeSize(65, 200), &manager, &container);
    expect([manager numberOfGlyphs] == 14, "wrapped: 14 glyphs (%lu)", (unsigned long) [manager numberOfGlyphs]);
    static const NSUInteger starts[] = {0, 5, 10}, lengths[] = {5, 5, 4};
    NSUInteger glyphIndex = 0, line = 0;
    while (glyphIndex < [manager numberOfGlyphs] && line < 4) {
        fragment = [manager lineFragmentRectForGlyphAtIndex: glyphIndex effectiveRange: &lineRange];
        if (line < 3)
            expect(NSEqualRanges(lineRange, NSMakeRange(starts[line], lengths[line])) &&
                           rectIs(fragment, 0, 12 * line, 65, 12),
                   "wrapped: line %lu holds glyphs %s in {0,%g,65,12} (%s)", (unsigned long) line,
                   [NSStringFromRange(lineRange) UTF8String], 12.0 * line, rectString(fragment));
        if (lineRange.length == 0)
            break;
        glyphIndex = NSMaxRange(lineRange);
        line++;
    }
    expect(line == 3, "wrapped: three lines (%lu)", (unsigned long) line);
    all = [manager usedRectForTextContainer: container];
    expect(near(all.size.height, 36), "wrapped: used height 36 (%s)", rectString(all));
}

// Pixel (x, y) in CG coordinates (origin bottom left) of an RGBA bitmap.
static const uint8_t *pixel(const uint8_t *data, size_t width, size_t height, size_t x, size_t y)
{
    return data + ((height - 1 - y) * width + x) * 4;
}

static void checkWithoutProvider(void)
{
    id shadow = [[[classNamed("NSShadow") alloc] init] autorelease];
    expect([shadow shadowColor] == nil, "no provider: a new shadow has no colour");
    [(id<TextSystemMethods>) shadow set];
    expect(YES, "no provider: -[NSShadow set] returns");

    id attachment = [[[classNamed("NSTextAttachment") alloc] init] autorelease];
    expect([attachment attachmentCell] == nil, "no provider: a new attachment has no cell");
    [attachment setContents: [@"data" dataUsingEncoding: NSUTF8StringEncoding]];
    expect([attachment imageForBounds: CGRectZero attributes: nil location: nil textContainer: nil] == nil,
          "no provider: an attachment makes no image");

    // Text without a font attribute: no fallback font without AppKit, so no
    // line height either; the glyphs exist and layout returns, but no line
    // fragment is placed.
    id manager, container;
    makeLayout(@"no font here", nil, NSMakeSize(200, 100), &manager, &container);
    expect([manager numberOfGlyphs] == 12, "no provider: text without a font gets one glyph per character (%lu)",
          (unsigned long) [manager numberOfGlyphs]);
    NSRange range = [manager glyphRangeForTextContainer: container];
    expect(NSMaxRange(range) <= 12, "no provider: layout of text without a font returns (%s)",
          [NSStringFromRange(range) UTF8String]);

    // Drawing has nowhere to go.
    NSDictionary *attributes = [NSDictionary
            dictionaryWithObjectsAndKeys: [[[TestBoxFont alloc] init] autorelease], @"NSFont",
                                          [TestColor r: 1 g: 0 b: 0 a: 1], @"NSBackgroundColor",
                                          [NSNumber numberWithInteger: 1], @"NSUnderline", nil];
    makeLayout(@"abcde", attributes, NSMakeSize(200, 100), &manager, &container);
    [manager drawBackgroundForGlyphRange: NSMakeRange(0, 5) atPoint: NSZeroPoint];
    [manager drawGlyphsForGlyphRange: NSMakeRange(0, 5) atPoint: NSZeroPoint];
    expect(YES, "no provider: drawing returns");
}

static void checkWithProvider(void)
{
    void (*setProvider)(id) = (void (*)(id)) dlsym(RTLD_DEFAULT, "_UIFoundationSetGraphicsContextProvider");
    expect(setProvider != NULL, "the provider registration function is exported");
    if (setProvider == NULL)
        return;
    TestProvider *provider = [[[TestProvider alloc] init] autorelease];
    setProvider(provider);

    const size_t width = 100, height = 24;
    uint8_t *data = calloc(width * height, 4);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    currentContext = CGBitmapContextCreate(data, width, height, 8, width * 4, space,
                                           kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(space);
    expect(currentContext != NULL, "bitmap context");
    if (currentContext == NULL)
        return;

    // Background of "bcd" (glyphs 1-3): x 15-45; the line is 12 high and,
    // the context being unflipped, at the bottom.
    NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
    [attributes setObject: [[[TestBoxFont alloc] init] autorelease] forKey: @"NSFont"];
    [attributes setObject: [TestColor r: 0 g: 0 b: 1 a: 1] forKey: @"NSUnderlineColor"];
    id manager, container;
    id storage = makeLayout(@"abcde", attributes, NSMakeSize(200, 100), &manager, &container);
    [storage addAttribute: @"NSBackgroundColor" value: [TestColor r: 1 g: 0 b: 0 a: 1] range: NSMakeRange(1, 3)];
    [manager ensureLayoutForTextContainer: container];
    [manager drawBackgroundForGlyphRange: NSMakeRange(0, 5) atPoint: NSZeroPoint];
    CGContextFlush(currentContext);
    const uint8_t *p = pixel(data, width, height, 30, 6);
    expect(p[0] == 255 && p[1] == 0 && p[2] == 0 && p[3] == 255, "background: red inside x 15-45 (%u,%u,%u,%u)", p[0],
          p[1], p[2], p[3]);
    p = pixel(data, width, height, 10, 6);
    const uint8_t *q = pixel(data, width, height, 50, 6), *t = pixel(data, width, height, 30, 18);
    expect(p[3] == 0 && q[3] == 0 && t[3] == 0, "background: nothing outside the characters' rects (%u %u %u)", p[3],
          q[3], t[3]);

    // A thick underline (1 point) at baseline offset 8 runs at y 7-8 from
    // x 5 to 55, in the underline colour.
    memset(data, 0, width * height * 4);
    NSRange lineRange;
    NSRect lineRect = [manager lineFragmentRectForGlyphAtIndex: 0 effectiveRange: &lineRange];
    [manager drawUnderlineForGlyphRange: NSMakeRange(0, 5)
                          underlineType: 0x02 /* NSUnderlineStyleThick */
                         baselineOffset: 8
                       lineFragmentRect: lineRect
                 lineFragmentGlyphRange: lineRange
                        containerOrigin: NSZeroPoint];
    CGContextFlush(currentContext);
    p = pixel(data, width, height, 30, 7);
    expect(p[2] > 128 && p[0] == 0 && p[3] > 128, "underline: blue at y 7 (%u,%u,%u,%u)", p[0], p[1], p[2], p[3]);
    p = pixel(data, width, height, 30, 10);
    q = pixel(data, width, height, 70, 7);
    expect(p[3] == 0 && q[3] == 0, "underline: nothing above it or after the text (%u %u)", p[3], q[3]);

    // The shadow and the attachment take what the provider gives them.
    id shadow = [[[classNamed("NSShadow") alloc] init] autorelease];
    expect([[shadow shadowColor] isKindOfClass: [TestColor class]], "provider: a new shadow has the provider's colour");
    [(id<TextSystemMethods>) shadow set];
    expect(shadowAskedForCGColor, "provider: -[NSShadow set] asks its colour for a CGColor");
    id attachment = [[[classNamed("NSTextAttachment") alloc] init] autorelease];
    id cell = [attachment attachmentCell];
    expect([cell isKindOfClass: [TestCell class]] && ((TestCell *) cell)->attachment == attachment,
          "provider: a new attachment has the provider's cell, pointing back to it");
    testImage = [[NSObject alloc] init];
    [attachment setContents: [@"data" dataUsingEncoding: NSUTF8StringEncoding]];
    expect([attachment imageForBounds: CGRectZero attributes: nil location: nil textContainer: nil] == testImage,
          "provider: an attachment's image comes from the provider");

    setProvider(nil);
    CGContextRelease(currentContext);
    currentContext = NULL;
    free(data);
}

int main(int argc, char **argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    BOOL withAppKit = argc > 1 && strcmp(argv[1], "--appkit") == 0;

    expect(!appKitLoaded(), "AppKit is not loaded at start");
    if (withAppKit)
        expect(dlopen(APPKIT_PATH, RTLD_LAZY) != NULL, "AppKit opens");
    expect(dlopen(UIFOUNDATION_PATH, RTLD_LAZY) != NULL, "UIFoundation opens");

    if (!withAppKit) {
        expect(!appKitLoaded(), "opening UIFoundation does not load AppKit");
        checkWithoutProvider();
    }
    checkLayout();
    if (!withAppKit) {
        checkWithProvider();
        expect(!appKitLoaded(), "AppKit is still not loaded at the end");
    }

    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
