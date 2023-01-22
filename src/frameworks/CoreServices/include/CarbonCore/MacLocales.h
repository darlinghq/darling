/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#ifndef MACLOCALES_H
#define MACLOCALES_H
#include <stdint.h>
#include <CarbonCore/MacErrors.h>

typedef int16_t RegionCode;
typedef int16_t LangCode;
typedef int16_t Utf16Char;

typedef int LocaleRef;
typedef uint32_t LocaleOperationClass;
typedef uint32_t LocaleOperationVariant;

struct LocaleAndVariant
{
	LocaleRef ref;
	LocaleOperationVariant variant;
};

#define kLocaleLanguageMask 0x1
#define kLocaleLanguageVariantMask 0x2
#define kLocaleScriptMask 0x4
#define kLocaleScriptVariantMask 0x8
#define kLocaleRegionMask 0x10
#define kLocaleRegionVariantMask 0x20
#define kLocaleAllPartsMask 0x3f

#define kLocaleNameMask 0x1
#define kLocaleOperationVariantNameMask 0x2
#define kLocaleAndVariantNameMask (kLocaleNameMask|kLocaleOperationVariantNameMask)

#define kTextLanguageDontCare -128
#define kTextScriptDontCare -128
#define kTextRegionDontCare -128

#ifdef __cplusplus
extern "C" {
#endif

OSStatus LocaleRefFromLangOrRegionCode(LangCode langCode, RegionCode regionCode, LocaleRef* refOut);
OSStatus LocaleRefFromLocaleString(const char* str, LocaleRef* refOut);
OSStatus LocaleRefGetPartString(LocaleRef ref, uint32_t partMask, unsigned long maxStringLen, char* stringOut);
OSStatus LocaleStringToLangAndRegionCodes(const char* name, LangCode* langCode, RegionCode* regionCode);
OSStatus LocaleOperationCountLocales(LocaleOperationClass cls, unsigned long* count);
OSStatus LocaleOperationGetLocales(LocaleOperationClass cls, unsigned long max, unsigned long* countOut, struct LocaleAndVariant* out);
OSStatus LocaleGetName(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, LocaleRef refDisplay, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName);
OSStatus LocaleCountNames(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, unsigned long* countOut);
OSStatus LocaleGetIndName(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, unsigned long index, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName, LocaleRef* displayLocale);
OSStatus LocaleGetRegionLanguageName(RegionCode regionCode, char name[256]);
OSStatus LocaleOperationGetName(LocaleOperationClass cls, LocaleRef ref, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName);
OSStatus LocaleOperationCountNames(LocaleOperationClass cls, unsigned long* count);
OSStatus LocaleOperationGetIndName(LocaleOperationClass cls, unsigned long index, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName, LocaleRef* displayLocale);

#ifdef __cplusplus
}
#endif

#endif

