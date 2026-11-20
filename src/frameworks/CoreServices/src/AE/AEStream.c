/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
*/

// AEStream: builds descriptors, lists, records and Apple Events by appending to a stream.
// This is written on top of the public Descriptor Manager calls (AEDescriptors.c), so the
// results are ordinary descriptors.

#include <AE/AE.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum StreamFrameKind {
	kStreamFrameData,   // a descriptor whose data is being appended
	kStreamFrameList,
	kStreamFrameRecord,
} StreamFrameKind;

// A descriptor under construction. keyed and key are the key the finished descriptor is stored
// under when its parent is a record or an event.
typedef struct StreamFrame {
	StreamFrameKind kind;
	Boolean keyed;
	AEKeyword key;
	DescType dataType;
	uint8_t* data;
	size_t size;
	size_t capacity;
	AEDesc desc;        // list or record
} StreamFrame;

struct OpaqueAEStreamRef {
	StreamFrame* frames;
	long depth;
	long capacity;
	AppleEvent event;   // when the stream builds an Apple Event: the event, which takes the values
	Boolean isEvent;
	AEDesc result;      // top-level value of a stream that does not build an event
	Boolean hasResult;
	Boolean pendingKeyed;   // a key waiting for the next value
	AEKeyword pendingKey;
};

static void freeFrame(StreamFrame* frame)
{
	free(frame->data);
	AEDisposeDesc(&frame->desc);
}

static OSStatus pushFrame(AEStreamRef ref, StreamFrameKind kind, DescType type)
{
	if (ref->depth == ref->capacity) {
		long capacity = ref->capacity ? ref->capacity * 2 : 4;
		StreamFrame* grown = realloc(ref->frames, capacity * sizeof(StreamFrame));
		if (grown == NULL)
			return memFullErr;
		ref->frames = grown;
		ref->capacity = capacity;
	}
	StreamFrame* frame = &ref->frames[ref->depth];
	memset(frame, 0, sizeof(*frame));
	frame->kind = kind;
	frame->dataType = type;
	AEInitializeDesc(&frame->desc);
	if (kind == kStreamFrameList || kind == kStreamFrameRecord) {
		OSStatus err = AECreateList(NULL, 0, kind == kStreamFrameRecord, &frame->desc);
		if (err != noErr)
			return err;
		if (kind == kStreamFrameRecord && type != typeAERecord) {
			AEDesc retyped;
			err = AECoerceDesc(&frame->desc, type, &retyped);
			if (err != noErr) {
				AEDisposeDesc(&frame->desc);
				return err;
			}
			AEDisposeDesc(&frame->desc);
			frame->desc = retyped;
		}
	}
	// A key written before this value belongs to it, not to what is written inside it.
	frame->keyed = ref->pendingKeyed;
	frame->key = ref->pendingKey;
	ref->pendingKeyed = false;
	ref->depth++;
	return noErr;
}

// Stores a finished value in the descriptor under construction around it.
static OSStatus deliver(AEStreamRef ref, Boolean keyed, AEKeyword key, const AEDesc* value)
{
	if (ref->depth > 0) {
		StreamFrame* parent = &ref->frames[ref->depth - 1];
		if (parent->kind == kStreamFrameList)
			return AEPutDesc(&parent->desc, 0, value);
		return AEPutParamDesc(&parent->desc, keyed ? key : typeWildCard, value);
	}
	if (ref->isEvent)
		return AEPutParamDesc(&ref->event, keyed ? key : typeWildCard, value);
	// Only the first top-level value is kept.
	if (!ref->hasResult) {
		OSStatus err = AEDuplicateDesc(value, &ref->result);
		if (err == noErr)
			ref->hasResult = true;
		return err;
	}
	return noErr;
}

// Finishes the frame on top of the stack and stores its value in the one below.
static OSStatus popFrame(AEStreamRef ref)
{
	StreamFrame* frame = &ref->frames[ref->depth - 1];
	AEDesc value;
	OSStatus err = noErr;

	AEInitializeDesc(&value);
	if (frame->kind == kStreamFrameData) {
		err = AECreateDesc(frame->dataType, frame->data, (Size) frame->size, &value);
	} else {
		value = frame->desc;
		AEInitializeDesc(&frame->desc);
	}
	Boolean keyed = frame->keyed;
	AEKeyword key = frame->key;
	freeFrame(frame);
	ref->depth--;
	if (err == noErr)
		err = deliver(ref, keyed, key, &value);
	AEDisposeDesc(&value);
	return err;
}

static StreamFrame* topFrame(AEStreamRef ref, StreamFrameKind kind)
{
	if (ref == NULL || ref->depth == 0)
		return NULL;
	StreamFrame* frame = &ref->frames[ref->depth - 1];
	return frame->kind == kind ? frame : NULL;
}

// Whether the next value is stored under a key: only the parameters of a record or an event are.
static Boolean takesKeys(AEStreamRef ref)
{
	if (ref->depth > 0)
		return ref->frames[ref->depth - 1].kind == kStreamFrameRecord;
	return ref->isEvent;
}

AEStreamRef AEStreamOpen(void)
{
	return calloc(1, sizeof(struct OpaqueAEStreamRef));
}

OSStatus AEStreamClose(AEStreamRef ref, AEDesc* desc)
{
	if (ref == NULL)
		return paramErr;

	OSStatus err = noErr;
	// Lists and records still open are finished; a descriptor whose data is still being
	// appended cannot be.
	while (ref->depth > 0 && err == noErr) {
		if (ref->frames[ref->depth - 1].kind == kStreamFrameData)
			err = errAEStreamBadNesting;
		else
			err = popFrame(ref);
	}
	AEDesc result;
	AEInitializeDesc(&result);
	if (err == noErr) {
		if (ref->isEvent) {
			result = ref->event;
			AEInitializeDesc(&ref->event);
		} else if (ref->hasResult) {
			result = ref->result;
			AEInitializeDesc(&ref->result);
		} else {
			err = errAEStreamBadNesting;
		}
	}

	if (desc != NULL)
		*desc = result;
	else
		AEDisposeDesc(&result);
	for (long i = 0; i < ref->depth; i++)
		freeFrame(&ref->frames[i]);
	free(ref->frames);
	AEDisposeDesc(&ref->event);
	AEDisposeDesc(&ref->result);
	free(ref);
	return err;
}

OSStatus AEStreamOpenDesc(AEStreamRef ref, DescType newType)
{
	if (ref == NULL)
		return paramErr;
	return pushFrame(ref, kStreamFrameData, newType);
}

OSStatus AEStreamWriteData(AEStreamRef ref, const void* data, Size length)
{
	if (ref == NULL || length < 0 || (data == NULL && length > 0))
		return paramErr;
	StreamFrame* frame = topFrame(ref, kStreamFrameData);
	if (frame == NULL)
		return errAEStreamBadNesting;
	if (frame->size + (size_t) length > frame->capacity) {
		size_t capacity = frame->capacity ? frame->capacity : 64;
		while (capacity < frame->size + (size_t) length)
			capacity *= 2;
		uint8_t* grown = realloc(frame->data, capacity);
		if (grown == NULL)
			return memFullErr;
		frame->data = grown;
		frame->capacity = capacity;
	}
	if (length > 0)
		memcpy(frame->data + frame->size, data, (size_t) length);
	frame->size += (size_t) length;
	return noErr;
}

OSStatus AEStreamCloseDesc(AEStreamRef ref)
{
	if (ref == NULL)
		return paramErr;
	if (topFrame(ref, kStreamFrameData) == NULL)
		return errAEStreamBadNesting;
	return popFrame(ref);
}

OSStatus AEStreamWriteDesc(AEStreamRef ref, DescType newType, const void* data, Size length)
{
	if (ref == NULL)
		return paramErr;
	AEDesc value;
	OSStatus err = AECreateDesc(newType, data, length, &value);
	if (err != noErr)
		return err;
	Boolean keyed = ref->pendingKeyed;
	AEKeyword key = ref->pendingKey;
	ref->pendingKeyed = false;
	err = deliver(ref, keyed, key, &value);
	AEDisposeDesc(&value);
	return err;
}

OSStatus AEStreamWriteAEDesc(AEStreamRef ref, const AEDesc* desc)
{
	if (ref == NULL || desc == NULL)
		return paramErr;
	Boolean keyed = ref->pendingKeyed;
	AEKeyword key = ref->pendingKey;
	ref->pendingKeyed = false;
	return deliver(ref, keyed, key, desc);
}

OSStatus AEStreamOpenList(AEStreamRef ref)
{
	if (ref == NULL)
		return paramErr;
	return pushFrame(ref, kStreamFrameList, typeAEList);
}

OSStatus AEStreamCloseList(AEStreamRef ref)
{
	if (ref == NULL)
		return paramErr;
	if (topFrame(ref, kStreamFrameList) == NULL)
		return errAEStreamBadNesting;
	return popFrame(ref);
}

OSStatus AEStreamOpenRecord(AEStreamRef ref, DescType newType)
{
	if (ref == NULL)
		return paramErr;
	return pushFrame(ref, kStreamFrameRecord, newType);
}

OSStatus AEStreamSetRecordType(AEStreamRef ref, DescType newType)
{
	if (ref == NULL)
		return paramErr;
	StreamFrame* frame = topFrame(ref, kStreamFrameRecord);
	if (frame == NULL)
		return errAEStreamBadNesting;
	AEDesc retyped;
	OSStatus err = AECoerceDesc(&frame->desc, newType, &retyped);
	if (err != noErr)
		return err;
	AEDisposeDesc(&frame->desc);
	frame->desc = retyped;
	frame->dataType = newType;
	return noErr;
}

OSStatus AEStreamCloseRecord(AEStreamRef ref)
{
	if (ref == NULL)
		return paramErr;
	if (topFrame(ref, kStreamFrameRecord) == NULL)
		return errAEStreamBadNesting;
	return popFrame(ref);
}

OSStatus AEStreamWriteKey(AEStreamRef ref, AEKeyword key)
{
	if (ref == NULL)
		return paramErr;
	ref->pendingKeyed = true;
	ref->pendingKey = key;
	return noErr;
}

OSStatus AEStreamWriteKeyDesc(AEStreamRef ref, AEKeyword key, DescType newType, const void* data, Size length)
{
	if (ref == NULL)
		return paramErr;
	// Outside a record or an event the key has nothing to be stored under and is dropped.
	if (takesKeys(ref))
		AEStreamWriteKey(ref, key);
	return AEStreamWriteDesc(ref, newType, data, length);
}

OSStatus AEStreamOpenKeyDesc(AEStreamRef ref, AEKeyword key, DescType newType)
{
	if (ref == NULL)
		return paramErr;
	if (takesKeys(ref))
		AEStreamWriteKey(ref, key);
	return AEStreamOpenDesc(ref, newType);
}

AEStreamRef AEStreamCreateEvent(AEEventClass clazz, AEEventID id, DescType targetType, const void* targetData,
	Size targetLength, SInt16 returnID, SInt32 transactionID)
{
	AEStreamRef ref = AEStreamOpen();
	if (ref == NULL)
		return NULL;

	AEDesc target;
	AEInitializeDesc(&target);
	OSStatus err = AECreateDesc(targetType, targetData, targetLength, &target);
	if (err == noErr)
		err = AECreateAppleEvent(clazz, id, target.descriptorType == typeNull ? NULL : &target, returnID,
			transactionID, &ref->event);
	AEDisposeDesc(&target);
	if (err != noErr) {
		free(ref);
		return NULL;
	}
	ref->isEvent = true;
	return ref;
}

AEStreamRef AEStreamOpenEvent(AppleEvent* event)
{
	if (event == NULL || event->descriptorType != typeAppleEvent || !AECheckIsRecord(event))
		return NULL;
	AEStreamRef ref = AEStreamOpen();
	if (ref == NULL)
		return NULL;
	// The stream takes the event over; the caller gets it back from AEStreamClose.
	ref->event = *event;
	ref->isEvent = true;
	AEInitializeDesc(event);
	return ref;
}

OSStatus AEStreamOptionalParam(AEStreamRef ref, AEKeyword key)
{
	if (ref == NULL)
		return paramErr;
	if (!ref->isEvent)
		return noErr;

	// The keywords are kept in the event's optional-keyword attribute, a list of keywords.
	AEDescList keywords;
	if (AEGetAttributeDesc(&ref->event, keyOptionalKeywordAttr, typeAEList, &keywords) != noErr) {
		OSStatus err = AECreateList(NULL, 0, false, &keywords);
		if (err != noErr)
			return err;
	}
	OSStatus err = AEPutPtr(&keywords, 0, typeKeyword, &key, sizeof(key));
	if (err == noErr)
		err = AEPutAttributeDesc(&ref->event, keyOptionalKeywordAttr, &keywords);
	AEDisposeDesc(&keywords);
	return err;
}
