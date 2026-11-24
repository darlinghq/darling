// CFLAGS: -framework Foundation -framework QuickLookUI
// Copyright (C) 2026 Lubos Dolezel
//
// QuickLookUI is a public framework (macOS 12.0+, with QLPreviewPanel and QLPreviewView documented from 10.6),
// and a client that links it imports _OBJC_CLASS_$_QLPreviewView from it. Quartz is the umbrella above it, so a
// client that links Quartz must reach the same classes. Both images are opened with RTLD_FIRST, which makes dlsym
// look only at the image itself and what it re-exports, not at its other dependencies; the two lookups must
// therefore resolve to the same class objects. On Darling the classes are defined in QuickLookUI and Quartz
// re-exports it; on a Mac the same test passes whichever image defines them.
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// The 32-bit Objective-C runtime has no OBJC_CLASS_$_ symbols to look up (no dlsym-able class symbol is found for
// these classes in Quartz or QuickLookUI on macOS 10.14 i386), so only the 64-bit slice checks them.
#ifndef __i386__
static void checkClassSymbol(void *quickLookUI, void *quartz, const char *symbol)
{
	void *direct = dlsym(quickLookUI, symbol);
	void *viaQuartz = dlsym(quartz, symbol);
	CHECK(direct != NULL);
	CHECK(viaQuartz != NULL);
	CHECK(direct == viaQuartz);
}
#endif

int main(void)
{
	@autoreleasepool {
		void *quickLookUI = dlopen("/System/Library/Frameworks/QuickLookUI.framework/Versions/A/QuickLookUI", RTLD_NOW | RTLD_FIRST);
		void *quartz = dlopen("/System/Library/Frameworks/Quartz.framework/Versions/A/Quartz", RTLD_NOW | RTLD_FIRST);
		CHECK(quickLookUI != NULL);
		CHECK(quartz != NULL);
#ifndef __i386__
		if (quickLookUI != NULL && quartz != NULL) {
			checkClassSymbol(quickLookUI, quartz, "OBJC_CLASS_$_QLPreviewView");
			checkClassSymbol(quickLookUI, quartz, "OBJC_CLASS_$_QLPreviewPanel");
		}
#endif

		Class view = NSClassFromString(@"QLPreviewView");
		Class panel = NSClassFromString(@"QLPreviewPanel");
		CHECK(view != Nil);
		CHECK(panel != Nil);
		CHECK(view != panel);
		CHECK([NSStringFromClass(view) isEqualToString:@"QLPreviewView"]);
		CHECK([NSStringFromClass(panel) isEqualToString:@"QLPreviewPanel"]);
	}

	printf(g_failures == 0 ? "PASS\n" : "FAIL\n");
	return g_failures != 0;
}
