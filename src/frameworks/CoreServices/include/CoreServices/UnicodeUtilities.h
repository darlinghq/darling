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

#ifndef UNICODEUTILITIES_H
#define UNICODEUTILITIES_H
#include "MacTypes.h"
#include "MacLocales.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CollatorRef;
enum //: uint32_t
{
	kUCCollateComposeInsensitiveMask = 0x2,
	kUCCollateWidthInsensitiveMask = 0x4,
	kUCCollateCaseInsensitiveMask = 0x8,
	kUCCollateDiacritInsensitiveMask = 0x10,
	kUCCollatePunctuationSignificantMask = 0x8000,
	kUCCollateDigitsOverrideMask = 0x10000,
	kUCCollateDigitsAsNumberMask = 0x20000
};

 
OSStatus UCCreateCollator(LocaleRef locale, LocaleOperationVariant opVariant, uint32_t options, CollatorRef* collator);

OSStatus UCGetCollationKey(CollatorRef collator, const UniChar * text, unsigned long textlen, unsigned long maxKeySize, unsigned long* actualKeySize, uint32_t* collationKey);

OSStatus UCCompareCollationKeys(const uint32_t* key1, unsigned long key1len, const uint32_t* key2, unsigned long key2len, Boolean* equiv, int32_t* order);

OSStatus UCCompareText(CollatorRef collator, const UniChar* text1, unsigned long text1len, const UniChar* text2, unsigned long text2len, Boolean* equiv, int32_t* order);

OSStatus UCDisposeCollator(CollatorRef* collator);

OSStatus UCCompareTextDefault(uint32_t options, const UniChar* text1, unsigned long text1len, const UniChar* text2, unsigned long text2len, Boolean* equiv, int32_t* order);

OSStatus UCCompareTextNoLocale(uint32_t options, const UniChar* text1, unsigned long text1len, const UniChar* text2, unsigned long text2len, Boolean* equiv, int32_t* order);

typedef UInt16 UCKeyOutput;

enum {
  kUCKeyLayoutHeaderFormat      = 0x1002,
  kUCKeyLayoutFeatureInfoFormat = 0x2001,
  kUCKeyModifiersToTableNumFormat = 0x3001,
  kUCKeyToCharTableIndexFormat  = 0x4001,
  kUCKeyStateRecordsIndexFormat = 0x5001,
  kUCKeyStateTerminatorsFormat  = 0x6001,
  kUCKeySequenceDataIndexFormat = 0x7001
};

struct UCKeyboardTypeHeader {
  UInt32              keyboardTypeFirst;
  UInt32              keyboardTypeLast;
  UInt32              keyModifiersToTableNumOffset;
  UInt32              keyToCharTableIndexOffset;
  UInt32              keyStateRecordsIndexOffset;
  UInt32              keyStateTerminatorsOffset;
  UInt32              keySequenceDataIndexOffset;
};
typedef struct UCKeyboardTypeHeader     UCKeyboardTypeHeader;
struct UCKeyboardLayout {
  UInt16              keyLayoutHeaderFormat;
  UInt16              keyLayoutDataVersion;
  UInt32              keyLayoutFeatureInfoOffset;
  UInt32              keyboardTypeCount;
  UCKeyboardTypeHeader  keyboardTypeList[1];
};
typedef struct UCKeyboardLayout         UCKeyboardLayout;

struct UCKeyLayoutFeatureInfo {
  UInt16              keyLayoutFeatureInfoFormat;
  UInt16              reserved;
  UInt32              maxOutputStringLength;
};
typedef struct UCKeyLayoutFeatureInfo   UCKeyLayoutFeatureInfo;

struct UCKeyModifiersToTableNum {
  UInt16              keyModifiersToTableNumFormat;
  UInt16              defaultTableNum;
  UInt32              modifiersCount;
  UInt8               tableNum[1];
};
typedef struct UCKeyModifiersToTableNum UCKeyModifiersToTableNum;

struct UCKeyToCharTableIndex {
  UInt16              keyToCharTableIndexFormat;
  UInt16              keyToCharTableSize;
  UInt32              keyToCharTableCount;
  UInt32              keyToCharTableOffsets[1];
};
typedef struct UCKeyToCharTableIndex    UCKeyToCharTableIndex;

extern OSStatus  UCKeyTranslate(
  const UCKeyboardLayout *  keyLayoutPtr,
  UInt16                    virtualKeyCode,
  UInt16                    keyAction,
  UInt32                    modifierKeyState,
  UInt32                    keyboardType,
  OptionBits                keyTranslateOptions,
  UInt32 *                  deadKeyState,
  UniCharCount              maxStringLength,
  UniCharCount *            actualStringLength,
  UniChar                   unicodeString[]);


#ifdef __cplusplus
}
#endif

#endif
