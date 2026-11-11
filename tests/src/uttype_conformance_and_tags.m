// Copyright (C) 2026 Lubos Dolezel
//
// UTType (UniformTypeIdentifiers) agrees with the UTType C functions of
// LaunchServices: identifier and tag lookups, conformance (transitive and
// reflexive), dynamic types for unknown tags, supertypes, runtime-declared
// types and NSSecureCoding. When the process may write to /Applications, a
// bundle that exports a type is registered and its type is looked up once
// launchservicesd has picked the bundle up.
//
// Everything except the bundle part also holds on macOS 27.0.1, where the
// program passes unchanged; the bundle part is skipped there because
// /Applications is not writable.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <dlfcn.h>
#include <objc/message.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// UniformTypeIdentifiers is loaded at run time and used through id, so the
// program links without the framework's import library.
static id UT(NSString *name)
{
	void *p = dlsym(RTLD_DEFAULT, [name UTF8String]);
	return p ? *(id *) p : nil;
}

static id (*sendId)(id, SEL, id) = (id (*)(id, SEL, id)) objc_msgSend;
static BOOL (*sendBool)(id, SEL, id) = (BOOL (*)(id, SEL, id)) objc_msgSend;
static id (*sendId2)(id, SEL, id, id) = (id (*)(id, SEL, id, id)) objc_msgSend;
static id (*sendTag)(id, SEL, id, id, id) = (id (*)(id, SEL, id, id, id)) objc_msgSend;

static id typeWithIdentifier(NSString *identifier) { return sendId(NSClassFromString(@"UTType"), @selector(typeWithIdentifier:), identifier); }
static id typeWithExtension(NSString *ext) { return sendId(NSClassFromString(@"UTType"), @selector(typeWithFilenameExtension:), ext); }
static id typeWithMIME(NSString *mime) { return sendId(NSClassFromString(@"UTType"), @selector(typeWithMIMEType:), mime); }

static void testBundleDeclaredType(void)
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *app = @"/Applications/UTTypeConformanceTest.app";
	NSString *contents = [app stringByAppendingPathComponent: @"Contents"];
	[fm removeItemAtPath: app error: nil];
	if (![fm createDirectoryAtPath: contents withIntermediateDirectories: YES attributes: nil error: nil]) {
		printf("SKIP: cannot create %s\n", [app UTF8String]);
		return;
	}
	NSDictionary *plist = @{
		@"CFBundleIdentifier": @"org.darlinghq.test.uttypeconformance",
		@"CFBundleName": @"UTTypeConformanceTest",
		@"CFBundlePackageType": @"APPL",
		@"UTExportedTypeDeclarations": @[ @{
			@"UTTypeIdentifier": @"org.darlinghq.test.uttypeconformance.doc",
			@"UTTypeDescription": @"UTType test document",
			@"UTTypeConformsTo": @[ @"public.plain-text" ],
			@"UTTypeTagSpecification": @{ @"public.filename-extension": @[ @"uttcdoc", @"uttcdocx" ] },
		} ],
	};
	CHECK([plist writeToFile: [contents stringByAppendingPathComponent: @"Info.plist"] atomically: YES]);

	id found = nil;
	for (int i = 0; i < 40 && !found; i++) {
		found = typeWithIdentifier(@"org.darlinghq.test.uttypeconformance.doc");
		if (!found)
			[NSThread sleepForTimeInterval: 0.5];
	}
	CHECK(found != nil);
	if (found) {
		CHECK([[found valueForKey: @"declared"] boolValue]);
		CHECK(sendBool(found, @selector(conformsToType:), UT(@"UTTypePlainText")));
		CHECK([[found valueForKey: @"preferredFilenameExtension"] isEqual: @"uttcdoc"]);
		CHECK([[found valueForKey: @"localizedDescription"] isEqual: @"UTType test document"]);
		CHECK(typeWithExtension(@"uttcdocx") != nil && [typeWithExtension(@"uttcdocx") isEqual: found]);
		CHECK(UTTypeConformsTo(CFSTR("org.darlinghq.test.uttypeconformance.doc"), CFSTR("public.item")));
	}
	[fm removeItemAtPath: app error: nil];
}

int main(void)
{
	@autoreleasepool {
		if (!dlopen("/System/Library/Frameworks/UniformTypeIdentifiers.framework/UniformTypeIdentifiers", RTLD_NOW)) {
			printf("FAIL: dlopen: %s\n", dlerror());
			return 1;
		}
		id plainText = UT(@"UTTypePlainText"), text = UT(@"UTTypeText"), item = UT(@"UTTypeItem"), data = UT(@"UTTypeData");
		id image = UT(@"UTTypeImage"), jpeg = UT(@"UTTypeJPEG"), png = UT(@"UTTypePNG"), json = UT(@"UTTypeJSON");
		CHECK(plainText && text && item && data && image && jpeg && png && json);

		// Constants and identifier lookup
		CHECK([[plainText valueForKey: @"identifier"] isEqual: @"public.plain-text"]);
		CHECK([[plainText valueForKey: @"declared"] boolValue]);
		CHECK(![[plainText valueForKey: @"dynamic"] boolValue]);
		CHECK([[plainText valueForKey: @"publicType"] boolValue]);
		CHECK([typeWithIdentifier(@"public.plain-text") isEqual: plainText]);
		CHECK([typeWithIdentifier(@"PUBLIC.PLAIN-TEXT") isEqual: plainText]);
		CHECK(typeWithIdentifier(@"org.darlinghq.test.surely-undeclared") == nil);
		CHECK(typeWithIdentifier(@"") == nil);
		CHECK(typeWithIdentifier(@"has space") == nil);

		// Tag lookup
		CHECK([typeWithExtension(@"txt") isEqual: plainText]);
		CHECK([typeWithExtension(@"TXT") isEqual: plainText]);
		CHECK([typeWithExtension(@"json") isEqual: json]);
		CHECK([typeWithExtension(@"jpg") isEqual: jpeg]);
		CHECK([typeWithMIME(@"application/json") isEqual: json]);
		CHECK([typeWithMIME(@"image/png") isEqual: png]);
		CHECK([[plainText valueForKey: @"preferredFilenameExtension"] isEqual: @"txt"]);
		CHECK([[plainText valueForKey: @"preferredMIMEType"] isEqual: @"text/plain"]);
		CHECK([[[jpeg valueForKey: @"tags"] objectForKey: @"public.filename-extension"] containsObject: @"jpg"]);
		CHECK([[[jpeg valueForKey: @"tags"] objectForKey: @"public.mime-type"] containsObject: @"image/jpeg"]);
		id jpegViaImage = sendTag(NSClassFromString(@"UTType"), @selector(typeWithTag:tagClass:conformingToType:), @"jpg", @"public.filename-extension", image);
		CHECK([jpegViaImage isEqual: jpeg]);
		NSArray *all = sendTag(NSClassFromString(@"UTType"), @selector(typesWithTag:tagClass:conformingToType:), @"jpg", @"public.filename-extension", nil);
		CHECK([all count] >= 1 && [[all objectAtIndex: 0] isEqual: jpeg]);

		// Conformance, in both directions and through both APIs
		CHECK(sendBool(plainText, @selector(conformsToType:), plainText));
		CHECK(sendBool(plainText, @selector(conformsToType:), text));
		CHECK(sendBool(plainText, @selector(conformsToType:), data));
		CHECK(sendBool(plainText, @selector(conformsToType:), item));
		CHECK(!sendBool(text, @selector(conformsToType:), plainText));
		CHECK(!sendBool(plainText, @selector(conformsToType:), image));
		CHECK(sendBool(jpeg, @selector(conformsToType:), item));
		CHECK(sendBool(item, @selector(isSupertypeOfType:), jpeg));
		CHECK(sendBool(jpeg, @selector(isSubtypeOfType:), image));
		CHECK(!sendBool(jpeg, @selector(isSubtypeOfType:), jpeg));
		CHECK(UTTypeConformsTo(CFSTR("public.jpeg"), CFSTR("public.item")) == YES);
		CHECK(UTTypeConformsTo(CFSTR("public.jpeg"), CFSTR("public.jpeg")) == YES);
		CHECK(UTTypeConformsTo(CFSTR("PUBLIC.JPEG"), CFSTR("public.image")) == YES);
		CHECK(UTTypeConformsTo(CFSTR("public.image"), CFSTR("public.jpeg")) == NO);
		CHECK(UTTypeConformsTo(CFSTR("org.darlinghq.test.surely-undeclared"), CFSTR("org.darlinghq.test.surely-undeclared")) == YES);
		CHECK(UTTypeConformsTo(CFSTR("org.darlinghq.test.surely-undeclared"), CFSTR("public.item")) == NO);

		// Supertypes: transitive, without the type itself
		NSSet *supers = [png valueForKey: @"supertypes"];
		CHECK([supers containsObject: image]);
		CHECK([supers containsObject: item]);
		CHECK(![supers containsObject: png]);

		// Unknown tags give dynamic types
		id dyn = typeWithExtension(@"uttestnosuchext");
		CHECK(dyn != nil);
		CHECK([[dyn valueForKey: @"dynamic"] boolValue]);
		CHECK(![[dyn valueForKey: @"declared"] boolValue]);
		CHECK([dyn valueForKey: @"localizedDescription"] == nil);
		CHECK([[[dyn valueForKey: @"tags"] objectForKey: @"public.filename-extension"] containsObject: @"uttestnosuchext"]);
		CHECK(UTTypeIsDynamic((CFStringRef) [dyn valueForKey: @"identifier"]));
		CHECK(typeWithIdentifier([dyn valueForKey: @"identifier"]) != nil);
		id dynText = sendTag(NSClassFromString(@"UTType"), @selector(typeWithTag:tagClass:conformingToType:), @"uttestnosuchext", @"public.filename-extension", text);
		CHECK(dynText != nil && sendBool(dynText, @selector(conformsToType:), text));
		CHECK(!sendBool(dyn, @selector(conformsToType:), text));

		// quirk_exported_undeclared_type: a runtime-declared type that no bundle declares reports itself as
		// declared, and conforms to nothing but itself even when a supertype is requested.
		id undeclared = sendId2(NSClassFromString(@"UTType"), @selector(exportedTypeWithIdentifier:conformingToType:), @"org.darlinghq.test.surely-undeclared", plainText);
		CHECK(undeclared != nil);
		CHECK([[undeclared valueForKey: @"declared"] boolValue]);
		CHECK(!sendBool(undeclared, @selector(conformsToType:), text));
		CHECK(!sendBool(undeclared, @selector(conformsToType:), item));
		CHECK(sendBool(undeclared, @selector(conformsToType:), undeclared));
		CHECK([[undeclared valueForKey: @"supertypes"] count] == 0);
		CHECK([undeclared valueForKey: @"localizedDescription"] == nil);

		// NSSecureCoding
		NSData *archived = [NSKeyedArchiver archivedDataWithRootObject: jpeg requiringSecureCoding: YES error: nil];
		id decoded = [NSKeyedUnarchiver unarchivedObjectOfClass: NSClassFromString(@"UTType") fromData: archived error: nil];
		CHECK([decoded isEqual: jpeg]);
		CHECK([[jpeg copy] isEqual: jpeg]);

		testBundleDeclaredType();
	}

	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
