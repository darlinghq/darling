// CFLAGS: -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// Where the text system classes live. On macOS the classes and constants
// listed below are defined by the private UIFoundation framework, and AppKit
// re-exports UIFoundation, so a program that links AppKit reaches them
// through AppKit (Mac OS X 10.14.4 and macOS 26.7.3 symbol tables of both
// frameworks, i386 and x86_64). This checks, for each listed class:
//   - the class is defined by the UIFoundation image (class_getImageName and
//     dladdr), not by AppKit;
//   - objc_getClass, NSClassFromString and, on x86_64, the OBJC_CLASS_$
//     symbol looked up through the AppKit handle and through the UIFoundation
//     handle all give the same class;
// for each listed constant: the symbol found through AppKit and through
// UIFoundation is the same variable, defined in UIFoundation, with the
// documented value; and for the images as a whole: AppKit re-exports
// UIFoundation (and not unrelated libraries), and no class is defined by both
// AppKit and UIFoundation (the runtime would then pick one of the two).
//
// Instances AppKit's text view creates are the UIFoundation classes, and a
// paragraph style survives a keyed archive round trip and a copy.
//
// Loads AppKit with dlopen; needs an X display (AppKit's text view looks up
// fonts). Prints PASS or FAIL lines, then
// ALL PASSED or FAILED n.
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <objc/runtime.h>
#include <stdio.h>
#include <string.h>

#define APPKIT_PATH "/System/Library/Frameworks/AppKit.framework/AppKit"
#define UIFOUNDATION_PATH "/System/Library/PrivateFrameworks/UIFoundation.framework/UIFoundation"

// Classes UIFoundation defines (and AppKit does not) on macOS.
static const char *const uifoundationClasses[] = {
    "UINibDecoder",
    "NSParagraphStyle",
    "NSMutableParagraphStyle",
    "NSTextTab",
    "NSTextList",
    "NSTextBlock",
    "NSTextTable",
    "NSTextTableBlock",
    "NSGlyphInfo",
};

// String constants UIFoundation defines (and AppKit does not) on macOS, with
// their values.
static const struct {
    const char *symbol;
    NSString *value;
} uifoundationStrings[] = {
    {"NSTabColumnTerminatorsAttributeName", @"NSTabColumnTerminatorsAttributeName"},
    {"NSTextListMarkerDecimal", @"{decimal}"},
    {"NSTextListMarkerDisc", @"{disc}"},
    {"NSTextListMarkerHyphen", @"{hyphen}"},
    {NULL, nil},
};

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

static BOOL hasSuffix(const char *s, const char *suffix)
{
    size_t a = s ? strlen(s) : 0, b = strlen(suffix);
    return s != NULL && a >= b && strcmp(s + a - b, suffix) == 0;
}

static BOOL isUIFoundation(const char *path)
{
    return hasSuffix(path, "/UIFoundation");
}

static const char *imageOf(const void *address)
{
    Dl_info info;
    if (address == NULL || dladdr(address, &info) == 0)
        return NULL;
    return info.dli_fname;
}

static void checkClass(void *appkit, void *uif, const char *name)
{
    Class cls = objc_getClass(name);
    expect(cls != Nil, "%s exists", name);
    if (cls == Nil)
        return;
    expect(NSClassFromString([NSString stringWithUTF8String: name]) == cls, "%s: NSClassFromString agrees", name);
    expect(isUIFoundation(class_getImageName(cls)), "%s: defined by UIFoundation (class_getImageName %s)", name,
          class_getImageName(cls));
    expect(isUIFoundation(imageOf(cls)), "%s: class object lies in UIFoundation (dladdr %s)", name, imageOf(cls));
#if __OBJC2__
    char symbol[256];
    snprintf(symbol, sizeof(symbol), "OBJC_CLASS_$_%s", name);
    void *viaAppKit = dlsym(appkit, symbol);
    void *viaUIFoundation = dlsym(uif, symbol);
    expect(viaAppKit == (void *) cls && viaUIFoundation == (void *) cls,
          "%s: OBJC_CLASS_$ symbol through AppKit (%p) and UIFoundation (%p) is the class (%p)", name, viaAppKit,
          viaUIFoundation, (void *) cls);
#endif
}

static void checkString(void *appkit, void *uif, const char *symbol, NSString *value)
{
    NSString **viaAppKit = dlsym(appkit, symbol);
    NSString **viaUIFoundation = dlsym(uif, symbol);
    expect(viaAppKit != NULL && viaAppKit == viaUIFoundation, "%s: same variable through AppKit and UIFoundation",
          symbol);
    expect(isUIFoundation(imageOf(viaUIFoundation)), "%s: defined by UIFoundation (%s)", symbol,
          imageOf(viaUIFoundation));
    expect(viaUIFoundation != NULL && [*viaUIFoundation isEqualToString: value], "%s: value %s", symbol,
          viaUIFoundation ? [*viaUIFoundation UTF8String] : "(missing)");
}

static void checkNoClassInBoth(const char *appkitImage, const char *uifImage)
{
    unsigned int appkitCount = 0, uifCount = 0;
    const char **appkitNames = objc_copyClassNamesForImage(appkitImage, &appkitCount);
    const char **uifNames = objc_copyClassNamesForImage(uifImage, &uifCount);
    NSMutableSet *appkitSet = [NSMutableSet set];
    for (unsigned int i = 0; i < appkitCount; i++)
        [appkitSet addObject: [NSString stringWithUTF8String: appkitNames[i]]];
    NSMutableArray *both = [NSMutableArray array];
    for (unsigned int i = 0; i < uifCount; i++) {
        NSString *name = [NSString stringWithUTF8String: uifNames[i]];
        if ([appkitSet containsObject: name])
            [both addObject: name];
    }
    free(appkitNames);
    free(uifNames);
    expect(appkitCount > 0 && uifCount > 0, "class lists of both images (%u AppKit, %u UIFoundation)", appkitCount,
          uifCount);
    expect([both count] == 0, "no class defined by both AppKit and UIFoundation%s%s", [both count] ? ": " : "",
          [both count] ? [[both componentsJoinedByString: @", "] UTF8String] : "");
}

// The AppKit headers are not used; only what is called is declared.
@protocol TextValueMethods
+ (id)defaultParagraphStyle;
- (id)initWithFrame:(NSRect)frame;
- (void)setString:(NSString *)string;
- (void)setAlignment:(NSInteger)alignment;
- (NSInteger)alignment;
- (id)textStorage;
- (NSArray *)tabStops;
- (NSArray *)textBlocks;
- (NSArray *)textLists;
- (void)setTextBlocks:(NSArray *)blocks;
- (void)setTextLists:(NSArray *)lists;
- (id)initWithTable:(id)table
        startingRow:(NSInteger)row
            rowSpan:(NSInteger)rowSpan
     startingColumn:(NSInteger)column
         columnSpan:(NSInteger)columnSpan;
- (id)initWithMarkerFormat:(NSString *)format options:(NSUInteger)options;
@end

static void checkAppKitInstances(void)
{
    // AppKit's text view makes the paragraph style when the alignment is set
    // (2 is NSTextAlignmentCenter on macOS on Intel).
    id textView = [[[objc_getClass("NSTextView") alloc] initWithFrame: NSMakeRect(0, 0, 200, 100)] autorelease];
    [textView setString: @"hello"];
    [textView setAlignment: 2];
    id style = [[textView textStorage] attribute: @"NSParagraphStyle" atIndex: 0 effectiveRange: NULL];
    expect(style != nil && isUIFoundation(class_getImageName([style class])) &&
                  [style isKindOfClass: objc_getClass("NSParagraphStyle")] && [style alignment] == 2,
          "paragraph style NSTextView sets is a UIFoundation class (%s)", style ? class_getName([style class]) : "nil");
    NSArray *tabs = [style tabStops];
    expect([tabs count] > 0 && isUIFoundation(class_getImageName([[tabs objectAtIndex: 0] class])),
          "its tab stops are UIFoundation's NSTextTab");

    // A mutable copy survives a keyed archive. (Text blocks and lists are not
    // archived: Darling's NSTextBlock and NSTextList do not implement
    // NSCoding yet.)
    id mutableStyle = [[style mutableCopy] autorelease];
    expect(isUIFoundation(class_getImageName([mutableStyle class])), "mutable copy is a UIFoundation class (%s)",
          class_getName([mutableStyle class]));
    NSData *archive = [NSKeyedArchiver archivedDataWithRootObject: mutableStyle];
    id decoded = [NSKeyedUnarchiver unarchiveObjectWithData: archive];
    expect(decoded != nil && isUIFoundation(class_getImageName([decoded class])) && [decoded isEqual: mutableStyle] &&
                  [[decoded tabStops] isEqual: [mutableStyle tabStops]],
          "paragraph style survives a keyed archive round trip (%s)", decoded ? class_getName([decoded class]) : "nil");

    // The other value classes, made directly, and stored in a paragraph style.
    id table = [[[objc_getClass("NSTextTable") alloc] init] autorelease];
    id block = [[[objc_getClass("NSTextTableBlock") alloc] initWithTable: table
                                                             startingRow: 0
                                                                 rowSpan: 1
                                                          startingColumn: 0
                                                              columnSpan: 1] autorelease];
    id list = [[[objc_getClass("NSTextList") alloc] initWithMarkerFormat: @"{decimal}" options: 0] autorelease];
    [mutableStyle setTextBlocks: [NSArray arrayWithObject: block]];
    [mutableStyle setTextLists: [NSArray arrayWithObject: list]];
    id copied = [[mutableStyle copy] autorelease];
    expect(isUIFoundation(class_getImageName([copied class])) && [copied isEqual: mutableStyle] &&
                  [[copied textBlocks] objectAtIndex: 0] == block && [[copied textLists] objectAtIndex: 0] == list,
          "an immutable copy keeps the text block and list (%s)", class_getName([copied class]));
    id defaultStyle = [objc_getClass("NSParagraphStyle") defaultParagraphStyle];
    expect(defaultStyle == [objc_getClass("NSParagraphStyle") defaultParagraphStyle] &&
                  isUIFoundation(class_getImageName([defaultStyle class])),
          "defaultParagraphStyle is one UIFoundation instance");
}

int main(void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    void *appkit = dlopen(APPKIT_PATH, RTLD_LAZY | RTLD_FIRST);
    void *uif = dlopen(UIFOUNDATION_PATH, RTLD_LAZY | RTLD_FIRST);
    expect(appkit != NULL && uif != NULL, "AppKit and UIFoundation open");
    if (appkit == NULL || uif == NULL)
        return 1;

    // AppKit re-exports UIFoundation: a UIFoundation function is found
    // through the AppKit handle alone, while a library AppKit merely links
    // (QuartzCore) is not.
    void *viaAppKit = dlsym(appkit, "UIDataLooksLikeNibArchive");
    expect(viaAppKit != NULL && viaAppKit == dlsym(uif, "UIDataLooksLikeNibArchive"),
          "AppKit re-exports UIFoundation");
    expect(dlsym(appkit, "kCAFillModeForwards") == NULL, "AppKit does not re-export QuartzCore (control)");

    const char *appkitImage = class_getImageName(objc_getClass("NSApplication"));
    const char *uifImage = class_getImageName(objc_getClass("UINibDecoder"));
    expect(hasSuffix(appkitImage, "/AppKit") && isUIFoundation(uifImage), "image paths (%s, %s)", appkitImage,
          uifImage);

    for (size_t i = 0; i < sizeof(uifoundationClasses) / sizeof(uifoundationClasses[0]); i++)
        checkClass(appkit, uif, uifoundationClasses[i]);
    for (size_t i = 0; uifoundationStrings[i].symbol != NULL; i++)
        checkString(appkit, uif, uifoundationStrings[i].symbol, uifoundationStrings[i].value);
    if (appkitImage && uifImage)
        checkNoClassInBoth(appkitImage, uifImage);
    checkAppKitInstances();

    [pool release];
    if (failures)
        printf("FAILED %d\n", failures);
    else
        printf("ALL PASSED\n");
    return failures != 0;
}
