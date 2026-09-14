/*
 This file is part of Darling.

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

// In-process Apple Event Descriptor Manager: descriptors, lists, records and Apple Events
// (parameters and attributes). Darling has no Apple Event transport, so sending reports that
// the target process could not be found.

#include <AE/AE.h>
#include <CoreFoundation/CoreFoundation.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define AE_STORAGE_MAGIC 0x41454453 // 'AEDS'

// Built-in descriptor types that AEDataModel.h does not declare.
#define kAETypeUInt16 'ushr'
#define kAETypeUInt64 'ucom'
#define kAETypeKeyword 'keyw'

// How a descriptor's contents are stored. This is tracked separately from the descriptor type,
// because a record keeps its structure when it is coerced to another type (e.g. 'obj ').
typedef enum AEKind {
	kAEKindData,
	kAEKindList,
	kAEKindRecord,
	kAEKindEvent,
} AEKind;

typedef struct AEItem {
	AEKeyword key;
	AEDesc desc;
} AEItem;

typedef struct AEStorage {
	uint32_t magic;
	AEKind kind;
	Size size;
	uint8_t* bytes;          // data for plain descriptors
	AEItem* items;           // elements (lists) or parameters (records, events)
	long itemCount;
	long itemCapacity;
	AEItem* attributes;      // Apple Event attributes
	long attributeCount;
	long attributeCapacity;
} AEStorage;

static AEStorage* storageOf(const AEDesc* desc)
{
	if (desc == NULL || desc->dataHandle == NULL)
		return NULL;
	AEStorage* storage = (AEStorage*) desc->dataHandle;
	return storage->magic == AE_STORAGE_MAGIC ? storage : NULL;
}

static AEKind kindForType(DescType type)
{
	switch (type) {
		case typeAEList: return kAEKindList;
		case typeAERecord: return kAEKindRecord;
		case typeAppleEvent: return kAEKindEvent;
		default: return kAEKindData;
	}
}

static Boolean isList(const AEStorage* storage)
{
	return storage != NULL && storage->kind != kAEKindData;
}

static Boolean isRecordLike(const AEStorage* storage)
{
	return storage != NULL && (storage->kind == kAEKindRecord || storage->kind == kAEKindEvent);
}

static Boolean isEvent(const AEStorage* storage)
{
	return storage != NULL && storage->kind == kAEKindEvent;
}

static OSErr newStorage(DescType type, AEKind kind, const void* dataPtr, Size dataSize, AEDesc* result)
{
	AEStorage* storage = calloc(1, sizeof(*storage));
	if (storage == NULL)
		return memFullErr;
	storage->magic = AE_STORAGE_MAGIC;
	storage->kind = kind;
	if (dataSize > 0) {
		storage->bytes = malloc(dataSize);
		if (storage->bytes == NULL) {
			free(storage);
			return memFullErr;
		}
		if (dataPtr)
			memcpy(storage->bytes, dataPtr, dataSize);
		else
			memset(storage->bytes, 0, dataSize);
		storage->size = dataSize;
	}
	result->descriptorType = type;
	result->dataHandle = (AEDataStorage) storage;
	return noErr;
}

static void freeItems(AEItem* items, long count)
{
	for (long i = 0; i < count; i++)
		AEDisposeDesc(&items[i].desc);
	free(items);
}

static OSErr appendItem(AEItem** items, long* count, long* capacity, AEKeyword key, const AEDesc* desc)
{
	if (*count == *capacity) {
		long newCapacity = *capacity ? *capacity * 2 : 4;
		AEItem* grown = realloc(*items, newCapacity * sizeof(AEItem));
		if (grown == NULL)
			return memFullErr;
		*items = grown;
		*capacity = newCapacity;
	}
	AEItem* item = &(*items)[*count];
	item->key = key;
	OSErr err = AEDuplicateDesc(desc, &item->desc);
	if (err == noErr)
		(*count)++;
	return err;
}

static AEItem* findItem(AEItem* items, long count, AEKeyword key)
{
	for (long i = 0; i < count; i++)
		if (items[i].key == key)
			return &items[i];
	return NULL;
}

// --- Built-in coercions ---

typedef struct AENumber {
	enum { kAENumberSigned, kAENumberUnsigned, kAENumberFloat } kind;
	int64_t s;
	uint64_t u;
	double d;
} AENumber;

#define READ_NUMBER(ctype, field, numberKind) \
	do { \
		ctype value; \
		if (size != sizeof(value)) \
			return false; \
		memcpy(&value, data, sizeof(value)); \
		number->kind = numberKind; \
		number->field = value; \
		return true; \
	} while (0)

// Reads a numeric or boolean descriptor's data in host byte order.
static Boolean readNumber(DescType type, const void* data, Size size, AENumber* number)
{
	switch (type) {
		case typeSInt16: READ_NUMBER(SInt16, s, kAENumberSigned);
		case kAETypeUInt16: READ_NUMBER(UInt16, s, kAENumberSigned);
		case typeSInt32: READ_NUMBER(SInt32, s, kAENumberSigned);
		case typeUInt32: READ_NUMBER(UInt32, s, kAENumberSigned);
		case typeSInt64: READ_NUMBER(SInt64, s, kAENumberSigned);
		case kAETypeUInt64: READ_NUMBER(UInt64, u, kAENumberUnsigned);
		case typeIEEE32BitFloatingPoint: READ_NUMBER(Float32, d, kAENumberFloat);
		case typeIEEE64BitFloatingPoint: READ_NUMBER(Float64, d, kAENumberFloat);
		case typeBoolean: READ_NUMBER(Boolean, s, kAENumberSigned);
		case typeTrue:
			number->kind = kAENumberSigned;
			number->s = 1;
			return true;
		case typeFalse:
			number->kind = kAENumberSigned;
			number->s = 0;
			return true;
		default:
			return false;
	}
}

#undef READ_NUMBER

// Converts a number to a signed integer in [min, max]; fails for fractions and out-of-range values.
static Boolean numberToInteger(const AENumber* number, int64_t min, int64_t max, int64_t* out)
{
	int64_t value;
	switch (number->kind) {
		case kAENumberSigned:
			value = number->s;
			break;
		case kAENumberUnsigned:
			if (number->u > (uint64_t) INT64_MAX)
				return false;
			value = (int64_t) number->u;
			break;
		default:
			if (!isfinite(number->d) || number->d != trunc(number->d) ||
				number->d < -9223372036854775808.0 || number->d >= 9223372036854775808.0)
				return false;
			value = (int64_t) number->d;
			break;
	}
	if (value < min || value > max)
		return false;
	*out = value;
	return true;
}

static Boolean numberToUInt64(const AENumber* number, uint64_t* out)
{
	switch (number->kind) {
		case kAENumberSigned:
			if (number->s < 0)
				return false;
			*out = (uint64_t) number->s;
			return true;
		case kAENumberUnsigned:
			*out = number->u;
			return true;
		default:
			if (!isfinite(number->d) || number->d != trunc(number->d) || number->d < 0 || number->d >= 18446744073709551616.0)
				return false;
			*out = (uint64_t) number->d;
			return true;
	}
}

static double numberToDouble(const AENumber* number)
{
	switch (number->kind) {
		case kAENumberSigned: return (double) number->s;
		case kAENumberUnsigned: return (double) number->u;
		default: return number->d;
	}
}

static OSErr writeNumber(const AENumber* number, DescType toType, AEDesc* result)
{
	int64_t integer;
	switch (toType) {
		case typeSInt16: {
			if (!numberToInteger(number, INT16_MIN, INT16_MAX, &integer))
				return errAECoercionFail;
			SInt16 value = (SInt16) integer;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case kAETypeUInt16: {
			if (!numberToInteger(number, 0, UINT16_MAX, &integer))
				return errAECoercionFail;
			UInt16 value = (UInt16) integer;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case typeSInt32: {
			if (!numberToInteger(number, INT32_MIN, INT32_MAX, &integer))
				return errAECoercionFail;
			SInt32 value = (SInt32) integer;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case typeUInt32: {
			if (!numberToInteger(number, 0, UINT32_MAX, &integer))
				return errAECoercionFail;
			UInt32 value = (UInt32) integer;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case typeSInt64: {
			if (!numberToInteger(number, INT64_MIN, INT64_MAX, &integer))
				return errAECoercionFail;
			SInt64 value = integer;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case kAETypeUInt64: {
			UInt64 value;
			if (!numberToUInt64(number, &value))
				return errAECoercionFail;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case typeIEEE32BitFloatingPoint: {
			double d = numberToDouble(number);
			if (isfinite(d) && fabs(d) > FLT_MAX)
				return errAECoercionFail;
			Float32 value = (Float32) d;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case typeIEEE64BitFloatingPoint: {
			Float64 value = numberToDouble(number);
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		case typeBoolean: {
			if (!numberToInteger(number, 0, 1, &integer))
				return errAECoercionFail;
			Boolean value = (Boolean) integer;
			return AECreateDesc(toType, &value, sizeof(value), result);
		}
		default:
			return errAECoercionFail;
	}
}

static Boolean textEncodingForType(DescType type, CFStringEncoding* encoding)
{
	switch (type) {
		case typeUTF8Text:
			*encoding = kCFStringEncodingUTF8;
			return true;
		case typeUnicodeText: // UTF-16 in host byte order, without a byte order mark
#ifdef __BIG_ENDIAN__
			*encoding = kCFStringEncodingUTF16BE;
#else
			*encoding = kCFStringEncodingUTF16LE;
#endif
			return true;
		case typeChar:
			*encoding = CFStringGetSystemEncoding();
			return true;
		default:
			return false;
	}
}

static OSErr coerceText(CFStringEncoding fromEncoding, const void* data, Size size, DescType toType,
	CFStringEncoding toEncoding, AEDesc* result)
{
	CFStringRef string = CFStringCreateWithBytes(kCFAllocatorDefault, size > 0 ? data : (const UInt8*) "", size,
		fromEncoding, false);
	if (string == NULL)
		return errAECoercionFail;

	CFRange range = CFRangeMake(0, CFStringGetLength(string));
	CFIndex needed = 0;
	OSErr err = errAECoercionFail;
	// A loss byte of 0 makes characters that the target encoding lacks fail the coercion.
	if (CFStringGetBytes(string, range, toEncoding, 0, false, NULL, 0, &needed) == range.length) {
		UInt8* bytes = malloc(needed > 0 ? needed : 1);
		if (bytes == NULL) {
			err = memFullErr;
		} else {
			CFStringGetBytes(string, range, toEncoding, 0, false, bytes, needed, &needed);
			err = AECreateDesc(toType, bytes, needed, result);
			free(bytes);
		}
	}
	CFRelease(string);
	return err;
}

static Boolean isTypeCode(DescType type)
{
	return type == typeType || type == typeEnumerated || type == kAETypeKeyword;
}

static OSErr coerceData(DescType fromType, const void* data, Size size, DescType toType, AEDesc* result)
{
	AENumber number;
	if (readNumber(fromType, data, size, &number))
		return writeNumber(&number, toType, result);

	CFStringEncoding fromEncoding, toEncoding;
	if (textEncodingForType(fromType, &fromEncoding) && textEncodingForType(toType, &toEncoding))
		return coerceText(fromEncoding, data, size, toType, toEncoding, result);

	if (isTypeCode(fromType) && isTypeCode(toType) && size == sizeof(FourCharCode))
		return AECreateDesc(toType, data, size, result);

	return errAECoercionFail;
}

static OSErr coerceDesc(const AEDesc* desc, DescType desiredType, AEDesc* result)
{
	AEInitializeDesc(result);
	if (desiredType == typeWildCard || desiredType == desc->descriptorType)
		return AEDuplicateDesc(desc, result);

	AEStorage* storage = storageOf(desc);
	if (storage != NULL && storage->kind == kAEKindRecord) {
		// A record can be coerced to any type other than a list or an Apple Event, and stays a record.
		if (desiredType == typeAEList || desiredType == typeAppleEvent)
			return errAECoercionFail;
		OSErr err = AEDuplicateDesc(desc, result);
		if (err == noErr)
			result->descriptorType = desiredType;
		return err;
	}
	if (storage == NULL || storage->kind != kAEKindData)
		return errAECoercionFail;
	return coerceData(desc->descriptorType, storage->bytes, storage->size, desiredType, result);
}

// Shared tail of the Get*Ptr functions: coerce, then copy at most maximumSize bytes.
static OSErr copyCoercedData(const AEDesc* item, DescType desiredType, DescType* typeCode, void* dataPtr,
	Size maximumSize, Size* actualSize)
{
	AEDesc desc;
	OSErr err = coerceDesc(item, desiredType, &desc);
	if (err != noErr)
		return err;
	if (typeCode)
		*typeCode = desc.descriptorType;
	if (actualSize)
		*actualSize = AEGetDescDataSize(&desc);
	if (dataPtr)
		err = AEGetDescData(&desc, dataPtr, maximumSize);
	AEDisposeDesc(&desc);
	return err;
}

// --- Descriptors ---

void AEInitializeDesc(AEDesc* desc)
{
	if (desc) {
		desc->descriptorType = typeNull;
		desc->dataHandle = NULL;
	}
}

OSErr AECreateDesc(DescType typeCode, const void* dataPtr, Size dataSize, AEDesc* result)
{
	if (result == NULL || dataSize < 0)
		return paramErr;
	AEInitializeDesc(result);
	if (typeCode == typeNull && dataSize == 0)
		return noErr;
	return newStorage(typeCode, kindForType(typeCode), dataPtr, dataSize, result);
}

OSErr AEDisposeDesc(AEDesc* desc)
{
	if (desc == NULL)
		return paramErr;
	AEStorage* storage = storageOf(desc);
	if (storage) {
		free(storage->bytes);
		freeItems(storage->items, storage->itemCount);
		freeItems(storage->attributes, storage->attributeCount);
		storage->magic = 0;
		free(storage);
	}
	AEInitializeDesc(desc);
	return noErr;
}

OSErr AEDuplicateDesc(const AEDesc* desc, AEDesc* result)
{
	if (desc == NULL || result == NULL)
		return paramErr;
	AEStorage* source = storageOf(desc);
	AEDesc copy;
	AEInitializeDesc(&copy);
	if (source == NULL) {
		copy.descriptorType = desc->descriptorType;
		*result = copy;
		return noErr;
	}

	OSErr err = newStorage(desc->descriptorType, source->kind, source->bytes, source->size, &copy);
	if (err != noErr)
		return err;
	AEStorage* target = storageOf(&copy);
	for (long i = 0; i < source->itemCount && err == noErr; i++)
		err = appendItem(&target->items, &target->itemCount, &target->itemCapacity, source->items[i].key, &source->items[i].desc);
	for (long i = 0; i < source->attributeCount && err == noErr; i++)
		err = appendItem(&target->attributes, &target->attributeCount, &target->attributeCapacity, source->attributes[i].key, &source->attributes[i].desc);
	if (err != noErr) {
		AEDisposeDesc(&copy);
		return err;
	}
	*result = copy;
	return noErr;
}

Size AEGetDescDataSize(const AEDesc* desc)
{
	AEStorage* storage = storageOf(desc);
	return (storage && !isList(storage)) ? storage->size : 0;
}

OSErr AEGetDescData(const AEDesc* desc, void* dataPtr, Size maximumSize)
{
	if (desc == NULL || dataPtr == NULL || maximumSize < 0)
		return paramErr;
	AEStorage* storage = storageOf(desc);
	if (storage == NULL)
		return desc->descriptorType == typeNull ? noErr : errAENotAEDesc;
	if (isList(storage))
		return errAEWrongDataType;
	memcpy(dataPtr, storage->bytes, storage->size < maximumSize ? storage->size : maximumSize);
	return noErr;
}

OSErr AEReplaceDescData(DescType typeCode, const void* dataPtr, Size dataSize, AEDesc* desc)
{
	if (desc == NULL)
		return paramErr;
	AEDisposeDesc(desc);
	return AECreateDesc(typeCode, dataPtr, dataSize, desc);
}

OSErr AECoercePtr(DescType typeCode, const void* dataPtr, Size dataSize, DescType toType, AEDesc* result)
{
	if (result == NULL)
		return paramErr;
	AEDesc desc;
	OSErr err = AECreateDesc(typeCode, dataPtr, dataSize, &desc);
	if (err != noErr) {
		AEInitializeDesc(result);
		return err;
	}
	err = coerceDesc(&desc, toType, result);
	AEDisposeDesc(&desc);
	return err;
}

OSErr AECoerceDesc(const AEDesc* desc, DescType toType, AEDesc* result)
{
	if (desc == NULL || result == NULL)
		return paramErr;
	if (desc == result) {
		// Coercing in place: build the result first, then replace the original.
		AEDesc coerced;
		OSErr err = coerceDesc(desc, toType, &coerced);
		if (err != noErr)
			return err;
		AEDisposeDesc(result);
		*result = coerced;
		return noErr;
	}
	return coerceDesc(desc, toType, result);
}

Boolean AECheckIsRecord(const AEDesc* desc)
{
	return isRecordLike(storageOf(desc));
}

// --- Lists and records ---

OSErr AECreateList(const void* factoringPtr, Size factoredSize, Boolean isRecord, AEDescList* result)
{
	if (result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	return newStorage(isRecord ? typeAERecord : typeAEList, isRecord ? kAEKindRecord : kAEKindList, NULL, 0, result);
}

OSErr AECountItems(const AEDescList* list, long* count)
{
	if (list == NULL || count == NULL)
		return paramErr;
	AEStorage* storage = storageOf(list);
	if (!isList(storage))
		return errAEWrongDataType;
	*count = storage->itemCount;
	return noErr;
}

OSErr AEPutDesc(AEDescList* list, long index, const AEDesc* desc)
{
	if (list == NULL || desc == NULL)
		return paramErr;
	AEStorage* storage = storageOf(list);
	if (!isList(storage))
		return errAEWrongDataType;
	// 0 or count + 1 appends; 1...count replaces; anything else is out of range.
	if (index == 0 || index == storage->itemCount + 1)
		return appendItem(&storage->items, &storage->itemCount, &storage->itemCapacity, 0, desc);
	if (index < 0 || index > storage->itemCount)
		return errAEIllegalIndex;

	AEDesc copy;
	OSErr err = AEDuplicateDesc(desc, &copy);
	if (err != noErr)
		return err;
	AEDisposeDesc(&storage->items[index - 1].desc);
	storage->items[index - 1].desc = copy;
	return noErr;
}

OSErr AEPutPtr(AEDescList* list, long index, DescType typeCode, const void* dataPtr, Size dataSize)
{
	AEDesc desc;
	OSErr err = AECreateDesc(typeCode, dataPtr, dataSize, &desc);
	if (err != noErr)
		return err;
	err = AEPutDesc(list, index, &desc);
	AEDisposeDesc(&desc);
	return err;
}

static OSErr nthItem(const AEDescList* list, long index, AEItem** item)
{
	AEStorage* storage = storageOf(list);
	if (!isList(storage))
		return errAEWrongDataType;
	if (index < 1 || index > storage->itemCount)
		return errAEBadListItem;
	*item = &storage->items[index - 1];
	return noErr;
}

OSErr AEGetNthDesc(const AEDescList* list, long index, DescType desiredType, AEKeyword* keyword, AEDesc* result)
{
	if (list == NULL || result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	AEItem* item;
	OSErr err = nthItem(list, index, &item);
	if (err != noErr)
		return err;
	if (keyword)
		*keyword = item->key;
	return coerceDesc(&item->desc, desiredType, result);
}

OSErr AEGetNthPtr(const AEDescList* list, long index, DescType desiredType, AEKeyword* keyword,
	DescType* typeCode, void* dataPtr, Size maximumSize, Size* actualSize)
{
	if (list == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = nthItem(list, index, &item);
	if (err != noErr)
		return err;
	if (keyword)
		*keyword = item->key;
	return copyCoercedData(&item->desc, desiredType, typeCode, dataPtr, maximumSize, actualSize);
}

OSErr AESizeOfNthItem(const AEDescList* list, long index, DescType* typeCode, Size* dataSize)
{
	if (list == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = nthItem(list, index, &item);
	if (err != noErr)
		return err;
	if (typeCode)
		*typeCode = item->desc.descriptorType;
	if (dataSize)
		*dataSize = AEGetDescDataSize(&item->desc);
	return noErr;
}

OSErr AEDeleteItem(AEDescList* list, long index)
{
	if (list == NULL)
		return paramErr;
	AEStorage* storage = storageOf(list);
	if (!isList(storage))
		return errAEWrongDataType;
	if (index < 1 || index > storage->itemCount)
		return errAEBadListItem;
	AEDisposeDesc(&storage->items[index - 1].desc);
	memmove(&storage->items[index - 1], &storage->items[index], (storage->itemCount - index) * sizeof(AEItem));
	storage->itemCount--;
	return noErr;
}

// --- Parameters (records and Apple Events) ---

static OSErr putKeyed(AEItem** items, long* count, long* capacity, AEKeyword keyword, const AEDesc* desc)
{
	AEItem* existing = findItem(*items, *count, keyword);
	if (existing == NULL)
		return appendItem(items, count, capacity, keyword, desc);

	AEDesc copy;
	OSErr err = AEDuplicateDesc(desc, &copy);
	if (err != noErr)
		return err;
	AEDisposeDesc(&existing->desc);
	existing->desc = copy;
	return noErr;
}

OSErr AEPutParamDesc(AERecord* record, AEKeyword keyword, const AEDesc* desc)
{
	if (record == NULL || desc == NULL)
		return paramErr;
	AEStorage* storage = storageOf(record);
	if (!isRecordLike(storage))
		return errAEWrongDataType;
	return putKeyed(&storage->items, &storage->itemCount, &storage->itemCapacity, keyword, desc);
}

OSErr AEPutParamPtr(AERecord* record, AEKeyword keyword, DescType typeCode, const void* dataPtr, Size dataSize)
{
	AEDesc desc;
	OSErr err = AECreateDesc(typeCode, dataPtr, dataSize, &desc);
	if (err != noErr)
		return err;
	err = AEPutParamDesc(record, keyword, &desc);
	AEDisposeDesc(&desc);
	return err;
}

static OSErr findParam(const AERecord* record, AEKeyword keyword, AEItem** item)
{
	AEStorage* storage = storageOf(record);
	if (!isRecordLike(storage))
		return errAEWrongDataType;
	*item = findItem(storage->items, storage->itemCount, keyword);
	return *item ? noErr : errAEDescNotFound;
}

OSErr AEGetParamDesc(const AERecord* record, AEKeyword keyword, DescType desiredType, AEDesc* result)
{
	if (record == NULL || result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	AEItem* item;
	OSErr err = findParam(record, keyword, &item);
	if (err != noErr)
		return err;
	return coerceDesc(&item->desc, desiredType, result);
}

OSErr AEGetParamPtr(const AERecord* record, AEKeyword keyword, DescType desiredType, DescType* typeCode,
	void* dataPtr, Size maximumSize, Size* actualSize)
{
	if (record == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = findParam(record, keyword, &item);
	if (err != noErr)
		return err;
	return copyCoercedData(&item->desc, desiredType, typeCode, dataPtr, maximumSize, actualSize);
}

OSErr AESizeOfParam(const AERecord* record, AEKeyword keyword, DescType* typeCode, Size* dataSize)
{
	if (record == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = findParam(record, keyword, &item);
	if (err != noErr)
		return err;
	if (typeCode)
		*typeCode = item->desc.descriptorType;
	if (dataSize)
		*dataSize = AEGetDescDataSize(&item->desc);
	return noErr;
}

OSErr AEDeleteParam(AERecord* record, AEKeyword keyword)
{
	if (record == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = findParam(record, keyword, &item);
	if (err != noErr)
		return err;
	AEStorage* storage = storageOf(record);
	long index = item - storage->items;
	AEDisposeDesc(&item->desc);
	memmove(item, item + 1, (storage->itemCount - index - 1) * sizeof(AEItem));
	storage->itemCount--;
	return noErr;
}

// --- Apple Events ---

OSErr AECreateAppleEvent(AEEventClass theAEEventClass, AEEventID theAEEventID, const AEAddressDesc* target,
	AEReturnID returnID, AETransactionID transactionID, AppleEvent* result)
{
	if (result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	OSErr err = newStorage(typeAppleEvent, kAEKindEvent, NULL, 0, result);
	if (err != noErr)
		return err;

	SInt32 transaction = transactionID;
	SInt16 returnValue = returnID;
	if ((err = AEPutAttributePtr(result, keyEventClassAttr, typeType, &theAEEventClass, sizeof(theAEEventClass))) == noErr &&
		(err = AEPutAttributePtr(result, keyEventIDAttr, typeType, &theAEEventID, sizeof(theAEEventID))) == noErr &&
		(err = AEPutAttributePtr(result, keyReturnIDAttr, typeSInt16, &returnValue, sizeof(returnValue))) == noErr &&
		(err = AEPutAttributePtr(result, keyTransactionIDAttr, typeSInt32, &transaction, sizeof(transaction))) == noErr &&
		target != NULL)
	{
		err = AEPutAttributeDesc(result, keyAddressAttr, target);
	}
	if (err != noErr)
		AEDisposeDesc(result);
	return err;
}

OSErr AEPutAttributeDesc(AppleEvent* theAppleEvent, AEKeyword keyword, const AEDesc* desc)
{
	if (theAppleEvent == NULL || desc == NULL)
		return paramErr;
	AEStorage* storage = storageOf(theAppleEvent);
	if (!isEvent(storage))
		return errAEWrongDataType;
	return putKeyed(&storage->attributes, &storage->attributeCount, &storage->attributeCapacity, keyword, desc);
}

OSErr AEPutAttributePtr(AppleEvent* theAppleEvent, AEKeyword keyword, DescType typeCode, const void* dataPtr, Size dataSize)
{
	AEDesc desc;
	OSErr err = AECreateDesc(typeCode, dataPtr, dataSize, &desc);
	if (err != noErr)
		return err;
	err = AEPutAttributeDesc(theAppleEvent, keyword, &desc);
	AEDisposeDesc(&desc);
	return err;
}

static OSErr findAttribute(const AppleEvent* theAppleEvent, AEKeyword keyword, AEItem** item)
{
	AEStorage* storage = storageOf(theAppleEvent);
	if (!isEvent(storage))
		return errAEWrongDataType;
	*item = findItem(storage->attributes, storage->attributeCount, keyword);
	return *item ? noErr : errAEDescNotFound;
}

OSErr AEGetAttributeDesc(const AppleEvent* theAppleEvent, AEKeyword keyword, DescType desiredType, AEDesc* result)
{
	if (theAppleEvent == NULL || result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	AEItem* item;
	OSErr err = findAttribute(theAppleEvent, keyword, &item);
	if (err != noErr)
		return err;
	return coerceDesc(&item->desc, desiredType, result);
}

OSErr AEGetAttributePtr(const AppleEvent* theAppleEvent, AEKeyword keyword, DescType desiredType, DescType* typeCode,
	void* dataPtr, Size maximumSize, Size* actualSize)
{
	if (theAppleEvent == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = findAttribute(theAppleEvent, keyword, &item);
	if (err != noErr)
		return err;
	return copyCoercedData(&item->desc, desiredType, typeCode, dataPtr, maximumSize, actualSize);
}

OSErr AESizeOfAttribute(const AppleEvent* theAppleEvent, AEKeyword keyword, DescType* typeCode, Size* dataSize)
{
	if (theAppleEvent == NULL)
		return paramErr;
	AEItem* item;
	OSErr err = findAttribute(theAppleEvent, keyword, &item);
	if (err != noErr)
		return err;
	if (typeCode)
		*typeCode = item->desc.descriptorType;
	if (dataSize)
		*dataSize = AEGetDescDataSize(&item->desc);
	return noErr;
}

OSStatus AESendMessage(const AppleEvent* event, AppleEvent* reply, AESendMode sendMode, long timeOutInTicks)
{
	if (reply)
		AEInitializeDesc(reply);
	return procNotFound; // no Apple Event transport to other processes
}

// --- Special handlers ---

OSErr AEInstallSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP handler, Boolean isSysHandler)
{
	return noErr;
}

OSErr AERemoveSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP handler, Boolean isSysHandler)
{
	return noErr;
}

OSErr AEGetSpecialHandler(AEKeyword functionClass, AEEventHandlerUPP* handler, Boolean isSysHandler)
{
	if (handler)
		*handler = NULL;
	return errAEHandlerNotFound;
}

// --- Debugging ---

static void appendText(char** buffer, size_t* length, size_t* capacity, const char* text)
{
	size_t add = strlen(text);
	if (*length + add + 1 > *capacity) {
		size_t newCapacity = (*length + add + 1) * 2;
		char* grown = realloc(*buffer, newCapacity);
		if (grown == NULL)
			return;
		*buffer = grown;
		*capacity = newCapacity;
	}
	memcpy(*buffer + *length, text, add + 1);
	*length += add;
}

static void fourCC(char out[5], FourCharCode code)
{
	for (int i = 0; i < 4; i++) {
		char c = (char) (code >> (24 - 8 * i));
		out[i] = (c >= 32 && c < 127) ? c : '?';
	}
	out[4] = 0;
}

static void describe(const AEDesc* desc, char** buffer, size_t* length, size_t* capacity)
{
	char type[5], key[5], text[64];
	fourCC(type, desc->descriptorType);
	AEStorage* storage = storageOf(desc);
	if (!isList(storage)) {
		snprintf(text, sizeof(text), "'%s'(%ld bytes)", type, (long) AEGetDescDataSize(desc));
		appendText(buffer, length, capacity, text);
		return;
	}
	snprintf(text, sizeof(text), "'%s'{", type);
	appendText(buffer, length, capacity, text);
	for (long i = 0; i < storage->itemCount; i++) {
		if (i)
			appendText(buffer, length, capacity, ", ");
		if (storage->kind != kAEKindList) {
			fourCC(key, storage->items[i].key);
			snprintf(text, sizeof(text), "'%s':", key);
			appendText(buffer, length, capacity, text);
		}
		describe(&storage->items[i].desc, buffer, length, capacity);
	}
	appendText(buffer, length, capacity, "}");
}

OSStatus AEPrintDescToHandle(const AEDesc* desc, Handle* result)
{
	if (desc == NULL || result == NULL)
		return paramErr;
	char* buffer = NULL;
	size_t length = 0, capacity = 0;
	describe(desc, &buffer, &length, &capacity);
	if (buffer == NULL)
		return memFullErr;

	Handle handle = NewHandle(length + 1);
	if (handle == NULL) {
		free(buffer);
		return memFullErr;
	}
	memcpy(*handle, buffer, length + 1);
	free(buffer);
	*result = handle;
	return noErr;
}
