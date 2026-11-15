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

#include <CarbonCore/Script.h>
#include <CarbonCore/UnicodeUtilities.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <string.h>
#include <mutex>
#include <vector>
#include <array>
#include <string>

// ---- Region code -----------------------------------------------------------------------------

struct RegionEntry
{
	const char* language;
	const char* country;
	long region;
};

// Locales of the region codes, as listed beside them in the public header.
static const RegionEntry kRegions[] = {
	{ "en", "US", verUS }, { "fr", "FR", verFrance }, { "en", "GB", verBritain }, { "de", "DE", verGermany },
	{ "it", "IT", verItaly }, { "nl", "NL", verNetherlands }, { "nl", "BE", verFlemish }, { "sv", "SE", verSweden },
	{ "es", "ES", verSpain }, { "da", "DK", verDenmark }, { "pt", "PT", verPortugal }, { "fr", "CA", verFrCanada },
	{ "nb", "NO", verNorway }, { "no", "NO", verNorway }, { "he", "IL", verIsrael }, { "ja", "JP", verJapan },
	{ "en", "AU", verAustralia }, { "fi", "FI", verFinland }, { "fr", "CH", verFrSwiss }, { "de", "CH", verGrSwiss },
	{ "el", "GR", verGreece }, { "is", "IS", verIceland }, { "mt", "MT", verMalta }, { "tr", "TR", verTurkey },
	{ "it", "CH", verItalianSwiss }, { "ro", "RO", verRomania }, { "lt", "LT", verLithuania }, { "pl", "PL", verPoland },
	{ "hu", "HU", verHungary }, { "et", "EE", verEstonia }, { "lv", "LV", verLatvia }, { "fa", "IR", verIran },
	{ "ru", "RU", verRussia }, { "ko", "KR", verKorea }, { "zh", "CN", verChina }, { "zh", "TW", verTaiwan },
	{ "th", "TH", verThailand }, { "cs", "CZ", verCzech }, { "sk", "SK", verSlovak }, { "uk", "UA", verUkraine },
	{ "sl", "SI", verSlovenian }, { "hr", "HR", verCroatia }, { "pt", "BR", verBrazil }, { "bg", "BG", verBulgaria },
	{ "ca", "ES", verCatalonia }, { "en", "CA", verEngCanada },
};

static long currentRegionCode()
{
	long region = verUS;
	CFLocaleRef locale = CFLocaleCopyCurrent();
	if (!locale)
		return region;

	char language[16] = "", country[16] = "";
	CFStringRef value = (CFStringRef) CFLocaleGetValue(locale, kCFLocaleLanguageCode);
	if (value)
		CFStringGetCString(value, language, sizeof(language), kCFStringEncodingUTF8);
	value = (CFStringRef) CFLocaleGetValue(locale, kCFLocaleCountryCode);
	if (value)
		CFStringGetCString(value, country, sizeof(country), kCFStringEncodingUTF8);
	CFRelease(locale);

	for (const RegionEntry& entry : kRegions)
	{
		if (strcmp(entry.language, language) == 0 && strcmp(entry.country, country) == 0)
			return entry.region;
	}
	return region;
}

// ---- KCHR cache ------------------------------------------------------------------------------

// The layout of the block smKCHRCache points at is that of a 'KCHR' resource, with the counts in
// the byte order of the host: a version word, the table number for each of the 256 values of the
// modifier byte (the high byte of EventRecord.modifiers), the number of 128-entry character tables,
// the tables (Mac Roman character per virtual key code), and the number of dead-key records.
// Dead keys are not generated: the layouts Darling builds hold no dead-key states.
static const size_t kKCHRTableCount = 256;

static std::string buildKCHR(const UCKeyboardLayout* layout)
{
	std::vector<std::array<uint8_t, 128>> tables;
	uint8_t tableIndex[kKCHRTableCount];

	for (size_t modifiers = 0; modifiers < kKCHRTableCount; modifiers++)
	{
		std::array<uint8_t, 128> table;
		for (UInt16 key = 0; key < 128; key++)
		{
			UInt32 deadKeyState = 0;
			UniChar chars[4];
			UniCharCount length = 0;
			table[key] = 0;

			if (UCKeyTranslate(layout, key, kUCKeyActionDown, (UInt32) modifiers, 0, kUCKeyTranslateNoDeadKeysMask, &deadKeyState, 4, &length, chars) != noErr || length != 1)
				continue;

			if (chars[0] < 0x80)
				table[key] = (uint8_t) chars[0];
			else
			{
				CFStringRef str = CFStringCreateWithCharacters(nullptr, chars, 1);
				UInt8 byte = 0;
				CFIndex used = 0;
				if (str && CFStringGetBytes(str, CFRangeMake(0, 1), kCFStringEncodingMacRoman, 0, false, &byte, 1, &used) == 1 && used == 1)
					table[key] = byte;
				if (str)
					CFRelease(str);
			}
		}

		size_t found = 0;
		while (found < tables.size() && tables[found] != table)
			found++;
		if (found == tables.size())
			tables.push_back(table);
		tableIndex[modifiers] = (uint8_t) found;
	}

	std::string blob;
	const uint16_t version = 0;
	const uint16_t tableCount = (uint16_t) tables.size();
	const uint16_t deadKeyCount = 0;
	blob.append((const char*) &version, 2);
	blob.append((const char*) tableIndex, sizeof(tableIndex));
	blob.append((const char*) &tableCount, 2);
	for (const auto& table : tables)
		blob.append((const char*) table.data(), table.size());
	blob.append((const char*) &deadKeyCount, 2);
	return blob;
}

// The Text Input Sources API is in HIToolbox, which sits above CarbonCore. The current keyboard
// layout is looked up through it at run time when something asks for the KCHR cache; without it
// (no HIToolbox in the process, or no layout) there is no cache.
static const UCKeyboardLayout* copyCurrentUnicodeLayout(CFDataRef* holder)
{
	typedef CFTypeRef (*CopyLayoutFn)(void);
	typedef void* (*GetPropertyFn)(CFTypeRef, CFStringRef);

	CopyLayoutFn copyLayout = (CopyLayoutFn) dlsym(RTLD_DEFAULT, "TISCopyCurrentKeyboardLayoutInputSource");
	GetPropertyFn getProperty = (GetPropertyFn) dlsym(RTLD_DEFAULT, "TISGetInputSourceProperty");
	CFStringRef* layoutKey = (CFStringRef*) dlsym(RTLD_DEFAULT, "kTISPropertyUnicodeKeyLayoutData");
	if (!copyLayout || !getProperty || !layoutKey || !*layoutKey)
		return nullptr;

	CFTypeRef source = copyLayout();
	if (!source)
		return nullptr;

	CFDataRef data = (CFDataRef) getProperty(source, *layoutKey);
	if (data)
		*holder = (CFDataRef) CFRetain(data);
	CFRelease(source);
	return data ? (const UCKeyboardLayout*) CFDataGetBytePtr(*holder) : nullptr;
}

static std::mutex g_kchrLock;
// Blocks handed out stay valid for the life of the process, as the cache does on macOS; a new one
// is built only when the keyboard layout changes.
static std::vector<std::string*> g_kchrBlocks;
static std::string g_kchrSource;

static const void* kchrCache()
{
	CFDataRef holder = nullptr;
	const UCKeyboardLayout* layout = copyCurrentUnicodeLayout(&holder);
	if (!layout)
		return nullptr;

	std::string source((const char*) CFDataGetBytePtr(holder), CFDataGetLength(holder));
	CFRelease(holder);

	std::lock_guard<std::mutex> lock(g_kchrLock);
	if (g_kchrBlocks.empty() || source != g_kchrSource)
	{
		g_kchrBlocks.push_back(new std::string(buildKCHR((const UCKeyboardLayout*) source.data())));
		g_kchrSource = source;
	}
	return g_kchrBlocks.back()->data();
}

// QUIRK: the selectors other than the four of 64-bit headers (smSysScript, smKeyScript, smKCHRCache,
// smRegionCode) still answer: smVersion is 0x1000, smIntlForce and smKeyDisableState 255, and the
// selectors that report state (smMunged, smBidirect, smFontForce, smForced, smDefault, smPrint,
// smSysScript, smLastScript, smKeyScript, smSysRef, smKeyCache, smKeySwap, smGenFlags, smOverride,
// smCharPortion) 0; a selector that does not exist also gives 0. smEnabled and smDoubleByte are 1
// and 0, the answer for a system with only the Roman script (the Mac observed had more scripts
// enabled). Evidence: black-box observation on macOS 10.14.4, i386 and x86_64 (tier 2),
// vd-upstream/mac-probes/v665-sm.c.
// Test: tests/src/script_manager_variables.c
long GetScriptManagerVariable(short selector)
{
	switch (selector)
	{
		case smVersion: return 0x1000;
		case smEnabled: return 1;
		case smIntlForce: return 255;
		case smKeyDisableState: return 255;
		case smKCHRCache: return (long) kchrCache();
		case smRegionCode: return currentRegionCode();
		default: return 0;
	}
}
