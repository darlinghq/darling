	#ifndef MACLOCALES_H
#define MACLOCALES_H
#include "MacErrors.h"
#include <stdint.h>

typedef int16_t RegionCode;
typedef int16_t LangCode;
typedef int16_t Utf16Char;

typedef const char* LocaleRef;
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

extern "C"
{

OSStatus LocaleRefFromLangOrRegionCode(LangCode langCode, RegionCode regionCode, LocaleRef* refOut);
OSStatus LocaleRefFromLocaleString(const char* str, LocaleRef* refOut);
OSStatus LocaleRefGetPartString(LocaleRef ref, uint32_t partMask, unsigned long maxStringLen, char* stringOut);
OSStatus LocaleStringToLangAndRegionCodes(const char* name, LangCode* langCode, RegionCode* regionCode);
OSStatus LocaleOperationCountLocales(LocaleOperationClass cls, unsigned long* count);
OSStatus LocaleOperationGetLocales(LocaleOperationClass cls, unsigned long max, unsigned long* countOut, LocaleAndVariant* out);
OSStatus LocaleGetName(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, LocaleRef refDisplay, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName);
OSStatus LocaleCountNames(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, unsigned long* countOut);
OSStatus LocaleGetIndName(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, unsigned long index, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName, LocaleRef* displayLocale);
OSStatus LocaleGetRegionLanguageName(RegionCode regionCode, char name[256]);
OSStatus LocaleOperationGetName(LocaleOperationClass cls, LocaleRef ref, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName);
OSStatus LocaleOperationCountNames(LocaleOperationClass cls, unsigned long* count);
OSStatus LocaleOperationGetIndName(LocaleOperationClass cls, unsigned long index, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName, LocaleRef* displayLocale);

}

#endif

