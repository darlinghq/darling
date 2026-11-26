/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <CommonPanels/CommonPanels.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CCP_FontPanelClose(void)
{
    if (verbose) puts("STUB: CCP_FontPanelClose called");
    return NULL;
}

void* CCP_FontPanelGetSelection(void)
{
    if (verbose) puts("STUB: CCP_FontPanelGetSelection called");
    return NULL;
}

void* CCP_FontPanelIsVisible(void)
{
    if (verbose) puts("STUB: CCP_FontPanelIsVisible called");
    return NULL;
}

void* CCP_FontPanelOpen(void)
{
    if (verbose) puts("STUB: CCP_FontPanelOpen called");
    return NULL;
}

void* CCP_FontPanelSelectFonts(void)
{
    if (verbose) puts("STUB: CCP_FontPanelSelectFonts called");
    return NULL;
}

void* CCP_NPickColor(void)
{
    if (verbose) puts("STUB: CCP_NPickColor called");
    return NULL;
}

void* CMCalibrateDisplay(void)
{
    if (verbose) puts("STUB: CMCalibrateDisplay called");
    return NULL;
}

void* Call_CommonCocoaPanels_NPickColor(void)
{
    if (verbose) puts("STUB: Call_CommonCocoaPanels_NPickColor called");
    return NULL;
}

void* DisposeCalibrateEventUPP(void)
{
    if (verbose) puts("STUB: DisposeCalibrateEventUPP called");
    return NULL;
}

void* DisposeCalibrateUPP(void)
{
    if (verbose) puts("STUB: DisposeCalibrateUPP called");
    return NULL;
}

void* DisposeCanCalibrateUPP(void)
{
    if (verbose) puts("STUB: DisposeCanCalibrateUPP called");
    return NULL;
}

void* FCAddCollection(void)
{
    if (verbose) puts("STUB: FCAddCollection called");
    return NULL;
}

void* FCAddFontDescriptorToCollection(void)
{
    if (verbose) puts("STUB: FCAddFontDescriptorToCollection called");
    return NULL;
}

void* FCCopyCollectionNames(void)
{
    if (verbose) puts("STUB: FCCopyCollectionNames called");
    return NULL;
}

void* FCCopyFontDescriptorsInCollection(void)
{
    if (verbose) puts("STUB: FCCopyFontDescriptorsInCollection called");
    return NULL;
}

void* FCFontDescriptorCreateWithFontAttributes(void)
{
    if (verbose) puts("STUB: FCFontDescriptorCreateWithFontAttributes called");
    return NULL;
}

void* FCFontDescriptorCreateWithName(void)
{
    if (verbose) puts("STUB: FCFontDescriptorCreateWithName called");
    return NULL;
}

void* FCRemoveCollection(void)
{
    if (verbose) puts("STUB: FCRemoveCollection called");
    return NULL;
}

void* FCRemoveFontDescriptorFromCollection(void)
{
    if (verbose) puts("STUB: FCRemoveFontDescriptorFromCollection called");
    return NULL;
}

void* FPIsFontPanelVisible(void)
{
    if (verbose) puts("STUB: FPIsFontPanelVisible called");
    return NULL;
}

void* FPShowHideFontPanel(void)
{
    if (verbose) puts("STUB: FPShowHideFontPanel called");
    return NULL;
}

void* GetColor(void)
{
    if (verbose) puts("STUB: GetColor called");
    return NULL;
}

void* GetHandleApplicationEventsUPP(void)
{
    if (verbose) puts("STUB: GetHandleApplicationEventsUPP called");
    return NULL;
}

void* GetHandleWinEventsUPP(void)
{
    if (verbose) puts("STUB: GetHandleWinEventsUPP called");
    return NULL;
}

void* InvokeCalibrateEventUPP(void)
{
    if (verbose) puts("STUB: InvokeCalibrateEventUPP called");
    return NULL;
}

void* InvokeCalibrateUPP(void)
{
    if (verbose) puts("STUB: InvokeCalibrateUPP called");
    return NULL;
}

void* InvokeCanCalibrateUPP(void)
{
    if (verbose) puts("STUB: InvokeCanCalibrateUPP called");
    return NULL;
}

void* LoadNSTypographyBundle(void)
{
    if (verbose) puts("STUB: LoadNSTypographyBundle called");
    return NULL;
}

void* NewCalibrateEventUPP(void)
{
    if (verbose) puts("STUB: NewCalibrateEventUPP called");
    return NULL;
}

void* NewCalibrateUPP(void)
{
    if (verbose) puts("STUB: NewCalibrateUPP called");
    return NULL;
}

void* NewCanCalibrateUPP(void)
{
    if (verbose) puts("STUB: NewCanCalibrateUPP called");
    return NULL;
}

void* SetFontInfoForSelection(void)
{
    if (verbose) puts("STUB: SetFontInfoForSelection called");
    return NULL;
}

void* SoftLinkingGetFunctionPointerFromBundleInDirectory(void)
{
    if (verbose) puts("STUB: SoftLinkingGetFunctionPointerFromBundleInDirectory called");
    return NULL;
}

void* UserWantsColorPanelFromCommonCocoaPanels(void)
{
    if (verbose) puts("STUB: UserWantsColorPanelFromCommonCocoaPanels called");
    return NULL;
}

void* _SoftLinkingLoadBundleFromLocation(void)
{
    if (verbose) puts("STUB: _SoftLinkingLoadBundleFromLocation called");
    return NULL;
}

#ifndef __LP64__

#import <Foundation/Foundation.h>
#import <objc/message.h>
#include <dlfcn.h>

// NPickColor() shows the colour panel of the application framework, which is not linked: the Carbon framework
// loads it when a colour is first picked, as it does for Navigation Services. The panel is run as a modal
// window with the buttons OK and Cancel in its accessory view. When the user accepts the dialog, the program
// gets the colour that the panel has, as a 16 bit RGB colour, and newColorChosen says so.

enum { kPanelAccepted = 1, kPanelCancelled = 0 };

// The buttons of the accessory view send these
@interface CarbonColorPickerButtons : NSObject
- (void) accept: (id) sender;
- (void) cancel: (id) sender;
@end

static void stopModal(NSInteger code)
{
	id app = ((id (*)(id, SEL)) objc_msgSend)(NSClassFromString(@"NSApplication"), @selector(sharedApplication));

	((void (*)(id, SEL, NSInteger)) objc_msgSend)(app, @selector(stopModalWithCode:), code);
}

@implementation CarbonColorPickerButtons
- (void) accept: (id) sender
{
	stopModal(kPanelAccepted);
}

- (void) cancel: (id) sender
{
	stopModal(kPanelCancelled);
}
@end

static id newObject(NSString* className, CGRect frame)
{
	id object = ((id (*)(id, SEL)) objc_msgSend)(NSClassFromString(className), @selector(alloc));

	return ((id (*)(id, SEL, CGRect)) objc_msgSend)(object, @selector(initWithFrame:), frame);
}

static id makeButton(NSString* title, CGRect frame, id target, SEL action, NSString* keyEquivalent)
{
	id button = newObject(@"NSButton", frame);

	((void (*)(id, SEL, NSString*)) objc_msgSend)(button, @selector(setTitle:), title);
	((void (*)(id, SEL, NSUInteger)) objc_msgSend)(button, @selector(setBezelStyle:), 1 /* NSRoundedBezelStyle */);
	((void (*)(id, SEL, id)) objc_msgSend)(button, @selector(setTarget:), target);
	((void (*)(id, SEL, SEL)) objc_msgSend)(button, @selector(setAction:), action);
	((void (*)(id, SEL, NSString*)) objc_msgSend)(button, @selector(setKeyEquivalent:), keyEquivalent);
	return [button autorelease];
}

static id makeColor(const RGBColor* rgb)
{
	return ((id (*)(id, SEL, CGFloat, CGFloat, CGFloat, CGFloat)) objc_msgSend)(NSClassFromString(@"NSColor"),
		@selector(colorWithCalibratedRed:green:blue:alpha:), rgb->red / 65535.0f, rgb->green / 65535.0f,
		rgb->blue / 65535.0f, 1.0f);
}

static UInt16 toComponent(CGFloat value)
{
	if (value <= 0)
		return 0;
	if (value >= 1)
		return 65535;
	return (UInt16) (value * 65535.0f + 0.5f);
}

// The colour the panel has, as the 16 bit RGB colour of the Color Picker
static BOOL readColor(id color, RGBColor* rgb)
{
	CGFloat red = 0, green = 0, blue = 0, alpha = 0;

	color = ((id (*)(id, SEL, NSString*)) objc_msgSend)(color, @selector(colorUsingColorSpaceName:), @"NSCalibratedRGBColorSpace");
	if (!color)
		return NO;
	((void (*)(id, SEL, CGFloat*, CGFloat*, CGFloat*, CGFloat*)) objc_msgSend)(color,
		@selector(getRed:green:blue:alpha:), &red, &green, &blue, &alpha);
	rgb->red = toComponent(red);
	rgb->green = toComponent(green);
	rgb->blue = toComponent(blue);
	return YES;
}

static BOOL runColorPanel(NColorPickerInfo* info)
{
	static dispatch_once_t once;
	static BOOL loaded;
	BOOL accepted = NO;

	dispatch_once(&once, ^{
		loaded = dlopen("/System/Library/Frameworks/AppKit.framework/AppKit", RTLD_LAZY | RTLD_GLOBAL) != NULL;
	});
	if (!loaded)
		return NO;

	@autoreleasepool
	{
		Class panelClass = NSClassFromString(@"NSColorPanel");
		id app = ((id (*)(id, SEL)) objc_msgSend)(NSClassFromString(@"NSApplication"), @selector(sharedApplication));
		id panel, accessory;
		CarbonColorPickerButtons* buttons = [[CarbonColorPickerButtons alloc] init];
		CFStringRef prompt = CFStringCreateWithPascalString(NULL, info->prompt, kCFStringEncodingMacRoman);

		if (!panelClass || !app)
			return NO;
		panel = ((id (*)(id, SEL)) objc_msgSend)(panelClass, @selector(sharedColorPanel));
		((void (*)(id, SEL, id)) objc_msgSend)(panel, @selector(setColor:), makeColor(&info->theColor.color.rgb));
		if (prompt && CFStringGetLength(prompt) > 0)
			((void (*)(id, SEL, NSString*)) objc_msgSend)(panel, @selector(setTitle:), (NSString*) prompt);
		if (prompt)
			CFRelease(prompt);

		accessory = [newObject(@"NSView", CGRectMake(0, 0, 220, 40)) autorelease];
		((void (*)(id, SEL, id)) objc_msgSend)(accessory, @selector(addSubview:),
			makeButton(@"Cancel", CGRectMake(20, 8, 90, 24), buttons, @selector(cancel:), @"\033"));
		((void (*)(id, SEL, id)) objc_msgSend)(accessory, @selector(addSubview:),
			makeButton(@"OK", CGRectMake(120, 8, 90, 24), buttons, @selector(accept:), @"\r"));
		((void (*)(id, SEL, id)) objc_msgSend)(panel, @selector(setAccessoryView:), accessory);
		((void (*)(id, SEL, id)) objc_msgSend)(panel, @selector(makeKeyAndOrderFront:), nil);

		if (((NSInteger (*)(id, SEL, id)) objc_msgSend)(app, @selector(runModalForWindow:), panel) == kPanelAccepted)
		{
			RGBColor rgb;
			id color = ((id (*)(id, SEL)) objc_msgSend)(panel, @selector(color));

			if (readColor(color, &rgb))
			{
				info->theColor.color.rgb = rgb;
				accepted = YES;
			}
		}
		((void (*)(id, SEL, id)) objc_msgSend)(panel, @selector(orderOut:), nil);
		((void (*)(id, SEL, id)) objc_msgSend)(panel, @selector(setAccessoryView:), nil);
		[buttons release];
	}
	return accepted;
}

// NPickColor() returns when the dialog is closed. Cancelling leaves the colour as it was.
OSErr NPickColor(NColorPickerInfo* theColorInfo)
{
	if (theColorInfo == NULL)
		return paramErr;
	theColorInfo->newColorChosen = runColorPanel(theColorInfo);
	return noErr;
}

#undef NewNColorChangedUPP
#undef DisposeNColorChangedUPP
#undef InvokeNColorChangedUPP
NColorChangedUPP NewNColorChangedUPP(NColorChangedProcPtr userRoutine)
{
    return userRoutine;
}

void DisposeNColorChangedUPP(NColorChangedUPP userUPP)
{
}

void InvokeNColorChangedUPP(SRefCon userData, NPMColor* newColor, NColorChangedUPP userUPP)
{
    (*userUPP)(userData, newColor);
}
#endif
