// Copyright (C) 2026 Darling Developers
//
// The private CoreUI framework exports the keys and values of the options
// dictionary that AppKit and WebKit pass to CUIDraw. Each constant is a
// constant CFString, all of them differ, and they work as dictionary keys and
// values. Their strings, observed on macOS, are checked by
// private_constant_values.m.
// build: guest-cc.sh -o t coreui_draw_option_keys.m -framework Foundation
#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, name) do { if (!(cond)) { printf("FAIL: %s: %s\n", name, #cond); failures++; } } while (0)

static const char *const names[] = {
	"kCUIWidgetKey",
	"kCUIStateKey",
	"kCUIValueKey",
	"kCUISizeKey",
	"kCUIScaleKey",
	"kCUIOrientationKey",
	"kCUIPresentationStateKey",
	"kCUIUserInterfaceLayoutDirectionKey",
	"kCUIIsFlippedKey",
	"kCUIMaskOnlyKey",
	"kCUIAnimationStartTimeKey",
	"kCUIAnimationTimeKey",
	"kCUIWidgetButtonComboBox",
	"kCUIWidgetButtonLittleArrows",
	"kCUIWidgetProgressBar",
	"kCUIWidgetProgressIndeterminateBar",
	"kCUIWidgetScrollBarTrackCorner",
	"kCUIWidgetSwitchBorder",
	"kCUIWidgetSwitchFill",
	"kCUIWidgetSwitchFillMask",
	"kCUIWidgetSwitchKnob",
	"kCUIWidgetSwitchOnOffLabel",
	"kCUIStateActive",
	"kCUIStateDisabled",
	"kCUIStatePressed",
	"kCUISizeMini",
	"kCUISizeSmall",
	"kCUISizeRegular",
	"kCUIOrientHorizontal",
	"kCUIPresentationStateActiveKey",
	"kCUIPresentationStateInactive",
	"kCUIUserInterfaceLayoutDirectionLeftToRight",
	"kCUIUserInterfaceLayoutDirectionRightToLeft",
};

int main(void)
{
	@autoreleasepool {
		void *coreUI = dlopen("/System/Library/PrivateFrameworks/CoreUI.framework/Versions/A/CoreUI", RTLD_NOW);
		if (!coreUI) {
			printf("FAIL: dlopen CoreUI: %s\n", dlerror());
			return 1;
		}

		const size_t count = sizeof(names) / sizeof(names[0]);
		CFStringRef values[count];
		for (size_t i = 0; i < count; i++) {
			const CFStringRef *symbol = dlsym(coreUI, names[i]);
			values[i] = symbol ? *symbol : NULL;
			CHECK(symbol != NULL, names[i]);
			if (!values[i])
				continue;
			CHECK(CFGetTypeID(values[i]) == CFStringGetTypeID(), names[i]);
			CHECK(CFStringGetLength(values[i]) > 0, names[i]);
			for (size_t j = 0; j < i; j++)
				CHECK(!values[j] || !CFEqual(values[i], values[j]), names[i]);
		}

		// The way the clients use them: an options dictionary keyed by the constants.
		const CFStringRef *widgetKey = dlsym(coreUI, "kCUIWidgetKey");
		const CFStringRef *stateKey = dlsym(coreUI, "kCUIStateKey");
		const CFStringRef *knob = dlsym(coreUI, "kCUIWidgetSwitchKnob");
		const CFStringRef *pressed = dlsym(coreUI, "kCUIStatePressed");
		if (widgetKey && stateKey && knob && pressed) {
			NSDictionary *options = @{ (__bridge NSString *)*widgetKey: (__bridge NSString *)*knob,
			                           (__bridge NSString *)*stateKey: (__bridge NSString *)*pressed };
			NSString *copy = [[(__bridge NSString *)*widgetKey mutableCopy] autorelease];
			CHECK([options[copy] isEqual:(__bridge NSString *)*knob], "lookup by an equal key");
			CHECK(options[(__bridge NSString *)*stateKey] == (__bridge NSString *)*pressed, "state value");
		}

		if (failures == 0)
			printf("PASS: %zu CoreUI draw option constants\n", count);
	}
	return failures ? 1 : 0;
}
