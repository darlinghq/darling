// CFLAGS: -framework Foundation -framework AppKit -fobjc-arc -Isrc/Finder src/Finder/FinderDSStore.m src/Finder/FinderFolderStyle.m
// ARGS: tests/fixtures/dsstore
// Copyright (C) 2026 Lubos Dolezel
//
// A folder's view settings from its .DS_Store (src/Finder/FinderFolderStyle): the window size and chrome, the view,
// icon and text sizes, the label position, the background colour or picture (resolved inside the folder's volume
// only), the items' positions, older records, values of the wrong type, and folders without a usable store. Each case
// copies a fixture from tests/fixtures/dsstore/ into a fresh folder of a temporary "volume". An optional store Finder
// wrote for a styled folder (see tests/finder.md) is checked too. See tests/finder.md.
// Usage: finder_folder_style <fixture dir> [<Finder-made store>]. Prints PASS/FAIL lines; exits 1 on a failure.
#import <AppKit/AppKit.h>
#import "FinderFolderStyle.h"
#include <math.h>
#include <stdio.h>

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSString *fixtureDir;

// A new "volume": a directory holding .background/background.png, a 2x2 picture
static NSString *makeVolume(void)
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *volume = [NSTemporaryDirectory() stringByAppendingPathComponent: [[NSUUID UUID] UUIDString]];
    [fm createDirectoryAtPath: [volume stringByAppendingPathComponent: @".background"] withIntermediateDirectories: YES
                   attributes: nil error: NULL];
    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL pixelsWide: 2 pixelsHigh: 2
                                                                 bitsPerSample: 8 samplesPerPixel: 3 hasAlpha: NO
                                                                      isPlanar: NO colorSpaceName: NSDeviceRGBColorSpace
                                                                   bytesPerRow: 0 bitsPerPixel: 0];
    [[rep representationUsingType: NSPNGFileType properties: @{}]
        writeToFile: [volume stringByAppendingPathComponent: @".background/background.png"] atomically: NO];
    return volume;
}

// The style of a new folder in volume whose .DS_Store is the fixture (or the file at a path)
static FinderFolderStyle *styleFor(NSString *volume, NSString *fixture)
{
    NSString *folder = [volume stringByAppendingPathComponent: [[NSUUID UUID] UUIDString]];
    [[NSFileManager defaultManager] createDirectoryAtPath: folder withIntermediateDirectories: YES attributes: nil error: NULL];
    if (fixture != nil) {
        NSString *source = [fixture isAbsolutePath] || [fixture containsString: @"/"]
            ? fixture : [fixtureDir stringByAppendingPathComponent: [fixture stringByAppendingString: @".DS_Store"]];
        [[NSFileManager defaultManager] copyItemAtPath: source toPath: [folder stringByAppendingPathComponent: @".DS_Store"] error: NULL];
    }
    return [FinderFolderStyle styleForFolder: [NSURL fileURLWithPath: folder] volume: [NSURL fileURLWithPath: volume]];
}

static NSString *resolved(NSURL *url)
{
    return [[url URLByResolvingSymlinksInPath] path];
}

static BOOL centerIs(FinderFolderStyle *style, NSString *name, CGFloat x, CGFloat y)
{
    NSPoint center = NSZeroPoint;
    return [style getCenter: &center forItemNamed: name] && center.x == x && center.y == y;
}

static BOOL near(CGFloat a, CGFloat b)
{
    return fabs(a - b) <= 1.0 / 255;
}

static BOOL colourIs(NSColor *colour, CGFloat r, CGFloat g, CGFloat b)
{
    NSColor *rgb = [colour colorUsingColorSpaceName: NSCalibratedRGBColorSpace];
    return rgb != nil && near([rgb redComponent], r) && near([rgb greenComponent], g) && near([rgb blueComponent], b);
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        setvbuf(stdout, NULL, _IONBF, 0);
        if (argc < 2) {
            fprintf(stderr, "usage: %s <fixture dir> [<Finder-made store>]\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        NSFileManager *fm = [NSFileManager defaultManager];
        fixtureDir = [NSString stringWithUTF8String: argv[1]];
        NSString *volume = makeVolume();
        NSString *picture = resolved([NSURL fileURLWithPath: [volume stringByAppendingPathComponent: @".background/background.png"]]);

        FinderFolderStyle *style = styleFor(volume, @"styled");
        expect(style != nil, @"styled has a style");
        expect(NSEqualSizes(style.windowContentSize, NSMakeSize(640, 400)),
               [NSString stringWithFormat: @"styled: content 640x400 (%@)", NSStringFromSize(style.windowContentSize)]);
        expect(!style.showsToolbar && !style.showsSidebar, @"styled: no toolbar, no sidebar");
        expect(style.viewStyle == FinderViewStyleIcons, @"styled: icons");
        expect(style.iconSize == 128 && style.textSize == 13,
               [NSString stringWithFormat: @"styled: icon 128, text 13 (%g, %g)", style.iconSize, style.textSize]);
        expect(style.labelOnBottom, @"styled: label at the bottom");
        expect([resolved(style.backgroundImageURL) isEqual: picture],
               [NSString stringWithFormat: @"styled: the picture is the volume's .background/background.png (%@)", style.backgroundImageURL]);
        expect(centerIs(style, @"Styled App.app", 160, 200) && centerIs(style, @"Applications", 480, 200),
               @"styled: centres (160, 200) and (480, 200)");
        expect(centerIs(style, [@"Ünï 😀.txt" precomposedStringWithCanonicalMapping], 320, 330),
               @"styled: a decomposed record is found by the precomposed name");
        NSPoint unset = NSMakePoint(-1, -1);
        expect(![style getCenter: &unset forItemNamed: @"Nope.txt"], @"styled: no centre for an item without a record");
        expect(![style getCenter: &unset forItemNamed: @"styled app.app"], @"styled: names match case-sensitively");

        style = styleFor(volume, @"styled-bookmark");
        expect([resolved(style.backgroundImageURL) isEqual: picture],
               [NSString stringWithFormat: @"styled-bookmark: the same picture (%@)", style.backgroundImageURL]);
        expect(style.iconSize == 128, @"styled-bookmark: icon 128");

        style = styleFor(volume, @"colour");
        expect(colourIs(style.backgroundColor, 0.2, 0.4, 0.6),
               [NSString stringWithFormat: @"colour: (0.2, 0.4, 0.6) (%@)", style.backgroundColor]);
        expect(style.backgroundImageURL == nil, @"colour: no picture");

        style = styleFor(volume, @"legacy");
        expect(style.iconSize == 72 && !style.labelOnBottom,
               [NSString stringWithFormat: @"legacy: icon 72, label on the right (%g, %d)", style.iconSize, style.labelOnBottom]);
        expect(colourIs(style.backgroundColor, 0.2, 0.4, 0.6),
               [NSString stringWithFormat: @"legacy: the colour (0.2, 0.4, 0.6) (%@)", style.backgroundColor]);

        style = styleFor(volume, @"list");
        expect(style != nil && style.viewStyle == FinderViewStyleList, @"list: the list view");

        // Pictures that aren't a regular file inside the folder's own volume
        [fm createSymbolicLinkAtPath: [volume stringByAppendingPathComponent: @".background/link.png"]
                 withDestinationPath: @"/etc/passwd" error: NULL];
        for (NSString *name in @[ @"escape-dotdot", @"escape-link" ]) {
            style = styleFor(volume, name);
            expect(style.backgroundImageURL == nil && style.iconSize == 128 && centerIs(style, @"Applications", 480, 200),
                   [NSString stringWithFormat: @"%@: no picture (%@), the rest of the style intact", name, style.backgroundImageURL]);
        }
        NSString *bare = makeVolume();
        [fm removeItemAtPath: [bare stringByAppendingPathComponent: @".background/background.png"] error: NULL];
        style = styleFor(bare, @"styled");
        expect(style.backgroundImageURL == nil && style.iconSize == 128, @"a missing picture: none, the rest intact");
        [fm createDirectoryAtPath: [bare stringByAppendingPathComponent: @".background/background.png"]
      withIntermediateDirectories: NO attributes: nil error: NULL];
        style = styleFor(bare, @"styled");
        expect(style.backgroundImageURL == nil && style.iconSize == 128, @"a picture that is a folder: none, the rest intact");

        style = styleFor(volume, @"wrong-types");
        expect(style != nil && NSEqualSizes(style.windowContentSize, NSZeroSize) && style.showsToolbar
                   && style.showsSidebar && style.iconSize == 64 && style.textSize == 12 && style.labelOnBottom
                   && ![style getCenter: &unset forItemNamed: @"Styled App.app"],
               [NSString stringWithFormat: @"wrong-types: a style with the defaults (%@, toolbar %d, icon %g)",
                NSStringFromSize(style.windowContentSize), style.showsToolbar, style.iconSize]);

        // Numbers that aren't finite keep the defaults
        style = styleFor(volume, @"icon-nan");
        expect(style.iconSize == 64 && style.textSize == 12 && style.backgroundColor == nil,
               [NSString stringWithFormat: @"icon-nan: icon 64, text 12, no colour (%g, %g, %@)", style.iconSize,
                style.textSize, style.backgroundColor]);
        // A size that doesn't scan as a finite number is none; a finite one, however large, is the window's to limit
        for (NSString *name in @[ @"bounds-inf", @"bounds-nan" ]) {
            NSSize size = styleFor(volume, name).windowContentSize;
            expect(isfinite(size.width) && isfinite(size.height),
                   [NSString stringWithFormat: @"%@: a finite window size or none (%@)", name, NSStringFromSize(size)]);
        }

        // A picture that is a file inside the volume is passed on whatever it holds; the colour comes with it
        [@"not a picture" writeToFile: [volume stringByAppendingPathComponent: @".background/notes.png"] atomically: NO
                             encoding: NSUTF8StringEncoding error: NULL];
        style = styleFor(volume, @"picture-not-image");
        expect([resolved(style.backgroundImageURL) isEqual: resolved([NSURL fileURLWithPath: [volume stringByAppendingPathComponent: @".background/notes.png"]])]
                   && colourIs(style.backgroundColor, 0.2, 0.4, 0.6),
               [NSString stringWithFormat: @"picture-not-image: the file and the colour (%@, %@)", style.backgroundImageURL,
                style.backgroundColor]);

        expect(styleFor(volume, nil) == nil, @"a folder without .DS_Store has no style");
        expect(styleFor(volume, @"empty") == nil, @"a folder with an empty .DS_Store has no style");
        expect(styleFor(volume, @"bad-magic") == nil, @"a folder with a malformed .DS_Store has no style");
        expect(styleFor(volume, @"no-settings") == nil, @"a .DS_Store with no view settings or positions gives no style");

        // The volume from NSURLVolumeURLKey
        NSString *folder = [volume stringByAppendingPathComponent: @"own volume"];
        [fm createDirectoryAtPath: folder withIntermediateDirectories: NO attributes: nil error: NULL];
        [fm copyItemAtPath: [fixtureDir stringByAppendingPathComponent: @"styled.DS_Store"]
                    toPath: [folder stringByAppendingPathComponent: @".DS_Store"] error: NULL];
        style = [FinderFolderStyle styleForFolder: [NSURL fileURLWithPath: folder]];
        expect(style.iconSize == 128, @"styleForFolder: finds the folder's volume itself");

        if (argc > 2) {
            style = styleFor(volume, [NSString stringWithUTF8String: argv[2]]);
            expect(style.iconSize == 96, [NSString stringWithFormat: @"Finder-made: icon 96 (%g)", style.iconSize]);
            expect(centerIs(style, @"One.txt", 100, 150) && centerIs(style, @"Two.txt", 400, 150),
                   @"Finder-made: centres (100, 150) and (400, 150)");
            expect(NSEqualSizes(style.windowContentSize, NSMakeSize(500, 300)),
                   [NSString stringWithFormat: @"Finder-made: content 500x300 (%@)", NSStringFromSize(style.windowContentSize)]);
            expect(!style.showsToolbar, @"Finder-made: no toolbar");
        }

        [fm removeItemAtPath: volume error: NULL];
        [fm removeItemAtPath: bare error: NULL];
        printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
    }
    return failures != 0;
}
