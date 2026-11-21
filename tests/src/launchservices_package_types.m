// Copyright (C) 2026 Lubos Dolezel
//
// LaunchServices: whether a directory is a package (kCFURLIsPackageKey) follows the types that
// registered applications declare. A directory is a package when its extension belongs to an
// exported or imported type that conforms to com.apple.package, or to a document type
// (CFBundleDocumentTypes) declared with LSTypeIsPackage; a document type without LSTypeIsPackage
// does not make one. The system's own imported declarations count too (.pages).
// Run with no arguments. It registers a throw-away application in ~/Applications, so run it only
// under Darling, not on a real Mac.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } else { printf("PASS %s\n", #cond); } } while (0)

// A fresh URL each time, so that no cached resource value is reused.
static int isPackage(NSString *path)
{
	NSURL *url = [NSURL fileURLWithPath:path isDirectory:YES];
	NSNumber *value = nil;
	if (![url getResourceValue:&value forKey:NSURLIsPackageKey error:NULL] || value == nil)
		return -1;
	return [value boolValue] ? 1 : 0;
}

int main(int argc, char **argv)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *apps = [NSHomeDirectory() stringByAppendingPathComponent:@"Applications"];
	[fm createDirectoryAtPath:apps withIntermediateDirectories:YES attributes:nil error:NULL];

	// Registrations of removed bundles stay in the database, so each run uses its own names.
	NSString *tag = [NSString stringWithFormat:@"%d", (int) getpid()];
	NSString *typeExtension = [@"darlingpkgtype" stringByAppendingString:tag];
	NSString *docExtension = [@"darlingpkgdoc" stringByAppendingString:tag];
	NSString *plainExtension = [@"darlingplaindoc" stringByAppendingString:tag];
	NSString *importedExtension = [@"darlingpkgimport" stringByAppendingString:tag];

	NSString *app = [apps stringByAppendingPathComponent:[NSString stringWithFormat:@"darlingpackagetypes%@.app", tag]];
	NSString *macos = [app stringByAppendingPathComponent:@"Contents/MacOS"];
	[fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL];
	[fm copyItemAtPath:[[NSBundle mainBundle] executablePath] toPath:[macos stringByAppendingPathComponent:@"x"] error:NULL];

	NSDictionary *exported = [NSDictionary dictionaryWithObjectsAndKeys:
		[@"org.darlinghq.test.packagetype" stringByAppendingString:tag], @"UTTypeIdentifier",
		[NSArray arrayWithObject:@"com.apple.package"], @"UTTypeConformsTo",
		[NSDictionary dictionaryWithObject:[NSArray arrayWithObject:typeExtension] forKey:@"public.filename-extension"], @"UTTypeTagSpecification",
		nil];
	NSDictionary *imported = [NSDictionary dictionaryWithObjectsAndKeys:
		[@"org.darlinghq.test.importedtype" stringByAppendingString:tag], @"UTTypeIdentifier",
		[NSArray arrayWithObject:@"com.apple.package"], @"UTTypeConformsTo",
		[NSDictionary dictionaryWithObject:[NSArray arrayWithObject:importedExtension] forKey:@"public.filename-extension"], @"UTTypeTagSpecification",
		nil];
	NSDictionary *packageDocument = [NSDictionary dictionaryWithObjectsAndKeys:
		@"Package document", @"CFBundleTypeName",
		@"Viewer", @"CFBundleTypeRole",
		[NSArray arrayWithObject:docExtension], @"CFBundleTypeExtensions",
		[NSNumber numberWithBool:YES], @"LSTypeIsPackage",
		nil];
	NSDictionary *plainDocument = [NSDictionary dictionaryWithObjectsAndKeys:
		@"Folder document", @"CFBundleTypeName",
		@"Viewer", @"CFBundleTypeRole",
		[NSArray arrayWithObject:plainExtension], @"CFBundleTypeExtensions",
		nil];
	NSDictionary *info = [NSDictionary dictionaryWithObjectsAndKeys:
		[@"org.darlinghq.test.packagetypes" stringByAppendingString:tag], @"CFBundleIdentifier",
		@"x", @"CFBundleExecutable",
		@"APPL", @"CFBundlePackageType",
		@"1", @"CFBundleVersion",
		@"x", @"CFBundleName",
		[NSArray arrayWithObject:exported], @"UTExportedTypeDeclarations",
		[NSArray arrayWithObject:imported], @"UTImportedTypeDeclarations",
		[NSArray arrayWithObjects:packageDocument, plainDocument, nil], @"CFBundleDocumentTypes",
		nil];
	[info writeToFile:[app stringByAppendingPathComponent:@"Contents/Info.plist"] atomically:YES];

	char dirTemplate[] = "/tmp/lspackagetypes.XXXXXX";
	NSString *dir = [NSString stringWithUTF8String:mkdtemp(dirTemplate)];
	NSString *typed = [dir stringByAppendingPathComponent:[@"a." stringByAppendingString:typeExtension]];
	NSString *document = [dir stringByAppendingPathComponent:[@"b." stringByAppendingString:docExtension]];
	NSString *plain = [dir stringByAppendingPathComponent:[@"c." stringByAppendingString:plainExtension]];
	NSString *application = [dir stringByAppendingPathComponent:@"d.app"];
	NSString *importedType = [dir stringByAppendingPathComponent:[@"e." stringByAppendingString:importedExtension]];
	NSString *pages = [dir stringByAppendingPathComponent:@"f.pages"];
	for (NSString *path in [NSArray arrayWithObjects:typed, document, plain, application, importedType, pages, nil])
		[fm createDirectoryAtPath:path withIntermediateDirectories:NO attributes:nil error:NULL];

	// No application declares these extensions yet.
	CHECK(isPackage(typed) == 0);
	CHECK(isPackage(document) == 0);
	CHECK(isPackage(plain) == 0);
	CHECK(isPackage(application) == 1);
	CHECK(isPackage(importedType) == 0);
	CHECK(isPackage(pages) == 1);

	OSStatus status = LSRegisterURL((CFURLRef) [NSURL fileURLWithPath:app], true);
	CHECK(status == noErr);

	CHECK(isPackage(typed) == 1);
	CHECK(isPackage(document) == 1);
	CHECK(isPackage(plain) == 0);
	CHECK(isPackage(application) == 1);
	CHECK(isPackage(importedType) == 1);

	[fm removeItemAtPath:dir error:NULL];
	[fm removeItemAtPath:app error:NULL];
	printf(g_failures ? "FAILED (%d)\n" : "PASSED\n", g_failures);
	[pool drain];
	return g_failures != 0;
}
