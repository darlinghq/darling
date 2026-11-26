// Copyright (C) 2026 Lubos Dolezel
//
// The values of private constants (and public ones without a documented
// value) that WebKit uses. Each expected value was read from the exported
// constant on macOS 26.7.3 (black-box observation, with the probe described
// below); the constants macOS 10.14.4 also exports have the same values there.
// The probe dlopen()s the frameworks listed in kLibraries, looks each name up
// with dlsym() and prints the CFString, or the CGFloat for the width and weight
// constants. This test does the same and compares.
// build: guest-cc.sh -o t private_constant_values.m -framework Foundation
#import <Foundation/Foundation.h>
#include <CoreGraphics/CGBase.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *kLibraries[] = {
	"/System/Library/Frameworks/AppKit.framework/AppKit",
	"/System/Library/Frameworks/CoreText.framework/CoreText",
	"/System/Library/Frameworks/QuartzCore.framework/QuartzCore",
	"/System/Library/Frameworks/CFNetwork.framework/CFNetwork",
	"/System/Library/PrivateFrameworks/CoreUI.framework/CoreUI",
	"/usr/lib/libAccessibility.dylib",
	NULL
};

static const struct { const char *name; const char *value; } kStrings[] = {
	{ "NSPopUpMenuPopupButtonBounds", "NSPopUpMenuPopupButtonBounds" },
	{ "NSPopUpMenuPopupButtonLabelOffset", "NSPopUpMenuPopupButtonLabelOffset" },
	{ "NSPopUpMenuPopupButtonSize", "NSPopUpMenuPopupButtonSize" },
	{ "NSPopUpMenuPopupButtonWidget", "NSPopUpMenuPopupButtonWidget" },
	{ "NSTextCompletionAttributeName", "NSTextCompletion" },
	{ "NSTextInsertionUndoableAttributeName", "NSTextInsertionUndoable" },
	{ "NSURLAuthenticationMethodOAuth", "NSURLAuthenticationMethodOAuth" },
	{ "_NXSmartPaste", "NeXT smart paste pasteboard type" },
	{ "kAXSEnhanceTextLegibilityChangedNotification", "com.apple.accessibility.text.legibility.status" },
	{ "kCAFilterColorBlendMode", "colorBlendMode" },
	{ "kCAFilterColorBurnBlendMode", "colorBurnBlendMode" },
	{ "kCAFilterColorDodgeBlendMode", "colorDodgeBlendMode" },
	{ "kCAFilterDarkenBlendMode", "darkenBlendMode" },
	{ "kCAFilterDifferenceBlendMode", "differenceBlendMode" },
	{ "kCAFilterExclusionBlendMode", "exclusionBlendMode" },
	{ "kCAFilterHardLightBlendMode", "hardLightBlendMode" },
	{ "kCAFilterHueBlendMode", "hueBlendMode" },
	{ "kCAFilterLightenBlendMode", "lightenBlendMode" },
	{ "kCAFilterLuminosityBlendMode", "luminosityBlendMode" },
	{ "kCAFilterOverlayBlendMode", "overlayBlendMode" },
	{ "kCAFilterPlusD", "plusD" },
	{ "kCAFilterPlusL", "plusL" },
	{ "kCAFilterSaturationBlendMode", "saturationBlendMode" },
	{ "kCAFilterScreenBlendMode", "screenBlendMode" },
	{ "kCAFilterSoftLightBlendMode", "softLightBlendMode" },
	{ "kCGGradientInterpolatesPremultiplied", "kCGGradientInterpolatesPremultiplied" },
	{ "kCTFontCSSFamilyCursive", "cursive" },
	{ "kCTFontCSSFamilyFantasy", "fantasy" },
	{ "kCTFontCSSFamilyMonospace", "monospace" },
	{ "kCTFontCSSFamilySansSerif", "sans-serif" },
	{ "kCTFontCSSFamilySerif", "serif" },
	{ "kCTFontCSSWeightAttribute", "CTFontCSSWeightAttribute" },
	{ "kCTFontCSSWidthAttribute", "CTFontCSSWidthAttribute" },
	{ "kCTFontContentSizeCategoryL", "UICTContentSizeCategoryL" },
	{ "kCTFontDescriptorTextStyleAttribute", "NSCTFontUIUsageAttribute" },
	{ "kCTFontFallbackOptionAttribute", "NSCTFontFallbackOptionAttribute" },
	{ "kCTFontGradeTrait", "NSCTFontGradeTrait" },
	{ "kCTFontIgnoreLegibilityWeightAttribute", "CTFontIgnoreLegibilityWeightAttribute" },
	{ "kCTFontOpticalSizeAttribute", "NSCTFontOpticalSizeAttribute" },
	{ "kCTFontPaletteAttribute", "CTFontPaletteAttribute" },
	{ "kCTFontPaletteColorsAttribute", "CTFontPaletteColorsAttribute" },
	{ "kCTFontPostScriptNameAttribute", "NSCTFontPostScriptNameAttribute" },
	{ "kCTFontReferenceURLAttribute", "NSCTFontReferenceURLAttribute" },
	{ "kCTFontSizeCategoryAttribute", "NSCTFontSizeCategoryAttribute" },
	{ "kCTFontTrackAttribute", "NSCTFontTrackAttribute" },
	{ "kCTFontUIFontDesignDefault", "NSCTFontUIFontDesignDefault" },
	{ "kCTFontUIFontDesignMonospaced", "NSCTFontUIFontDesignMonospaced" },
	{ "kCTFontUIFontDesignRounded", "NSCTFontUIFontDesignRounded" },
	{ "kCTFontUIFontDesignSerif", "NSCTFontUIFontDesignSerif" },
	{ "kCTFontUIFontDesignTrait", "NSCTFontUIFontDesignTrait" },
	{ "kCTFontUnscaledTrackingAttribute", "NSCTFontUnscaledTrackingAttribute" },
	{ "kCTFontUserInstalledAttribute", "NSCTFontUserInstalledAttribute" },
	{ "kCTFrameMaximumNumberOfLinesAttributeName", "CTFrameMaximumNumberOfLines" },
	{ "kCTUIFontTextStyleBody", "UICTFontTextStyleBody" },
	{ "kCTUIFontTextStyleCaption1", "UICTFontTextStyleCaption1" },
	{ "kCTUIFontTextStyleCaption2", "UICTFontTextStyleCaption2" },
	{ "kCTUIFontTextStyleFootnote", "UICTFontTextStyleFootnote" },
	{ "kCTUIFontTextStyleHeadline", "UICTFontTextStyleHeadline" },
	{ "kCTUIFontTextStyleShortBody", "UICTFontTextStyleShortBody" },
	{ "kCTUIFontTextStyleShortCaption1", "UICTFontTextStyleShortCaption1" },
	{ "kCTUIFontTextStyleShortFootnote", "UICTFontTextStyleShortFootnote" },
	{ "kCTUIFontTextStyleShortHeadline", "UICTFontTextStyleShortHeadline" },
	{ "kCTUIFontTextStyleShortSubhead", "UICTFontTextStyleShortSubhead" },
	{ "kCTUIFontTextStyleSubhead", "UICTFontTextStyleSubhead" },
	{ "kCTUIFontTextStyleTallBody", "UICTFontTextStyleTallBody" },
	{ "kCTUIFontTextStyleTitle0", "UICTFontTextStyleTitle0" },
	{ "kCTUIFontTextStyleTitle1", "UICTFontTextStyleTitle1" },
	{ "kCTUIFontTextStyleTitle2", "UICTFontTextStyleTitle2" },
	{ "kCTUIFontTextStyleTitle3", "UICTFontTextStyleTitle3" },
	{ "kCTUIFontTextStyleTitle4", "UICTFontTextStyleTitle4" },
	{ "kCUIAnimationStartTimeKey", "animstart" },
	{ "kCUIAnimationTimeKey", "animend" },
	{ "kCUIIsFlippedKey", "is.flipped" },
	{ "kCUIMaskOnlyKey", "kCUIMaskOnlyKey" },
	{ "kCUIOrientHorizontal", "kCUIOrientHorizontal" },
	{ "kCUIOrientationKey", "kCUIOrientationKey" },
	{ "kCUIPresentationStateActiveKey", "kCUIPresentationStateActiveKey" },
	{ "kCUIPresentationStateInactive", "kCUIPresentationStateInactive" },
	{ "kCUIPresentationStateKey", "kCUIPresentationStateKey" },
	{ "kCUIScaleKey", "kCUIScaleKey" },
	{ "kCUISizeKey", "size" },
	{ "kCUISizeMini", "mini" },
	{ "kCUISizeRegular", "regular" },
	{ "kCUISizeSmall", "small" },
	{ "kCUIStateActive", "normal" },
	{ "kCUIStateDisabled", "disabled" },
	{ "kCUIStateKey", "state" },
	{ "kCUIStatePressed", "pressed" },
	{ "kCUIUserInterfaceLayoutDirectionKey", "kCUIUserInterfaceLayoutDirectionKey" },
	{ "kCUIUserInterfaceLayoutDirectionLeftToRight", "kCUIUserInterfaceLayoutDirectionLeftToRight" },
	{ "kCUIUserInterfaceLayoutDirectionRightToLeft", "kCUIUserInterfaceLayoutDirectionRightToLeft" },
	{ "kCUIValueKey", "value" },
	{ "kCUIWidgetButtonComboBox", "combobox" },
	{ "kCUIWidgetButtonLittleArrows", "littlearrows" },
	{ "kCUIWidgetKey", "widget" },
	{ "kCUIWidgetProgressBar", "kCUIWidgetProgressBar" },
	{ "kCUIWidgetProgressIndeterminateBar", "kCUIWidgetIndeterminateBar" },
	{ "kCUIWidgetScrollBarTrackCorner", "kCUIWidgetScrollBarTrackCorner" },
	{ "kCUIWidgetSwitchBorder", "kCUIWidgetSwitchBorder" },
	{ "kCUIWidgetSwitchFill", "kCUIWidgetSwitchFill" },
	{ "kCUIWidgetSwitchFillMask", "kCUIWidgetSwitchFillMask" },
	{ "kCUIWidgetSwitchKnob", "kCUIWidgetSwitchKnob" },
	{ "kCUIWidgetSwitchOnOffLabel", "kCUIWidgetSwitchOnOffLabel" },
};

static const struct { const char *name; double value; } kNumbers[] = {
	{ "kCTFontWeightBlack", 0.62000000476837158 },
	{ "kCTFontWeightBold", 0.40000000596046448 },
	{ "kCTFontWeightHeavy", 0.56000000238418579 },
	{ "kCTFontWeightLight", -0.40000000596046448 },
	{ "kCTFontWeightMedium", 0.23000000417232513 },
	{ "kCTFontWeightRegular", 0 },
	{ "kCTFontWeightSemibold", 0.30000001192092896 },
	{ "kCTFontWeightThin", -0.60000002384185791 },
	{ "kCTFontWeightUltraLight", -0.80000001192092896 },
	{ "kCTFontWidthCondensed", -0.20000000000000001 },
	{ "kCTFontWidthExpanded", 0.20000000000000001 },
	{ "kCTFontWidthExtraCompressed", -0.40000000000000002 },
	{ "kCTFontWidthExtraCondensed", -0.29999999999999999 },
	{ "kCTFontWidthExtraExpanded", 0.29999999999999999 },
	{ "kCTFontWidthSemiCondensed", -0.10000000000000001 },
	{ "kCTFontWidthSemiExpanded", 0.10000000000000001 },
	{ "kCTFontWidthStandard", 0 },
	{ "kCTFontWidthUltraCompressed", -0.5 },
};

int main(void)
{
	int failures = 0;
	for (int i = 0; kLibraries[i]; i++)
		dlopen(kLibraries[i], RTLD_LAZY | RTLD_GLOBAL);
	for (size_t i = 0; i < sizeof kStrings / sizeof kStrings[0]; i++) {
		const CFStringRef *symbol = dlsym(RTLD_DEFAULT, kStrings[i].name);
		NSString *value = symbol ? (NSString *) *symbol : nil;
		if (![value isEqualToString: @(kStrings[i].value)]) {
			printf("FAIL: %s is \"%s\", expected \"%s\"\n", kStrings[i].name,
				symbol ? [value UTF8String] : "(missing)", kStrings[i].value);
			failures++;
		}
	}
	for (size_t i = 0; i < sizeof kNumbers / sizeof kNumbers[0]; i++) {
		const CGFloat *symbol = dlsym(RTLD_DEFAULT, kNumbers[i].name);
		// macOS 26.7.3 holds some of these as widened single-precision values
		if (!symbol || fabs(*symbol - kNumbers[i].value) > 1e-6) {
			printf("FAIL: %s is %g, expected %g\n", kNumbers[i].name,
				symbol ? (double) *symbol : NAN, kNumbers[i].value);
			failures++;
		}
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
