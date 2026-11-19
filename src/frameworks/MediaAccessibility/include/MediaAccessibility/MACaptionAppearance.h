/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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


#ifndef MediaAccessibility_MACaptionAppearance_h
#define MediaAccessibility_MACaptionAppearance_h

#include <MediaAccessibility/MADefinitions.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>

MA_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

// CFNotification posted when user-defined caption settings change.
MA_EXPORT const CFStringRef kMACaptionAppearanceSettingsChangedNotification;

// Which set of preferences is queried. The user domain is what captions should be
// rendered with; the default domain holds the system defaults, meant for comparison.
typedef CF_ENUM(CFIndex, MACaptionAppearanceDomain) {
	kMACaptionAppearanceDomainDefault = 0,
	kMACaptionAppearanceDomainUser = 1,
};

typedef CF_ENUM(CFIndex, MACaptionAppearanceDisplayType) {
	kMACaptionAppearanceDisplayTypeForcedOnly = 0,
	kMACaptionAppearanceDisplayTypeAutomatic = 1,
	kMACaptionAppearanceDisplayTypeAlwaysOn = 2,
};

// How a returned preference value is to be used: it overrides content-defined values,
// or it applies only where the content defines nothing.
typedef CF_ENUM(CFIndex, MACaptionAppearanceBehavior) {
	kMACaptionAppearanceBehaviorUseValue = 0,
	kMACaptionAppearanceBehaviorUseContentIfAvailable = 1,
};

typedef CF_ENUM(CFIndex, MACaptionAppearanceFontStyle) {
	kMACaptionAppearanceFontStyleDefault = 0,
	kMACaptionAppearanceFontStyleMonospacedWithSerif = 1,
	kMACaptionAppearanceFontStyleProportionalWithSerif = 2,
	kMACaptionAppearanceFontStyleMonospacedWithoutSerif = 3,
	kMACaptionAppearanceFontStyleProportionalWithoutSerif = 4,
	kMACaptionAppearanceFontStyleCasual = 5,
	kMACaptionAppearanceFontStyleCursive = 6,
	kMACaptionAppearanceFontStyleSmallCapital = 7,
};

typedef CF_ENUM(CFIndex, MACaptionAppearanceTextEdgeStyle) {
	kMACaptionAppearanceTextEdgeStyleUndefined = 0,
	kMACaptionAppearanceTextEdgeStyleNone = 1,
	kMACaptionAppearanceTextEdgeStyleRaised = 2,
	kMACaptionAppearanceTextEdgeStyleDepressed = 3,
	kMACaptionAppearanceTextEdgeStyleUniform = 4,
	kMACaptionAppearanceTextEdgeStyleDropShadow = 5,
};

// Media characteristics of legible tracks made for accessibility.
MA_EXPORT CFStringRef const MAMediaCharacteristicDescribesMusicAndSoundForAccessibility;
MA_EXPORT CFStringRef const MAMediaCharacteristicTranscribesSpokenDialogForAccessibility;

// Informs accessibility products which captions are on screen (an empty array: none).
MA_EXPORT void MACaptionAppearanceDidDisplayCaptions(CFArrayRef strings);

// Language preferences.
MA_EXPORT bool MACaptionAppearanceAddSelectedLanguage(MACaptionAppearanceDomain domain, CFStringRef language);
MA_EXPORT CFArrayRef MACaptionAppearanceCopySelectedLanguages(MACaptionAppearanceDomain domain);

// General preferences.
MA_EXPORT MACaptionAppearanceDisplayType MACaptionAppearanceGetDisplayType(MACaptionAppearanceDomain domain);
MA_EXPORT void MACaptionAppearanceSetDisplayType(MACaptionAppearanceDomain domain, MACaptionAppearanceDisplayType displayType);
MA_EXPORT CFArrayRef MACaptionAppearanceCopyPreferredCaptioningMediaCharacteristics(MACaptionAppearanceDomain domain);
MA_EXPORT bool MACaptionAppearanceIsCustomized(MACaptionAppearanceDomain domain);

// Colour preferences. The colours are returned retained; behavior may be NULL.
MA_EXPORT CGColorRef MACaptionAppearanceCopyForegroundColor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT CGColorRef MACaptionAppearanceCopyBackgroundColor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT CGColorRef MACaptionAppearanceCopyWindowColor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT CGFloat MACaptionAppearanceGetForegroundOpacity(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT CGFloat MACaptionAppearanceGetBackgroundOpacity(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT CGFloat MACaptionAppearanceGetWindowOpacity(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT CGFloat MACaptionAppearanceGetWindowRoundedCornerRadius(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);

// Text preferences.
MA_EXPORT CTFontDescriptorRef MACaptionAppearanceCopyFontDescriptorForStyle(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior, MACaptionAppearanceFontStyle fontStyle);
MA_EXPORT CGFloat MACaptionAppearanceGetRelativeCharacterSize(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);
MA_EXPORT MACaptionAppearanceTextEdgeStyle MACaptionAppearanceGetTextEdgeStyle(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior *__nullable behavior);

// Profiles. A profile ID is an opaque string; names are for display.
MA_EXPORT CFArrayRef MACaptionAppearanceCopyProfileIDs(void) CF_RETURNS_RETAINED;
MA_EXPORT void MACaptionAppearanceSetActiveProfileID(CFStringRef profileID);
MA_EXPORT CFStringRef MACaptionAppearanceCopyActiveProfileID(void) CF_RETURNS_RETAINED;
MA_EXPORT CFStringRef __nullable MACaptionAppearanceCopyProfileName(CFStringRef profileID) CF_RETURNS_RETAINED;
MA_EXPORT void MACaptionAppearanceExecuteBlockForProfileID(CFStringRef profileID, void (^aBlock)(void));

CF_ASSUME_NONNULL_END
MA_EXTERN_C_END

#endif
