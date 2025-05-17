#include <HIToolbox/IBCarbonRuntime.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


OSStatus CreateMenuBarFromNib(IBNibRef inNibRef, CFStringRef inName, Handle *outMenuBar)
{
    if (verbose) {
        puts("STUB: CreateMenuBarFromNib");
    }

	return noErr;
}
OSStatus CreateMenuFromNib(IBNibRef inNibRef, CFStringRef inName, MenuRef *outMenuRef)
{
    if (verbose) {
        puts("STUB: CreateMenuFromNib");
    }

	return noErr;
}
OSStatus CreateNibReference(CFStringRef inNibName, IBNibRef *outNibRef)
{
    if (verbose) {
        puts("STUB: CreateNibReference");
    }

	return noErr;
}
OSStatus CreateNibReferenceWithCFBundle(CFBundleRef inBundle, CFStringRef inNibName, IBNibRef *outNibRef)
{
    if (verbose) {
        puts("STUB: CreateNibReferenceWithCFBundle");
    }

	return noErr;
}
OSStatus CreateWindowFromNib(IBNibRef inNibRef, CFStringRef inName, WindowRef *outWindow)
{
    if (verbose) {
        puts("STUB: CreateWindowFromNib");
    }

	return noErr;
}

void DisposeNibReference(IBNibRef inNibRef)
{
    if (verbose) {
        puts("STUB: DisposeNibReference");
    }
}

OSStatus SetMenuBarFromNib(IBNibRef inNibRef, CFStringRef inName)
{
    if (verbose) {
        puts("STUB: SetMenuBarFromNib");
    }

	return noErr;
}
