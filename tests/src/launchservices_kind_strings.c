// CFLAGS: -framework CoreFoundation -framework CoreServices
// Copyright (C) 2026 Lubos Dolezel
//
// The deprecated LaunchServices item functions on FSRefs and type information: kind strings,
// display name, item info, application lookup, acceptance, registration, item attributes and
// handler options. The expected values and error codes are what macOS (Mojave and 26) reports.
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)
#define CHECK_ST(expr, want) do { OSStatus s_ = (expr); if (s_ != (want)) { printf("FAIL: %s = %d, want %d (line %d)\n", #expr, (int) s_, (int) (want), __LINE__); failures++; } } while (0)

static int equalsString(CFStringRef value, const char *expected)
{
	CFStringRef e = CFStringCreateWithCString(NULL, expected, kCFStringEncodingUTF8);
	int r = value && CFEqual(value, e);
	CFRelease(e);
	return r;
}

static void makeFile(const char *dir, const char *name)
{
	char path[1024];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	FILE *f = fopen(path, "w");
	if (f)
		fclose(f);
}

static void makeDir(const char *dir, const char *name)
{
	char path[1024];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	mkdir(path, 0755);
}

static int refFor(const char *dir, const char *name, FSRef *ref)
{
	char path[1024];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	return FSPathMakeRef((const UInt8 *) path, ref, NULL) == noErr;
}

// The kind string of an item through both the FSRef and the URL function; they have to agree.
static CFStringRef kindOf(const char *dir, const char *name)
{
	FSRef ref;
	CFStringRef viaRef = NULL, viaURL = NULL;
	char path[1024];

	snprintf(path, sizeof(path), "%s/%s", dir, name);
	CHECK(refFor(dir, name, &ref));
	CHECK_ST(LSCopyKindStringForRef(&ref, &viaRef), noErr);

	CFStringRef p = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, p, kCFURLPOSIXPathStyle, false);
	CHECK_ST(LSCopyKindStringForURL(url, &viaURL), noErr);
	CHECK(viaRef && viaURL && CFEqual(viaRef, viaURL));
	CFRelease(url);
	CFRelease(p);
	if (viaURL)
		CFRelease(viaURL);
	return viaRef;
}

static CFStringRef kindForExtension(const char *ext)
{
	CFStringRef e = CFStringCreateWithCString(NULL, ext, kCFStringEncodingUTF8);
	CFStringRef kind = NULL;
	CHECK_ST(LSCopyKindStringForTypeInfo(0, 0, e, &kind), noErr);
	CFRelease(e);
	return kind;
}

int main(void)
{
	char dir[] = "/tmp/lskindXXXXXX";
	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	makeDir(dir, "folder");
	makeDir(dir, "Sample.app");
	makeFile(dir, "plain.txt");
	makeFile(dir, "unknown.xyzzyq");

	CFStringRef kind;

	kind = kindOf(dir, "folder");
	CHECK(equalsString(kind, "Folder"));
	if (kind) CFRelease(kind);

	kind = kindOf(dir, "Sample.app");
	CHECK(equalsString(kind, "Application"));
	if (kind) CFRelease(kind);

	// A file whose type is not declared is a generic document.
	kind = kindOf(dir, "unknown.xyzzyq");
	CHECK(equalsString(kind, "Document"));
	if (kind) CFRelease(kind);

	// A known type has a description of its own, and the type information and MIME type paths agree
	// with the file path.
	CFStringRef fileKind = kindOf(dir, "plain.txt");
	CHECK(fileKind && CFStringGetLength(fileKind) > 0 && !equalsString(fileKind, "Document"));
	kind = kindForExtension("txt");
	CHECK(fileKind && kind && CFEqual(fileKind, kind));
	if (kind) CFRelease(kind);
	kind = kindForExtension("TXT");
	CHECK(fileKind && kind && CFEqual(fileKind, kind));
	if (kind) CFRelease(kind);
	kind = NULL;
	CHECK_ST(LSCopyKindStringForMIMEType(CFSTR("text/plain"), &kind), noErr);
	CHECK(fileKind && kind && CFEqual(fileKind, kind));
	if (kind) CFRelease(kind);
	if (fileKind) CFRelease(fileKind);

	kind = kindForExtension("xyzzyq");
	CHECK(equalsString(kind, "Document"));
	if (kind) CFRelease(kind);
	kind = NULL;
	CHECK_ST(LSCopyKindStringForMIMEType(CFSTR("application/x-no-such-type-zq"), &kind), noErr);
	CHECK(equalsString(kind, "Document"));
	if (kind) CFRelease(kind);

	// Errors.
	FSRef txt, app, folder;
	CHECK(refFor(dir, "plain.txt", &txt));
	CHECK(refFor(dir, "Sample.app", &app));
	CHECK(refFor(dir, "folder", &folder));
	CFStringRef out = NULL;
	CHECK_ST(LSCopyKindStringForRef(NULL, &out), paramErr);
	CHECK_ST(LSCopyKindStringForRef(&txt, NULL), paramErr);
	CHECK_ST(LSCopyKindStringForURL(NULL, &out), paramErr);
	CFURLRef missing = CFURLCreateWithString(NULL, CFSTR("file:///nonexistent-dir-zq/zz.txt"), NULL);
	CHECK_ST(LSCopyKindStringForURL(missing, &out), fnfErr);
	CFRelease(missing);
	CFURLRef web = CFURLCreateWithString(NULL, CFSTR("http://example.com/a.txt"), NULL);
	CHECK_ST(LSCopyKindStringForURL(web, &out), kLSUnknownErr);
	CFRelease(web);
	CHECK_ST(LSCopyKindStringForMIMEType(NULL, &out), paramErr);
	CHECK_ST(LSCopyKindStringForTypeInfo(0, 0, NULL, &out), paramErr);
	// A NULL output of the type information and MIME type variants is paramErr on Mojave but noErr on
	// macOS 26, so it is not checked.

	// Display name and item info on a reference.
	out = NULL;
	CHECK_ST(LSCopyDisplayNameForRef(&txt, &out), noErr);
	CHECK(equalsString(out, "plain.txt"));
	if (out) CFRelease(out);
	CHECK_ST(LSCopyDisplayNameForRef(NULL, &out), paramErr);
	CHECK_ST(LSCopyDisplayNameForRef(&txt, NULL), paramErr);

	LSItemInfoRecord info;
	memset(&info, 0, sizeof(info));
	CHECK_ST(LSCopyItemInfoForRef(&folder, kLSRequestAllInfo, &info), noErr);
	CHECK(info.flags & kLSItemInfoIsContainer);
	memset(&info, 0, sizeof(info));
	CHECK_ST(LSCopyItemInfoForRef(&txt, kLSRequestAllInfo, &info), noErr);
	CHECK(info.flags & kLSItemInfoIsPlainFile);
	CHECK(equalsString(info.extension, "txt"));
	if (info.extension) CFRelease(info.extension);
	CHECK_ST(LSCopyItemInfoForRef(&txt, kLSRequestAllInfo, NULL), paramErr);
	CHECK_ST(LSCopyItemInfoForRef(NULL, kLSRequestAllInfo, &info), paramErr);

	// Application lookup, acceptance, registration and attributes.
	FSRef appRef;
	CFURLRef appURL = NULL;
	CHECK_ST(LSGetApplicationForItem(NULL, kLSRolesAll, &appRef, &appURL), paramErr);
	FSRef unknown;
	CHECK(refFor(dir, "unknown.xyzzyq", &unknown));
	CHECK_ST(LSGetApplicationForItem(&unknown, kLSRolesAll, &appRef, &appURL), kLSApplicationNotFoundErr);
	CHECK(appURL == NULL);

	Boolean accepts = true;
	CHECK_ST(LSCanRefAcceptItem(&txt, &txt, kLSRolesAll, kLSAcceptDefault, &accepts), kLSNotAnApplicationErr);
	CHECK_ST(LSCanRefAcceptItem(NULL, &txt, kLSRolesAll, kLSAcceptDefault, &accepts), paramErr);
	CHECK_ST(LSCanRefAcceptItem(&txt, NULL, kLSRolesAll, kLSAcceptDefault, &accepts), paramErr);
	CHECK_ST(LSCanRefAcceptItem(&txt, &app, kLSRolesAll, kLSAcceptDefault, NULL), paramErr);

	// Only the error paths: registering would change the database of the system under test.
	CHECK_ST(LSRegisterFSRef(NULL, false), paramErr);

	CHECK_ST(LSSetItemAttribute(NULL, kLSRolesAll, kLSItemExtensionIsHidden, kCFBooleanTrue), paramErr);
	CHECK_ST(LSSetItemAttribute(&txt, kLSRolesAll, NULL, kCFBooleanTrue), paramErr);
	CHECK_ST(LSSetItemAttribute(&txt, kLSRolesAll, kLSItemExtensionIsHidden, kCFBooleanTrue), kLSAttributeNotSettableErr);

	// Handler options default to none; setting them changes the user's preferences, so it is only
	// exercised inside the Darling prefix.
	CHECK(LSGetHandlerOptionsForContentType(CFSTR("public.no-such-type-zq")) == kLSHandlerOptionsDefault);
	CHECK(LSGetHandlerOptionsForContentType(NULL) == kLSHandlerOptionsDefault);
	if (access("/Volumes/SystemRoot", F_OK) == 0) {
		CFStringRef type = CFSTR("public.lskind-test-type");
		CHECK_ST(LSSetHandlerOptionsForContentType(NULL, kLSHandlerOptionsIgnoreCreator), paramErr);
		CHECK_ST(LSSetHandlerOptionsForContentType(type, kLSHandlerOptionsIgnoreCreator), noErr);
		CHECK(LSGetHandlerOptionsForContentType(type) == kLSHandlerOptionsIgnoreCreator);
		CHECK_ST(LSSetHandlerOptionsForContentType(type, kLSHandlerOptionsDefault), noErr);
		CHECK(LSGetHandlerOptionsForContentType(type) == kLSHandlerOptionsDefault);
	}

	printf(failures ? "FAILED\n" : "PASSED\n");
	return failures ? 1 : 0;
}
