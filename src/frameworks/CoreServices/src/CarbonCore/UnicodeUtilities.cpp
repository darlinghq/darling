/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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


#include <CoreServices/UnicodeUtilities.h>
#include <unicode/coll.h>
#include <unicode/sortkey.h>
#include <string>
#include <cstring>
#include <iostream>
#include <CoreServices/UniChar.h>
#include <CoreServices/MacErrors.h>

namespace Darling
{
	int getLocaleUID(const std::string& str);
	const char* getLocaleString(int uid);
}

static UColAttributeValue optsToColAttr(uint32_t options)
{
	UColAttributeValue strength = UCOL_DEFAULT_STRENGTH;
	
	if (options & kUCCollateComposeInsensitiveMask)
		strength = UCOL_QUATERNARY;
	//if (options & kUCCollateWidthInsensitiveMask)
	if (options & kUCCollateCaseInsensitiveMask)
		strength = UCOL_SECONDARY;
	if (options & kUCCollateDiacritInsensitiveMask)
		strength = UCOL_PRIMARY;
	
	return strength;
}

OSStatus UCCreateCollator(LocaleRef locale, LocaleOperationVariant opVariant, uint32_t options, CollatorRef* collator)
{
	UErrorCode code = U_ZERO_ERROR;
	const char* localeStr = Darling::getLocaleString(locale);
	Collator* c = Collator::createInstance(localeStr, code);
	
	*collator = c;
	
	if (!c)
		return paramErr;
	
	code = U_ZERO_ERROR;
	c->setAttribute(UCOL_STRENGTH, optsToColAttr(options), code);
	return noErr;
}

OSStatus UCGetCollationKey(CollatorRef collator, const UniChar* text, unsigned long textlen, unsigned long maxKeySize, unsigned long* actualKeySize, uint32_t* collationKey)
{
	if (!text || !actualKeySize || !collationKey || !collator)
		return paramErr;
	
	UnicodeString str1((const char*) text, textlen*2, UNICHAR_ENCODING);
	Collator* c = static_cast<Collator*>(collator);
	CollationKey key;
	UErrorCode code = U_ZERO_ERROR;
	int32_t count;
	const uint8_t* data;
	
	*actualKeySize = 0;
	c->getCollationKey(str1, key, code);
	
	if (code != U_ZERO_ERROR)
		return -1;
	
	data = key.getByteArray(count);
	if (count > sizeof(uint32_t)*maxKeySize)
		return kUCOutputBufferTooSmall;
	
	*actualKeySize = (count + sizeof(uint32_t) - 1) / sizeof(uint32_t);
	memset(collationKey, 0, *actualKeySize);
	memcpy(collationKey, data, count);
	
	return noErr;
}

OSStatus UCCompareCollationKeys(const uint32_t* key1, unsigned long key1len, const uint32_t* key2, unsigned long key2len, Boolean* equiv, int32_t* order)
{
	if (!equiv && !order)
		return paramErr;
	
	CollationKey ckey1((const uint8_t*)key1, key1len);
	CollationKey ckey2((const uint8_t*)key2, key2len);
	UCollationResult result;
	UErrorCode code = U_ZERO_ERROR;
	
	result = ckey1.compareTo(ckey2, code);
	
	if (code != U_ZERO_ERROR)
		return -1;
	
	if (equiv != nullptr)
		*equiv = result == UCOL_EQUAL;
	
	if (order != nullptr)
	{
		if (result == UCOL_GREATER)
			*order = 1;
		else if (result == UCOL_LESS)
			*order = -1;
		else
			*order = 0;
	}
	
	return noErr;
}

OSStatus UCCompareText(CollatorRef collator, const UniChar* text1, unsigned long text1len, const UniChar* text2, unsigned long text2len, Boolean* equiv, int32_t* order)
{
	if (!equiv && !order)
		return paramErr;
	if (!text1 || !text2 || !collator)
		return paramErr;
	
	UnicodeString str1((const char*) text1, text1len*2, UNICHAR_ENCODING);
	UnicodeString str2((const char*) text2, text2len*2, UNICHAR_ENCODING);
	Collator* c = static_cast<Collator*>(collator);
	UCollationResult result;
	UErrorCode code = U_ZERO_ERROR;
	
	result = c->compare(str1, str2, code);
	
	if (code != U_ZERO_ERROR)
		return -1;
	
	if (equiv != nullptr)
		*equiv = (result == UCOL_EQUAL);
	
	if (order != nullptr)
	{
		if (result == UCOL_GREATER)
			*order = 1;
		else if (result == UCOL_LESS)
			*order = -1;
		else
			*order = 0;
	}
	
	return noErr;
}

OSStatus UCDisposeCollator(CollatorRef* collator)
{
	delete static_cast<Collator*>(*collator);
	*collator = nullptr;
	return noErr;
}

OSStatus UCCompareTextDefault(uint32_t options, const UniChar* text1, unsigned long text1len, const UniChar* text2, unsigned long text2len, Boolean* equiv, int32_t* order)
{
	UErrorCode code = U_ZERO_ERROR;
	Collator* col = Collator::createInstance(code);
	OSStatus rv;
	
	code = U_ZERO_ERROR;
	col->setAttribute(UCOL_STRENGTH, optsToColAttr(options), code);
	rv = UCCompareText(col, text1, text1len, text2, text2len, equiv, order);
	
	delete col;
	return rv;
}

OSStatus UCCompareTextNoLocale(uint32_t options, const UniChar* text1, unsigned long text1len, const UniChar* text2, unsigned long text2len, Boolean* equiv, int32_t* order)
{
	UErrorCode code = U_ZERO_ERROR;
	Collator* col = Collator::createInstance(Locale::getRoot(), code); // is getRoot correct?
	OSStatus rv;
	
	code = U_ZERO_ERROR;
	col->setAttribute(UCOL_STRENGTH, optsToColAttr(options), code);
	rv = UCCompareText(col, text1, text1len, text2, text2len,  equiv, order);
	
	delete col;
	return rv;
}

#define shiftKeyBit 9
#define shiftKey (1 << shiftKeyBit)

OSStatus UCKeyTranslate(const UCKeyboardLayout *keyLayoutPtr, UInt16 virtualKeyCode, UInt16 keyAction, UInt32 modifierKeyState, UInt32 keyboardType,
	OptionBits keyTranslateOptions, UInt32 *deadKeyState, UniCharCount maxStringLength, UniCharCount *actualStringLength, UniChar unicodeString[])
{
	if (!actualStringLength)
		return paramErr;

	*actualStringLength = 0;
	if (!keyLayoutPtr || !unicodeString)
		return paramErr;

	const UCKeyModifiersToTableNum* modMaps = (UCKeyModifiersToTableNum*) (((char*) keyLayoutPtr) + keyLayoutPtr->keyboardTypeList[0].keyModifiersToTableNumOffset);
	const UCKeyToCharTableIndex* tableIndex = (UCKeyToCharTableIndex*) (((char*) keyLayoutPtr) + keyLayoutPtr->keyboardTypeList[0].keyToCharTableIndexOffset);
	uint8_t tableNumber = modMaps->defaultTableNum;

	if ((modifierKeyState >> 8) < modMaps->modifiersCount)
		tableNumber = modMaps->tableNum[(modifierKeyState >> 8)];

	UniChar c;
	if (virtualKeyCode >= tableIndex->keyToCharTableSize || tableNumber >= tableIndex->keyToCharTableCount)
		c = 0;
	else
	{
		const UCKeyOutput* table = (UCKeyOutput*) (((char*) keyLayoutPtr) + tableIndex->keyToCharTableOffsets[tableNumber]);
		c = table[virtualKeyCode];
	}

	*actualStringLength = 1;
	if (maxStringLength > 0)
	{
		unicodeString[0] = c;
	}

	// std::cout << "UCKeyTranslate() virtualKeyCode: " << virtualKeyCode << " -> UC " << unicodeString[0] << std::endl;

	return noErr;
}
