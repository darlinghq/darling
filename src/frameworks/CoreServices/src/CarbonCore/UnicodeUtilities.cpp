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


#include <CarbonCore/UnicodeUtilities.h>
#include <unicode/coll.h>
#include <unicode/sortkey.h>
#include <string>
#include <cstring>
#include <iostream>
#include <CoreServices/UniChar.h>
#include <CarbonCore/MacErrors.h>

using namespace icu;

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

// Reads from the layout data. The data is a flat blob without a length, so fields are read with
// memcpy instead of through (possibly misaligned) struct pointers.
static inline uint16_t layoutU16(const uint8_t* p)
{
	uint16_t v;
	memcpy(&v, p, sizeof v);
	return v;
}

static inline uint32_t layoutU32(const uint8_t* p)
{
	uint32_t v;
	memcpy(&v, p, sizeof v);
	return v;
}

namespace
{
	// The text produced by one translation, with the documented too-small-buffer handling:
	// what fits is delivered and the error is reported at the end.
	struct KeyOutput
	{
		UniChar* buffer;
		UniCharCount capacity;
		UniCharCount length = 0;
		bool overflow = false;

		void add(UniChar c)
		{
			if (length < capacity)
				buffer[length++] = c;
			else
				overflow = true;
		}
	};

	struct LayoutView
	{
		const uint8_t* records;     // UCKeyStateRecordsIndex or null
		const uint8_t* terminators; // UCKeyStateTerminators or null
		const uint8_t* sequences;   // UCKeySequenceDataIndex or null
	};
}

// Appends the text for a UCKeyOutput / UCKeyCharSeq value that is not a state record index.
// 0xFFFE and 0xFFFF stand for no character. An index into the sequence data gives that sequence;
// without sequence data, or past its end, the value is a single character.
static void appendCharSeq(const LayoutView& layout, UInt16 value, KeyOutput& out)
{
	if (value == 0xFFFE || value == 0xFFFF)
		return;

	if ((value & kUCKeyOutputTestForIndexMask) == kUCKeyOutputSequenceIndexMask && layout.sequences != nullptr)
	{
		const unsigned index = value & kUCKeyOutputGetIndexMask;
		if (index < layoutU16(layout.sequences + 2))
		{
			const uint8_t* offsets = layout.sequences + 4;
			const uint16_t from = layoutU16(offsets + 2 * index);
			const uint16_t to = layoutU16(offsets + 2 * (index + 1));
			for (uint32_t at = from; at + 2 <= to; at += 2)
				out.add(layoutU16(layout.sequences + at));
			return;
		}
	}

	out.add(value);
}

static void appendTerminator(const LayoutView& layout, UInt32 state, KeyOutput& out)
{
	if (state == 0 || layout.terminators == nullptr)
		return;
	if (state - 1 < layoutU16(layout.terminators + 2))
		appendCharSeq(layout, layoutU16(layout.terminators + 4 + 2 * (state - 1)), out);
}

// Finds the entry of a state record for the pending state. On success it gives the entry's
// character and next state; the next state of a terminal entry is zero.
static bool findStateEntry(const uint8_t* record, UInt32 state, UInt16* charData, UInt16* nextState)
{
	const uint16_t count = layoutU16(record + 4);
	const uint16_t format = layoutU16(record + 6);
	const uint8_t* entries = record + 8;

	for (unsigned i = 0; i < count; i++)
	{
		if (format == kUCKeyStateEntryTerminalFormat)
		{
			const uint8_t* e = entries + 4 * i;
			if (layoutU16(e) == state)
			{
				*charData = layoutU16(e + 2);
				*nextState = 0;
				return true;
			}
		}
		else if (format == kUCKeyStateEntryRangeFormat)
		{
			const uint8_t* e = entries + 8 * i;
			const UInt32 start = layoutU16(e);
			const UInt32 range = e[2];
			if (state >= start && state <= start + range)
			{
				const UInt32 delta = (state - start) * e[3];
				UInt16 c = layoutU16(e + 4);
				UInt16 next = layoutU16(e + 6);
				if (c < 0xFFFE)
					c = (UInt16) (c + delta);
				if (next != 0)
					next = (UInt16) (next + delta);
				*charData = c;
				*nextState = next;
				return true;
			}
		}
		else
			return false;
	}
	return false;
}

// QUIRK: a keyboardType in 31..193 never selects a keyboard-type entry of the layout other than
// the first (default) one, even when an entry's range lists it. Types outside that interval
// select the entry whose range contains them, and the default entry serves every other type.
// Evidence: black-box observation of UCKeyTranslate on macOS 26.7.3 and 10.14.4 with hand-built
// layouts (tier 2); identical on both. Test: tests/src/uckeytranslate_layout.c
// (keyboard type selection).
static const UCKeyboardTypeHeader* selectKeyboardType(const UCKeyboardLayout* layout, UInt32 keyboardType)
{
	const UCKeyboardTypeHeader* list = layout->keyboardTypeList;
	if (keyboardType < 31 || keyboardType > 193)
	{
		for (UInt32 i = 1; i < layout->keyboardTypeCount; i++)
		{
			if (keyboardType >= list[i].keyboardTypeFirst && keyboardType <= list[i].keyboardTypeLast)
				return &list[i];
		}
	}
	return &list[0];
}

OSStatus UCKeyTranslate(const UCKeyboardLayout *keyLayoutPtr, UInt16 virtualKeyCode, UInt16 keyAction, UInt32 modifierKeyState, UInt32 keyboardType,
	OptionBits keyTranslateOptions, UInt32 *deadKeyState, UniCharCount maxStringLength, UniCharCount *actualStringLength, UniChar unicodeString[])
{
	if (!keyLayoutPtr || !deadKeyState || !actualStringLength || !unicodeString || keyAction > kUCKeyActionDisplay)
		return paramErr;

	*actualStringLength = 0;
	if (keyLayoutPtr->keyboardTypeCount == 0)
		return paramErr;

	const UCKeyboardTypeHeader* type = selectKeyboardType(keyLayoutPtr, keyboardType);
	const uint8_t* base = reinterpret_cast<const uint8_t*>(keyLayoutPtr);

	LayoutView layout;
	layout.records = type->keyStateRecordsIndexOffset ? base + type->keyStateRecordsIndexOffset : nullptr;
	layout.terminators = type->keyStateTerminatorsOffset ? base + type->keyStateTerminatorsOffset : nullptr;
	layout.sequences = type->keySequenceDataIndexOffset ? base + type->keySequenceDataIndexOffset : nullptr;

	// The caller passes the Carbon modifier flags shifted right by 8: the Command, Shift, Caps
	// Lock, Option and Control bits and their right-hand variants make one byte. Bits above the
	// byte are ignored. A byte at or past the table's count uses the default table.
	const uint8_t* modMaps = base + type->keyModifiersToTableNumOffset;
	const uint8_t* tableIndex = base + type->keyToCharTableIndexOffset;
	const UInt32 modifierByte = modifierKeyState & 0xFF;
	unsigned tableNumber = layoutU16(modMaps + 2);
	if (modifierByte < layoutU32(modMaps + 4))
		tableNumber = modMaps[8 + modifierByte];

	KeyOutput out;
	out.buffer = unicodeString;
	out.capacity = maxStringLength;

	// Only the low 16 bits of the dead-key state number the pending state.
	const UInt32 pending = *deadKeyState & 0xFFFF;
	UInt32 newState = pending;

	UInt16 value = 0xFFFE;
	if (virtualKeyCode < layoutU16(tableIndex + 2) && tableNumber < layoutU32(tableIndex + 4))
	{
		const uint8_t* table = base + layoutU32(tableIndex + 8 + 4 * tableNumber);
		value = layoutU16(table + 2 * virtualKeyCode);
	}

	const bool noDeadKeys = (keyTranslateOptions & kUCKeyTranslateNoDeadKeysMask) != 0;

	if ((value & kUCKeyOutputTestForIndexMask) == kUCKeyOutputStateIndexMask)
	{
		// The key's output depends on the pending state.
		if (layout.records == nullptr || (value & kUCKeyOutputGetIndexMask) >= layoutU16(layout.records + 2))
			return paramErr;

		const uint8_t* record = base + layoutU32(layout.records + 4 + 4 * (value & kUCKeyOutputGetIndexMask));
		const UInt16 zeroChar = layoutU16(record);
		const UInt16 zeroNext = layoutU16(record + 2);
		UInt16 entryChar = 0, entryNext = 0;
		const bool hasEntry = pending != 0 && findStateEntry(record, pending, &entryChar, &entryNext);

		// QUIRK: besides the key-down handling in the header, the key actions behave as follows
		// (observed with hand-built layouts). Key up gives the text of a key-down with no new
		// state and an empty state afterwards, and a dead key (a record with a next state) that
		// is pressed with no state pending gives its character followed by the terminator of
		// its next state instead of entering that state, as with kUCKeyTranslateNoDeadKeysMask.
		// Auto-key handles the record as if no state were pending. Display does too once a
		// state is pending; with none pending a dead key also shows its terminator, and the next
		// state is entered; a matching entry sets the state to 0x80000000.
		// Evidence: black-box observation on macOS 26.7.3 and 10.14.4 (tier 2).
		// Test: tests/src/uckeytranslate_layout.c (key actions).
		switch (keyAction)
		{
			case kUCKeyActionDown:
			case kUCKeyActionUp:
			{
				const bool up = keyAction == kUCKeyActionUp;
				if (hasEntry)
				{
					appendCharSeq(layout, entryChar, out);
					// QUIRK: the state after an entry hit keeps the matched state in bits 16-31
					// on top of the next state (so it is non-zero after a completed
					// combination). Evidence: black-box observation on macOS 26.7.3 and 10.14.4
					// (tier 2). Test: tests/src/uckeytranslate_layout.c (state after entry).
					newState = up ? 0 : ((pending << 16) | (noDeadKeys ? 0 : entryNext));
				}
				else
				{
					if (pending != 0)
						appendTerminator(layout, pending, out);
					appendCharSeq(layout, zeroChar, out);
					if (up || noDeadKeys)
					{
						if (pending == 0)
							appendTerminator(layout, zeroNext, out);
						newState = 0;
					}
					else
						newState = zeroNext;
				}
				break;
			}
			case kUCKeyActionAutoKey:
				appendCharSeq(layout, zeroChar, out);
				newState = noDeadKeys ? 0 : zeroNext;
				break;
			default: // kUCKeyActionDisplay
				if (hasEntry)
				{
					appendCharSeq(layout, entryChar, out);
					newState = 0x80000000;
				}
				else
				{
					appendCharSeq(layout, zeroChar, out);
					if (pending == 0)
						appendTerminator(layout, zeroNext, out);
					newState = noDeadKeys ? 0 : zeroNext;
				}
				break;
		}
	}
	else
	{
		// A key with a fixed output. Key down and up first give the terminator of the pending
		// state; auto-key and display do not. Auto-key keeps the pending state.
		if ((keyAction == kUCKeyActionDown || keyAction == kUCKeyActionUp) && pending != 0)
			appendTerminator(layout, pending, out);
		appendCharSeq(layout, value, out);
		newState = keyAction == kUCKeyActionAutoKey ? *deadKeyState : 0;
	}

	*deadKeyState = newState;
	*actualStringLength = out.length;
	return out.overflow ? kUCOutputBufferTooSmall : noErr;
}
