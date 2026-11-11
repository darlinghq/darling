// Copyright (C) 2026 Lubos Dolezel
//
// Guest test: OnBoardingKit, AppSupport, SoftLinking, GeoKit and SoftwareUpdate load from their
// install names and export what their clients bind. Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include <dlfcn.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <stdio.h>

static int failures;

static void expect(BOOL ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static void *load(const char *name)
{
	char path[256];
	snprintf(path, sizeof(path), "/System/Library/PrivateFrameworks/%s.framework/Versions/A/%s", name, name);
	void *handle = dlopen(path, RTLD_NOW);
	if (!handle)
		printf("dlopen %s: %s\n", path, dlerror());
	expect(handle != NULL, name);
	return handle;
}

static BOOL classDerivesFrom(const char *name, const char *base)
{
	Class c = objc_getClass(name);
	Class b = objc_getClass(base);
	for (; c; c = class_getSuperclass(c))
		if (c == b)
			return b != Nil;
	return NO;
}

int main(void)
{
	@autoreleasepool {
		// OnBoardingKit: the fourteen classes, five identifiers, and the AppKit superclasses.
		void *onboarding = load("OnBoardingKit");
		const char *obViews[] = { "OBTemplateView", "OBTemplatePartBulletList" };
		const char *obButtons[] = { "OBBoldTrayButton", "OBLinkTrayButton", "OBBulletedListItemLinkButton" };
		const char *obControllers[] = { "OBTemplateContainerViewController", "OBTableWelcomeController", "OBWelcomeController",
			"OBNavigationController", "OBPrivacyLinkController", "OBPrivacySplashController" };
		const char *obPlain[] = { "OBBundle", "OBPrivacyPresenter", "OBPrivacyFlow" };
		for (size_t i = 0; i < 2; i++)
			expect(classDerivesFrom(obViews[i], "NSView"), obViews[i]);
		for (size_t i = 0; i < 3; i++)
			expect(classDerivesFrom(obButtons[i], "NSButton"), obButtons[i]);
		for (size_t i = 0; i < 6; i++)
			expect(classDerivesFrom(obControllers[i], "NSViewController"), obControllers[i]);
		for (size_t i = 0; i < 3; i++)
			expect(objc_getClass(obPlain[i]) != Nil, obPlain[i]);
		const char *identifiers[] = { "OBPrivacyAppStoreIdentifier", "OBPrivacyAppleMusicIdentifier", "OBPrivacyAppleArcadeIdentifier",
			"OBPrivacyTVAppIdentifier", "OBPrivacyiTunesStoreIdentifier" };
		for (size_t i = 0; i < 5; i++) {
			NSString **p = onboarding ? dlsym(onboarding, identifiers[i]) : NULL;
			expect(p && [*p length] > 0, identifiers[i]);
		}
		id presenter = [objc_getClass("OBPrivacyPresenter") presenterForBundleIdentifier: @"org.darlinghq.test"];
		expect([[presenter servicesRequiringConsent] count] == 0, "OBPrivacyPresenter reports no services");
		id splash = [[objc_getClass("OBPrivacySplashController") alloc] init];
		[splash showPrivacySplash];
		expect(splash != nil, "OBPrivacySplashController instantiates");

		// AppSupport: three C functions; the bitmap helpers report failure.
		void *appSupport = load("AppSupport");
		void (*flush)(void) = appSupport ? dlsym(appSupport, "CPLoggingFlush") : NULL;
		expect(flush != NULL, "CPLoggingFlush");
		if (flush)
			flush();
		CFArrayRef (*readImages)(CFStringRef, CFDictionaryRef) = appSupport ? dlsym(appSupport, "CPBitmapCreateImagesFromPath") : NULL;
		expect(readImages != NULL && readImages(CFSTR("/nonexistent.png"), NULL) == NULL, "CPBitmapCreateImagesFromPath fails");
		expect(appSupport && dlsym(appSupport, "CPBitmapWriteImagesToPath") != NULL, "CPBitmapWriteImagesToPath");

		// SoftLinking: _sl_dlopen is a dlopen that reports NULL for an absent library.
		void *softLinking = load("SoftLinking");
		void *(*slOpen)(const char *, int) = softLinking ? dlsym(softLinking, "_sl_dlopen") : NULL;
		expect(slOpen != NULL, "_sl_dlopen is exported");
		if (slOpen) {
			void *libSystem = slOpen("/usr/lib/libSystem.B.dylib", RTLD_NOW);
			expect(libSystem != NULL, "_sl_dlopen loads a library that is present");
			expect(slOpen("/usr/lib/libdoesnotexist.dylib", RTLD_NOW) == NULL, "_sl_dlopen returns NULL for an absent library");
		}
		void *(*getFunction)(const char *, const char *) = softLinking ? dlsym(softLinking, "TSUSoftLinkingGetFrameworkFunction") : NULL;
		expect(getFunction != NULL && getFunction("Missing", "missing") == NULL, "TSUSoftLinkingGetFrameworkFunction returns NULL");

		// GeoKit: two classes and two contexts that are NULL.
		void *geoKit = load("GeoKit");
		expect(objc_getClass("GEOCity") != Nil && objc_getClass("GeoKitPlace") != Nil, "GeoKit classes");
		id *context = geoKit ? dlsym(geoKit, "GEOManagedObjectContext") : NULL;
		id *defaultContext = geoKit ? dlsym(geoKit, "GEODefaultManagedObjectContext") : NULL;
		expect(context != NULL && *context == nil, "GEOManagedObjectContext is NULL");
		expect(defaultContext != NULL && *defaultContext == nil, "GEODefaultManagedObjectContext is NULL");

		// SoftwareUpdate: a check completes at once with no update.
		load("SoftwareUpdate");
		id controller = [[objc_getClass("SUSoftwareUpdateController") alloc] init];
		__block BOOL completed = NO;
		__block id found = @"unset";
		[controller checkForUpdatesWithCompletion: ^(id update) {
			completed = YES;
			found = update;
		}];
		expect(completed && found == nil, "SUSoftwareUpdateController reports no update");
		BOOL (*apply)(id, SEL, id, NSError **) = (BOOL (*)(id, SEL, id, NSError **)) objc_msgSend;
		expect(!apply(controller, sel_registerName("applyUpdate:error:"), nil, NULL), "SUSoftwareUpdateController applies nothing");

		printf("failures=%d\n", failures);
		return failures != 0;
	}
}
