/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/
// A file without an icon of its own shows the icon its type's handler application declares:
// UTTypeIconFile of the application's type declaration, or CFBundleTypeIconFile of its
// CFBundleDocumentTypes entry (matched by LSItemContentTypes or by the older extension list).
// The type's own declaration wins over a document type entry. Each icon has a different size,
// which tells the icons apart; the generic icons are 32x32.
#import <AppKit/AppKit.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void WriteIcon(NSString* path, NSInteger pixels)
{
	NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:pixels pixelsHigh:pixels
		bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace
		bytesPerRow:0 bitsPerPixel:32];
	memset([rep bitmapData], 0x80, [rep bytesPerRow] * pixels);
	[[rep representationUsingType:NSPNGFileType properties:@{}] writeToFile:path atomically:YES];
}

static NSString* MakeApp(NSString* root, NSString* name, NSString* bundleID, NSArray* docTypes, NSArray* exported,
	NSDictionary<NSString*, NSNumber*>* icons)
{
	NSString* app = [root stringByAppendingPathComponent:[name stringByAppendingString:@".app"]];
	NSString* macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
	NSString* resources = [app stringByAppendingPathComponent:@"Contents/Resources"];
	NSFileManager* fm = [NSFileManager defaultManager];
	[fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
	[fm createDirectoryAtPath:resources withIntermediateDirectories:YES attributes:nil error:NULL];
	[@"x" writeToFile:[macos stringByAppendingPathComponent:@"app"] atomically:YES encoding:NSUTF8StringEncoding error:NULL];
	for (NSString* file in icons)
		WriteIcon([resources stringByAppendingPathComponent:file], [icons[file] integerValue]);
	NSMutableDictionary* info = [@{
		@"CFBundleIdentifier": bundleID,
		@"CFBundleExecutable": @"app",
		@"CFBundlePackageType": @"APPL",
	} mutableCopy];
	if (docTypes) info[@"CFBundleDocumentTypes"] = docTypes;
	if (exported) info[@"UTExportedTypeDeclarations"] = exported;
	[info writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];
	return app;
}

static NSDictionary* TypeDecl(NSString* uti, NSString* ext, NSString* icon)
{
	NSMutableDictionary* d = [@{
		@"UTTypeIdentifier": uti,
		@"UTTypeConformsTo": @[ @"public.data" ],
		@"UTTypeTagSpecification": @{ @"public.filename-extension": @[ ext ] },
	} mutableCopy];
	if (icon) d[@"UTTypeIconFile"] = icon;
	return d;
}

static NSString* FileFor(NSString* root, NSString* ext)
{
	NSString* path = [root stringByAppendingPathComponent:[@"doc." stringByAppendingString:ext]];
	[@"x" writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:NULL];
	return path;
}

// The pixel width of the largest representation tells the icons apart: an icon starts out at 32x32 points
// whatever its representations hold.
static CGFloat WidthOf(NSImage* image)
{
	if (!image)
		return -1;
	CGFloat width = 0;
	for (NSImageRep* rep in image.representations)
		if (rep.pixelsWide > width)
			width = rep.pixelsWide;
	return width > 0 ? width : image.size.width;
}

int main(void)
{
	@autoreleasepool {
		NSString* tag = [NSString stringWithFormat:@"%d", (int) getpid()];
		NSString* root = [NSHomeDirectory() stringByAppendingPathComponent:[@"docicons-" stringByAppendingString:tag]];
		NSWorkspace* ws = [NSWorkspace sharedWorkspace];

		NSString* extLegacy = [@"dicl" stringByAppendingString:tag];
		NSString* extDoc = [@"dicd" stringByAppendingString:tag];
		NSString* extUTI = [@"dicu" stringByAppendingString:tag];
		NSString* extNone = [@"dicn" stringByAppendingString:tag];
		NSString* utiDoc = [@"org.darlinghq.test.docicons.d" stringByAppendingString:tag];
		NSString* utiBoth = [@"org.darlinghq.test.docicons.u" stringByAppendingString:tag];
		NSString* utiNone = [@"org.darlinghq.test.docicons.n" stringByAppendingString:tag];

		NSArray* apps = @[
			// The older extension list.
			MakeApp(root, @"Legacy", [@"org.darlinghq.test.docicons.legacy" stringByAppendingString:tag], @[ @{
				@"CFBundleTypeRole": @"Viewer",
				@"CFBundleTypeExtensions": @[ extLegacy ],
				@"CFBundleTypeIconFile": @"legacy.png",
			} ], nil, @{ @"legacy.png": @48 }),
			// A declared type whose handler entry carries the icon.
			MakeApp(root, @"Typed", [@"org.darlinghq.test.docicons.typed" stringByAppendingString:tag], @[ @{
				@"CFBundleTypeRole": @"Viewer",
				@"LSItemContentTypes": @[ utiDoc ],
				@"CFBundleTypeIconFile": @"doc.png",
			} ], @[ TypeDecl(utiDoc, extDoc, nil) ], @{ @"doc.png": @40 }),
			// The type's own icon and the handler entry's icon both exist.
			MakeApp(root, @"Both", [@"org.darlinghq.test.docicons.both" stringByAppendingString:tag], @[ @{
				@"CFBundleTypeRole": @"Viewer",
				@"LSItemContentTypes": @[ utiBoth ],
				@"CFBundleTypeIconFile": @"doc.png",
			} ], @[ TypeDecl(utiBoth, extUTI, @"type.png") ], @{ @"doc.png": @40, @"type.png": @56 }),
			// A handler that declares no icon at all.
			MakeApp(root, @"Plain", [@"org.darlinghq.test.docicons.plain" stringByAppendingString:tag], @[ @{
				@"CFBundleTypeRole": @"Viewer",
				@"LSItemContentTypes": @[ utiNone ],
			} ], @[ TypeDecl(utiNone, extNone, nil) ], nil),
		];
		for (NSString* app in apps)
			CHECK(LSRegisterURL((__bridge CFURLRef) [NSURL fileURLWithPath:app], true) == noErr);

		CHECK(NSEqualSizes([ws iconForFile:FileFor(root, extLegacy)].size, NSMakeSize(32, 32)));
		CHECK(WidthOf([ws iconForFile:FileFor(root, extLegacy)]) == 48);
		CHECK(WidthOf([ws iconForFileType:extLegacy]) == 48);

		CHECK(WidthOf([ws iconForFile:FileFor(root, extDoc)]) == 40);
		CHECK(WidthOf([ws iconForFileType:extDoc]) == 40);
		CHECK(WidthOf([ws iconForFileType:utiDoc]) == 40);

		CHECK(WidthOf([ws iconForFile:FileFor(root, extUTI)]) == 56);
		CHECK(WidthOf([ws iconForFileType:utiBoth]) == 56);

		// No declared icon: the generic icon.
		CHECK(WidthOf([ws iconForFile:FileFor(root, extNone)]) == 32);
		CHECK(WidthOf([ws iconForFileType:utiNone]) == 32);
		// An unregistered type is still a generic document.
		CHECK(WidthOf([ws iconForFileType:[@"dicx" stringByAppendingString:tag]]) == 32);

		[[NSFileManager defaultManager] removeItemAtPath:root error:NULL];
	}
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	return g_failures != 0;
}
