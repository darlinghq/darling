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


// Caption appearance preferences (MACaptionAppearance.h).
//
// Darling has no system-wide caption settings UI and the preference storage of the
// original is not documented, so no persisted preferences are read. The values
// returned are the defaults of each caption profile, as observed on macOS 26.7.3
// (test: tests/src/mediaaccessibility_caption_appearance.m); changes made through
// the Set functions apply to the current process only.

#include <MediaAccessibility/MediaAccessibility.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#include <pthread.h>

const CFStringRef kMACaptionAppearanceSettingsChangedNotification = CFSTR("com.apple.mediaaccessibility.captionAppearanceSettingsChanged");

// Same value, exported under a second name.
const CFStringRef kMAXCaptionAppearanceSettingsChangedNotification = CFSTR("com.apple.mediaaccessibility.captionAppearanceSettingsChanged");

CFStringRef const MAMediaCharacteristicDescribesMusicAndSoundForAccessibility = CFSTR("public.accessibility.describes-music-and-sound");
CFStringRef const MAMediaCharacteristicTranscribesSpokenDialogForAccessibility = CFSTR("public.accessibility.transcribes-spoken-dialog");

CFStringRef const MACaptionAppearanceSystemFontIdentifier = CFSTR(".AXAppleSystemFont");
CFStringRef const MACaptionAppearanceMediumSystemFontIdentifier = CFSTR(".AXAppleSystemFontBold");
CFStringRef const MACaptionAppearanceMonoSystemFontIdentifier = CFSTR(".AXAppleSystemFontMono");

// One preference value together with how it is to be used.
typedef struct {
	CGFloat value;
	MACaptionAppearanceBehavior behavior;
} PrefValue;

// What a profile yields in the user domain. Colours are fixed (white text on black).
typedef struct {
	CFStringRef profileID;
	CFStringRef name;
	MACaptionAppearanceBehavior foregroundColor, backgroundColor, windowColor;
	PrefValue foregroundOpacity, backgroundOpacity, windowOpacity;
	PrefValue cornerRadius, characterSize;
	CFIndex edgeStyle;
	MACaptionAppearanceBehavior edgeStyleBehavior;
	MACaptionAppearanceBehavior fontBehavior;
	bool outlined;
	const char* monoFontForAllStyles; // when set, every documented style resolves to this font
	const char* defaultStyleFont;     // when set, replaces the font of the default style
} Profile;

#define USE_VALUE kMACaptionAppearanceBehaviorUseValue
#define USE_CONTENT kMACaptionAppearanceBehaviorUseContentIfAvailable

// Order matches MACaptionAppearanceCopyProfileIDs().
static const Profile profiles[] = {
	{ CFSTR("MACaptionProfile-Classic"), CFSTR("Classic"),
	  USE_CONTENT, USE_CONTENT, USE_CONTENT,
	  { 1, USE_CONTENT }, { 1, USE_CONTENT }, { 0, USE_VALUE },
	  { 0, USE_VALUE }, { 1, USE_CONTENT },
	  kMACaptionAppearanceTextEdgeStyleNone, USE_CONTENT, USE_CONTENT, false,
	  ".AppleSystemUIFontMonospaced-Regular", NULL },
	{ CFSTR("MACaptionProfile-LargeText"), CFSTR("Large Text"),
	  USE_CONTENT, USE_VALUE, USE_CONTENT,
	  { 1, USE_CONTENT }, { 0, USE_VALUE }, { 0.5, USE_VALUE },
	  { 10, USE_VALUE }, { 1.5, USE_VALUE },
	  kMACaptionAppearanceTextEdgeStyleNone, USE_CONTENT, USE_CONTENT, false,
	  NULL, ".AppleSystemUIFont" },
	{ CFSTR("MACaptionProfile-OutlineText"), CFSTR("Outline Text"),
	  USE_CONTENT, USE_VALUE, USE_CONTENT,
	  { 1, USE_CONTENT }, { 0, USE_VALUE }, { 0, USE_VALUE },
	  { 10, USE_VALUE }, { 1.5, USE_CONTENT },
	  kMACaptionAppearanceTextEdgeStyleUniform, USE_CONTENT, USE_CONTENT, true,
	  NULL, ".AppleSystemUIFont" },
	{ CFSTR("MACaptionProfile-Default"), CFSTR("Transparent Background"),
	  USE_CONTENT, USE_VALUE, USE_CONTENT,
	  { 1, USE_CONTENT }, { 0, USE_VALUE }, { 0.5, USE_VALUE },
	  { 8, USE_VALUE }, { 1, USE_CONTENT },
	  kMACaptionAppearanceTextEdgeStyleNone, USE_CONTENT, USE_CONTENT, false,
	  NULL, NULL },
};
#define PROFILE_COUNT (sizeof(profiles) / sizeof(profiles[0]))
#define DEFAULT_PROFILE 3

// The default domain is the same for every profile and never defers to content.
static const Profile systemDefaults = {
	NULL, NULL,
	USE_VALUE, USE_VALUE, USE_VALUE,
	{ 1, USE_VALUE }, { 0, USE_VALUE }, { 0.5, USE_VALUE },
	{ 8, USE_VALUE }, { 1, USE_VALUE },
	kMACaptionAppearanceTextEdgeStyleNone, USE_VALUE, USE_VALUE, false,
	NULL, NULL,
};

// Outlined text is stroked at this fraction of the point size, up to a limit.
#define STROKE_PER_POINT (7.0 / 72.0)
#define STROKE_MAX (455.0 / 72.0)

static const char* const styleFontNames[] = {
	".AppleSystemUIFontMedium",
	".AppleSystemUIFontMonospaced-Regular",
	"IowanOldStyle-Bold",
	"Menlo-Bold",
	"TrebuchetMS-Bold",
	"AvenirNext-Medium",
	".SavoyeLetPlainCC",
	"Copperplate-Bold",
};
#define STYLE_COUNT (sizeof(styleFontNames) / sizeof(styleFontNames[0]))
static const char* const unknownStyleFontName = ".AppleSystemUIFont";

static pthread_mutex_t stateLock = PTHREAD_MUTEX_INITIALIZER;
static size_t activeProfile = DEFAULT_PROFILE;
static CFIndex displayTypeOverride = -1;
static CFMutableArrayRef addedLanguages; // newest first

static pthread_once_t profileKeyOnce = PTHREAD_ONCE_INIT;
static pthread_key_t profileKey; // holds (index + 1) while ExecuteBlockForProfileID runs

static void makeProfileKey(void)
{
	pthread_key_create(&profileKey, NULL);
}

static bool findProfile(CFStringRef profileID, size_t* index)
{
	if (!profileID)
		return false;
	for (size_t i = 0; i < PROFILE_COUNT; i++)
	{
		if (CFEqual(profiles[i].profileID, profileID))
		{
			*index = i;
			return true;
		}
	}
	return false;
}

static const Profile* valuesForDomain(MACaptionAppearanceDomain domain)
{
	if (domain == kMACaptionAppearanceDomainDefault)
		return &systemDefaults;

	pthread_once(&profileKeyOnce, makeProfileKey);
	uintptr_t overridden = (uintptr_t) pthread_getspecific(profileKey);
	if (overridden != 0)
		return &profiles[overridden - 1];

	pthread_mutex_lock(&stateLock);
	size_t index = activeProfile;
	pthread_mutex_unlock(&stateLock);
	return &profiles[index];
}

static void postSettingsChanged(void)
{
	CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(),
		kMACaptionAppearanceSettingsChangedNotification, NULL, NULL, true);
}

static void report(MACaptionAppearanceBehavior* out, MACaptionAppearanceBehavior behavior)
{
	if (out)
		*out = behavior;
}

static CGColorRef copyBlackOrWhite(CGFloat gray, MACaptionAppearanceBehavior* out, MACaptionAppearanceBehavior behavior)
{
	report(out, behavior);

	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	if (!space)
		return NULL;
	const CGFloat components[4] = { gray, gray, gray, 1 };
	CGColorRef color = CGColorCreate(space, components);
	CGColorSpaceRelease(space);
	return color;
}

CGColorRef MACaptionAppearanceCopyForegroundColor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	const Profile* p = valuesForDomain(domain);
	return copyBlackOrWhite(1, behavior, p->foregroundColor);
}

CGColorRef MACaptionAppearanceCopyBackgroundColor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	const Profile* p = valuesForDomain(domain);
	return copyBlackOrWhite(0, behavior, p->backgroundColor);
}

CGColorRef MACaptionAppearanceCopyWindowColor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	const Profile* p = valuesForDomain(domain);
	return copyBlackOrWhite(0, behavior, p->windowColor);
}

static CGFloat getValue(PrefValue v, MACaptionAppearanceBehavior* out)
{
	report(out, v.behavior);
	return v.value;
}

CGFloat MACaptionAppearanceGetForegroundOpacity(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	return getValue(valuesForDomain(domain)->foregroundOpacity, behavior);
}

CGFloat MACaptionAppearanceGetBackgroundOpacity(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	return getValue(valuesForDomain(domain)->backgroundOpacity, behavior);
}

CGFloat MACaptionAppearanceGetWindowOpacity(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	return getValue(valuesForDomain(domain)->windowOpacity, behavior);
}

CGFloat MACaptionAppearanceGetWindowRoundedCornerRadius(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	return getValue(valuesForDomain(domain)->cornerRadius, behavior);
}

CGFloat MACaptionAppearanceGetRelativeCharacterSize(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	return getValue(valuesForDomain(domain)->characterSize, behavior);
}

MACaptionAppearanceTextEdgeStyle MACaptionAppearanceGetTextEdgeStyle(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior)
{
	const Profile* p = valuesForDomain(domain);
	report(behavior, p->edgeStyleBehavior);
	return (MACaptionAppearanceTextEdgeStyle) p->edgeStyle;
}

static CTFontDescriptorRef copyFontDescriptor(MACaptionAppearanceDomain domain, MACaptionAppearanceBehavior* behavior,
	MACaptionAppearanceFontStyle style)
{
	const Profile* p = valuesForDomain(domain);
	const char* fontName = unknownStyleFontName;
	MACaptionAppearanceBehavior fontBehavior = USE_VALUE;

	// Styles outside the documented ones resolve to the system font and always use the value.
	if (style >= 0 && (size_t) style < STYLE_COUNT)
	{
		fontName = styleFontNames[style];
		if (p->monoFontForAllStyles)
			fontName = p->monoFontForAllStyles;
		else if (style == kMACaptionAppearanceFontStyleDefault && p->defaultStyleFont)
			fontName = p->defaultStyleFont;
		fontBehavior = p->fontBehavior;
	}
	report(behavior, fontBehavior);

	CFStringRef name = CFStringCreateWithCString(NULL, fontName, kCFStringEncodingUTF8);
	CGFloat zero = 0;
	CFNumberRef strokeWidth = CFNumberCreate(NULL, sizeof(CGFloat) == sizeof(double) ? kCFNumberDoubleType : kCFNumberFloatType, &zero);
	CFTypeRef keys[2] = { kCTFontNameAttribute, CFSTR("MACaptionFontAttributeStrokeWidth") };
	CFTypeRef values[2] = { name, strokeWidth };
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, 2,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CTFontDescriptorRef descriptor = attributes ? CTFontDescriptorCreateWithAttributes(attributes) : NULL;

	if (attributes)
		CFRelease(attributes);
	if (strokeWidth)
		CFRelease(strokeWidth);
	if (name)
		CFRelease(name);
	return descriptor;
}

CTFontDescriptorRef MACaptionAppearanceCopyFontDescriptorForStyle(MACaptionAppearanceDomain domain,
	MACaptionAppearanceBehavior* behavior, MACaptionAppearanceFontStyle fontStyle)
{
	return copyFontDescriptor(domain, behavior, fontStyle);
}

// Used by WebKit; not declared in the public headers. The stroke width is the outline
// thickness, in points, to draw at the given point size (0 when the text is not outlined).
CTFontDescriptorRef MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(MACaptionAppearanceDomain domain,
	MACaptionAppearanceBehavior* behavior, MACaptionAppearanceFontStyle fontStyle, CFStringRef trackLanguage,
	CGFloat pointSize, CGFloat* strokeWidthPt)
{
	(void) trackLanguage; // no per-language font choices yet

	if (strokeWidthPt)
	{
		CGFloat width = 0;
		if (valuesForDomain(domain)->outlined)
		{
			width = pointSize * STROKE_PER_POINT;
			if (width > STROKE_MAX)
				width = STROKE_MAX;
			if (width < 0)
				width = 0;
		}
		*strokeWidthPt = width;
	}
	return copyFontDescriptor(domain, behavior, fontStyle);
}

MACaptionAppearanceDisplayType MACaptionAppearanceGetDisplayType(MACaptionAppearanceDomain domain)
{
	CFIndex type = kMACaptionAppearanceDisplayTypeAutomatic;

	if (domain != kMACaptionAppearanceDomainDefault)
	{
		pthread_mutex_lock(&stateLock);
		if (displayTypeOverride >= 0)
			type = displayTypeOverride;
		pthread_mutex_unlock(&stateLock);
	}
	return (MACaptionAppearanceDisplayType) type;
}

void MACaptionAppearanceSetDisplayType(MACaptionAppearanceDomain domain, MACaptionAppearanceDisplayType displayType)
{
	if (domain == kMACaptionAppearanceDomainDefault)
		return; // the system defaults do not change
	if (displayType < kMACaptionAppearanceDisplayTypeForcedOnly || displayType > kMACaptionAppearanceDisplayTypeAlwaysOn)
		return;

	pthread_mutex_lock(&stateLock);
	bool changed = displayTypeOverride != displayType;
	displayTypeOverride = displayType;
	pthread_mutex_unlock(&stateLock);

	if (changed)
		postSettingsChanged();
}

CFArrayRef MACaptionAppearanceCopyPreferredCaptioningMediaCharacteristics(MACaptionAppearanceDomain domain)
{
	(void) domain;
	return CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
}

bool MACaptionAppearanceIsCustomized(MACaptionAppearanceDomain domain)
{
	(void) domain; // only the built-in profiles exist, none of them user-edited
	return false;
}

static void appendUnique(CFMutableArrayRef array, CFStringRef value)
{
	if (!CFArrayContainsValue(array, CFRangeMake(0, CFArrayGetCount(array)), value))
		CFArrayAppendValue(array, value);
}

static CFStringRef copyCanonicalLanguage(CFStringRef language)
{
	if (!language || CFStringGetLength(language) == 0)
		return NULL;
	return CFLocaleCreateCanonicalLanguageIdentifierFromString(NULL, language);
}

bool MACaptionAppearanceAddSelectedLanguage(MACaptionAppearanceDomain domain, CFStringRef language)
{
	if (domain == kMACaptionAppearanceDomainDefault)
		return false; // the system defaults are not user-adjustable

	CFStringRef canonical = copyCanonicalLanguage(language);
	if (!canonical)
		return false;

	pthread_mutex_lock(&stateLock);
	if (!addedLanguages)
		addedLanguages = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// A language selected again moves to the front.
	CFIndex existing = CFArrayGetFirstIndexOfValue(addedLanguages, CFRangeMake(0, CFArrayGetCount(addedLanguages)), canonical);
	if (existing != kCFNotFound)
		CFArrayRemoveValueAtIndex(addedLanguages, existing);
	CFArrayInsertValueAtIndex(addedLanguages, 0, canonical);
	pthread_mutex_unlock(&stateLock);

	CFRelease(canonical);
	postSettingsChanged();
	return true;
}

CFArrayRef MACaptionAppearanceCopySelectedLanguages(MACaptionAppearanceDomain domain)
{
	CFMutableArrayRef result = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (domain != kMACaptionAppearanceDomainDefault)
	{
		pthread_mutex_lock(&stateLock);
		if (addedLanguages)
		{
			for (CFIndex i = 0; i < CFArrayGetCount(addedLanguages); i++)
				appendUnique(result, CFArrayGetValueAtIndex(addedLanguages, i));
		}
		pthread_mutex_unlock(&stateLock);
	}

	// Followed by the system languages, each with its bare language code behind it.
	CFArrayRef system = CFLocaleCopyPreferredLanguages();
	for (CFIndex i = 0; system && i < CFArrayGetCount(system); i++)
	{
		CFStringRef canonical = copyCanonicalLanguage(CFArrayGetValueAtIndex(system, i));
		if (!canonical)
			continue;
		appendUnique(result, canonical);

		CFDictionaryRef parts = CFLocaleCreateComponentsFromLocaleIdentifier(NULL, canonical);
		CFStringRef code = parts ? CFDictionaryGetValue(parts, kCFLocaleLanguageCode) : NULL;
		if (code)
			appendUnique(result, code);
		if (parts)
			CFRelease(parts);
		CFRelease(canonical);
	}
	if (system)
		CFRelease(system);
	return result;
}

void MACaptionAppearanceDidDisplayCaptions(CFArrayRef strings)
{
	// Nothing observes displayed captions in this implementation.
	(void) strings;
}

CFArrayRef MACaptionAppearanceCopyProfileIDs(void)
{
	CFMutableArrayRef ids = CFArrayCreateMutable(NULL, PROFILE_COUNT, &kCFTypeArrayCallBacks);
	for (size_t i = 0; i < PROFILE_COUNT; i++)
		CFArrayAppendValue(ids, profiles[i].profileID);
	return ids;
}

void MACaptionAppearanceSetActiveProfileID(CFStringRef profileID)
{
	size_t index;
	if (!findProfile(profileID, &index))
		return; // undefined by the contract; leave the selection alone

	pthread_mutex_lock(&stateLock);
	bool changed = activeProfile != index;
	activeProfile = index;
	pthread_mutex_unlock(&stateLock);

	if (changed)
		postSettingsChanged();
}

CFStringRef MACaptionAppearanceCopyActiveProfileID(void)
{
	pthread_mutex_lock(&stateLock);
	size_t index = activeProfile;
	pthread_mutex_unlock(&stateLock);
	return CFRetain(profiles[index].profileID);
}

CFStringRef MACaptionAppearanceCopyProfileName(CFStringRef profileID)
{
	size_t index;
	if (!findProfile(profileID, &index))
		return NULL;
	return CFRetain(profiles[index].name);
}

void MACaptionAppearanceExecuteBlockForProfileID(CFStringRef profileID, void (^aBlock)(void))
{
	size_t index;
	if (!aBlock)
		return;
	if (!findProfile(profileID, &index))
	{
		aBlock();
		return;
	}

	pthread_once(&profileKeyOnce, makeProfileKey);
	void* previous = pthread_getspecific(profileKey);
	pthread_setspecific(profileKey, (void*) (uintptr_t) (index + 1));
	aBlock();
	pthread_setspecific(profileKey, previous);
}
