#include "MacLocales.h"
#include "MacErrors.h"
#include "libSystem/libc/darwin_errno_codes.h"
#include "libSystem/libc/errno.h"
#include <unicode/locid.h>
#include <unicode/coll.h>
#include <cstring>
#include <algorithm>
#include <iconv.h>
#include <errno.h>
#include <cassert>

static iconv_t g_icUtf32ToUtf16 = nullptr;

__attribute__((constructor)) void initConversions()
{
	g_icUtf32ToUtf16 = iconv_open("UTF-16", "UTF-32");
	assert(g_icUtf32ToUtf16 != iconv_t(-1));
}

__attribute__((destructor)) void exitConversions()
{
	iconv_close(g_icUtf32ToUtf16);
}

OSStatus LocaleRefFromLangOrRegionCode(LangCode langCode, RegionCode regionCode, LocaleRef* refOut)
{
	if (langCode == kTextLanguageDontCare || regionCode == kTextRegionDontCare)
	{
		*refOut = Locale::getDefault().getName();
		return 0;
	}

	// LocaleRef is a pointer to a struct
	// Instead of storing a pointer, we simply use 4 bytes of the pointer to save the lang code
	char lc[3], rc[3];
	lc[0] = char(langCode & 255);
	lc[1] = char(langCode >> 8);
	lc[2] = 0;
	rc[0] = char(regionCode & 255);
	rc[1] = char(regionCode >> 8);
	rc[2] = 0;

	Locale loc(lc, rc);
	if (loc.isBogus())
	{
		*refOut = 0;
		return makeOSStatus(DARWIN_ENOENT);
	}
	else
	{
		*refOut = loc.getName();
		return 0;
	}
}

OSStatus LocaleRefFromLocaleString(const char* str, LocaleRef* refOut)
{
	Locale loc(str);
	if (loc.isBogus())
	{
		*refOut = 0;
		return makeOSStatus(DARWIN_EINVAL);
	}
	else
	{
		*refOut = loc.getName();
		return 0;
	}
}

OSStatus LocaleRefGetPartString(LocaleRef ref, uint32_t partMask, unsigned long maxStringLen, char* stringOut)
{
	Locale loc(ref);
	char buffer[50] = "";

	if (loc.isBogus())
	{
		if (maxStringLen)
			*stringOut = 0;
		return makeOSStatus(DARWIN_EINVAL);
	}

	if (partMask & (kLocaleLanguageMask|kLocaleLanguageVariantMask))
		strcpy(buffer, loc.getLanguage());
	if (partMask & (kLocaleRegionMask|kLocaleRegionVariantMask))
	{
		strcat(buffer, "_");
		strcat(buffer, loc.getCountry());
	}
	if (partMask & (kLocaleScriptMask|kLocaleScriptVariantMask))
	{
		strcat(buffer, "@");
		strcat(buffer, loc.getScript());
	}

	strncpy(stringOut, buffer, maxStringLen-1);
	stringOut[maxStringLen-1] = 0;
	return 0;
}

OSStatus LocaleStringToLangAndRegionCodes(const char* name, LangCode* langCode, RegionCode* regionCode)
{
	Locale loc(name);
	if (loc.isBogus())
	{
		*langCode = kTextLanguageDontCare;
		*regionCode = kTextRegionDontCare;
		return makeOSStatus(DARWIN_ENOENT);
	}

	const char* lang = loc.getLanguage();
	const char* region = loc.getCountry();
	*langCode = LangCode(*lang) | (LangCode(lang[1]) << 8);
	*regionCode = RegionCode(*region) | (RegionCode(region[1]) << 8);

	return 0;
}

OSStatus LocaleOperationCountLocales(LocaleOperationClass cls, unsigned long* count)
{
	if (cls != 'ucol')
		*count = 0;
	else
	{
		int32_t cc;
		Collator::getAvailableLocales(cc);
		*count = cc;
	}
	return 0;
}

OSStatus LocaleOperationGetLocales(LocaleOperationClass cls, unsigned long max, unsigned long* countOut, LocaleAndVariant* out)
{
	if (cls != 'ucol')
		*countOut = 0;
	else
	{
		int32_t cc;
		const Locale* locales = Collator::getAvailableLocales(cc);

		for (int i = 0; i < cc && i < max; i++)
		{
			out[i].ref = locales[i].getName();
			out[i].variant = 0;
		}
		*countOut = std::min<int32_t>(cc, max);
	}
	return 0;
}

OSStatus LocaleGetName(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask,
	LocaleRef refDisplay, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName)
{
	Locale loc(ref);
	Locale locDisplay(refDisplay);

	// variant and nameMask currently ignored
	
	UnicodeString str, str2;
	loc.getDisplayLanguage(locDisplay, str);
	str += " (";
	loc.getDisplayCountry(locDisplay, str2);
	str += str2;
	str += ")";

	const UChar* buf = str.getTerminatedBuffer();
	size_t inLen = (str.length()+1) * sizeof(UChar);
	size_t outLen = maxLen * sizeof(Utf16Char);
	const char* inbuf = reinterpret_cast<const char*>(buf);
	char* outbuf = reinterpret_cast<char*>(displayName);
	size_t r = iconv(g_icUtf32ToUtf16, const_cast<char**>(&inbuf), &inLen, &outbuf, &outLen);

	if (r == size_t(-1))
	{
		if (errno == E2BIG)
		{
			displayName[maxLen-1] = 0;
			r = maxLen;
		}
		else
		{
			r = 0;
			*displayName = 0;
			return makeOSStatus(errnoLinuxToDarwin(errno));
		}
	}
	
	*lenOut = r;
	return 0;
}

OSStatus LocaleCountNames(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, unsigned long* countOut)
{
	return unimpErr;
}

OSStatus LocaleGetIndName(LocaleRef ref, LocaleOperationVariant variant, uint32_t nameMask, unsigned long index, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName, LocaleRef* displayLocale)
{
	return unimpErr;
}

OSStatus LocaleGetRegionLanguageName(RegionCode regionCode, char name[256])
{
	return unimpErr;
}

OSStatus LocaleOperationGetName(LocaleOperationClass cls, LocaleRef ref, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName)
{
	return unimpErr;
}

OSStatus LocaleOperationCountNames(LocaleOperationClass cls, unsigned long* count)
{
	return unimpErr;
}

OSStatus LocaleOperationGetIndName(LocaleOperationClass cls, unsigned long index, unsigned long maxLen, unsigned long* lenOut, Utf16Char* displayName, LocaleRef* displayLocale)
{
	return unimpErr;
}


