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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define AE_STORAGE_MAGIC 0x41454453 // 'AEDS'

typedef struct AEItem {
	AEKeyword key;
	AEDesc desc;
} AEItem;

typedef struct AEStorage {
	uint32_t magic;
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

static Boolean isList(DescType type)
{
	return type == typeAEList || type == typeAERecord || type == typeAppleEvent;
}

static Boolean isRecordLike(DescType type)
{
	return type == typeAERecord || type == typeAppleEvent;
}

static OSErr newStorage(DescType type, const void* dataPtr, Size dataSize, AEDesc* result)
{
	AEStorage* storage = calloc(1, sizeof(*storage));
	if (storage == NULL)
		return memFullErr;
	storage->magic = AE_STORAGE_MAGIC;
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

static OSErr coerceDesc(const AEDesc* desc, DescType desiredType, AEDesc* result)
{
	if (desiredType == typeWildCard || desiredType == desc->descriptorType)
		return AEDuplicateDesc(desc, result);
	return errAECoercionFail;
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
	return newStorage(typeCode, dataPtr, dataSize, result);
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

	OSErr err = newStorage(desc->descriptorType, source->bytes, source->size, &copy);
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
	return (storage && !isList(desc->descriptorType)) ? storage->size : 0;
}

OSErr AEGetDescData(const AEDesc* desc, void* dataPtr, Size maximumSize)
{
	if (desc == NULL || dataPtr == NULL || maximumSize < 0)
		return paramErr;
	AEStorage* storage = storageOf(desc);
	if (storage == NULL)
		return desc->descriptorType == typeNull ? noErr : errAENotAEDesc;
	if (isList(desc->descriptorType))
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

// --- Lists and records ---

OSErr AECreateList(const void* factoringPtr, Size factoredSize, Boolean isRecord, AEDescList* result)
{
	if (result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	return newStorage(isRecord ? typeAERecord : typeAEList, NULL, 0, result);
}

OSErr AECountItems(const AEDescList* list, long* count)
{
	if (list == NULL || count == NULL)
		return paramErr;
	AEStorage* storage = storageOf(list);
	if (storage == NULL || !isList(list->descriptorType))
		return errAEWrongDataType;
	*count = storage->itemCount;
	return noErr;
}

OSErr AEPutDesc(AEDescList* list, long index, const AEDesc* desc)
{
	if (list == NULL || desc == NULL)
		return paramErr;
	AEStorage* storage = storageOf(list);
	if (storage == NULL || !isList(list->descriptorType))
		return errAEWrongDataType;
	if (index <= 0 || index > storage->itemCount)
		return appendItem(&storage->items, &storage->itemCount, &storage->itemCapacity, 0, desc);

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

OSErr AEGetNthDesc(const AEDescList* list, long index, DescType desiredType, AEKeyword* keyword, AEDesc* result)
{
	if (list == NULL || result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	AEStorage* storage = storageOf(list);
	if (storage == NULL || !isList(list->descriptorType))
		return errAEWrongDataType;
	if (index < 1 || index > storage->itemCount)
		return errAEBadListItem;
	AEItem* item = &storage->items[index - 1];
	if (keyword)
		*keyword = item->key;
	return coerceDesc(&item->desc, desiredType, result);
}

OSErr AEGetNthPtr(const AEDescList* list, long index, DescType desiredType, AEKeyword* keyword,
	DescType* typeCode, void* dataPtr, Size maximumSize, Size* actualSize)
{
	AEDesc desc;
	OSErr err = AEGetNthDesc(list, index, desiredType, keyword, &desc);
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

OSErr AEDeleteItem(AEDescList* list, long index)
{
	AEStorage* storage = storageOf(list);
	if (storage == NULL || !isList(list->descriptorType))
		return errAEWrongDataType;
	if (index < 1 || index > storage->itemCount)
		return errAEBadListItem;
	AEDisposeDesc(&storage->items[index - 1].desc);
	memmove(&storage->items[index - 1], &storage->items[index], (storage->itemCount - index) * sizeof(AEItem));
	storage->itemCount--;
	return noErr;
}

// --- Parameters (records and Apple Events) ---

OSErr AEPutParamDesc(AERecord* record, AEKeyword keyword, const AEDesc* desc)
{
	if (record == NULL || desc == NULL)
		return paramErr;
	AEStorage* storage = storageOf(record);
	if (storage == NULL || !isRecordLike(record->descriptorType))
		return errAEWrongDataType;
	AEItem* existing = findItem(storage->items, storage->itemCount, keyword);
	if (existing == NULL)
		return appendItem(&storage->items, &storage->itemCount, &storage->itemCapacity, keyword, desc);

	AEDesc copy;
	OSErr err = AEDuplicateDesc(desc, &copy);
	if (err != noErr)
		return err;
	AEDisposeDesc(&existing->desc);
	existing->desc = copy;
	return noErr;
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

OSErr AEGetParamDesc(const AERecord* record, AEKeyword keyword, DescType desiredType, AEDesc* result)
{
	if (record == NULL || result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	AEStorage* storage = storageOf(record);
	if (storage == NULL || !isRecordLike(record->descriptorType))
		return errAEWrongDataType;
	AEItem* item = findItem(storage->items, storage->itemCount, keyword);
	if (item == NULL)
		return errAEDescNotFound;
	return coerceDesc(&item->desc, desiredType, result);
}

OSErr AEGetParamPtr(const AERecord* record, AEKeyword keyword, DescType desiredType, DescType* typeCode,
	void* dataPtr, Size maximumSize, Size* actualSize)
{
	AEDesc desc;
	OSErr err = AEGetParamDesc(record, keyword, desiredType, &desc);
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

OSErr AESizeOfParam(const AERecord* record, AEKeyword keyword, DescType* typeCode, Size* dataSize)
{
	AEStorage* storage = storageOf(record);
	if (storage == NULL || !isRecordLike(record->descriptorType))
		return errAEWrongDataType;
	AEItem* item = findItem(storage->items, storage->itemCount, keyword);
	if (item == NULL)
		return errAEDescNotFound;
	if (typeCode)
		*typeCode = item->desc.descriptorType;
	if (dataSize)
		*dataSize = AEGetDescDataSize(&item->desc);
	return noErr;
}

OSErr AEDeleteParam(AERecord* record, AEKeyword keyword)
{
	AEStorage* storage = storageOf(record);
	if (storage == NULL || !isRecordLike(record->descriptorType))
		return errAEWrongDataType;
	AEItem* item = findItem(storage->items, storage->itemCount, keyword);
	if (item == NULL)
		return errAEDescNotFound;
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
	OSErr err = newStorage(typeAppleEvent, NULL, 0, result);
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
	if (storage == NULL || theAppleEvent->descriptorType != typeAppleEvent)
		return errAEWrongDataType;
	AEItem* existing = findItem(storage->attributes, storage->attributeCount, keyword);
	if (existing == NULL)
		return appendItem(&storage->attributes, &storage->attributeCount, &storage->attributeCapacity, keyword, desc);

	AEDesc copy;
	OSErr err = AEDuplicateDesc(desc, &copy);
	if (err != noErr)
		return err;
	AEDisposeDesc(&existing->desc);
	existing->desc = copy;
	return noErr;
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

OSErr AEGetAttributeDesc(const AppleEvent* theAppleEvent, AEKeyword keyword, DescType desiredType, AEDesc* result)
{
	if (theAppleEvent == NULL || result == NULL)
		return paramErr;
	AEInitializeDesc(result);
	AEStorage* storage = storageOf(theAppleEvent);
	if (storage == NULL || theAppleEvent->descriptorType != typeAppleEvent)
		return errAEWrongDataType;
	AEItem* item = findItem(storage->attributes, storage->attributeCount, keyword);
	if (item == NULL)
		return errAEDescNotFound;
	return coerceDesc(&item->desc, desiredType, result);
}

OSErr AEGetAttributePtr(const AppleEvent* theAppleEvent, AEKeyword keyword, DescType desiredType, DescType* typeCode,
	void* dataPtr, Size maximumSize, Size* actualSize)
{
	AEDesc desc;
	OSErr err = AEGetAttributeDesc(theAppleEvent, keyword, desiredType, &desc);
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
	if (!isList(desc->descriptorType) || storage == NULL) {
		snprintf(text, sizeof(text), "'%s'(%ld bytes)", type, (long) AEGetDescDataSize(desc));
		appendText(buffer, length, capacity, text);
		return;
	}
	snprintf(text, sizeof(text), "'%s'{", type);
	appendText(buffer, length, capacity, text);
	for (long i = 0; i < storage->itemCount; i++) {
		if (i)
			appendText(buffer, length, capacity, ", ");
		if (desc->descriptorType != typeAEList) {
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
