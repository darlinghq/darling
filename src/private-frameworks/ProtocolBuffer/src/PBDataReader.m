/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#import <ProtocolBuffer/PBDataReader.h>
#import <ProtocolBuffer/PBConstants.h>
#import <darling-stub-forwarding.h>
#include <string.h>

@implementation PBDataReader

- (instancetype)initWithData:(NSData*)data
{
	self = [super init];
	if (self != nil) {
		_data = [data copy];
		_bytes = [_data bytes];
		_end = [_data length];
	}
	return self;
}

- (void)dealloc
{
	[_data release];
	[super dealloc];
}

- (NSData*)data
{
	return _data;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

static uint64_t ReadVarint(PBDataReader* reader)
{
	uint64_t value = 0;
	for (unsigned shift = 0; shift < 64; shift += 7) {
		if (reader->_position >= reader->_end) {
			reader->_error = YES;
			return 0;
		}
		uint8_t byte = reader->_bytes[reader->_position++];
		value |= (uint64_t)(byte & 0x7f) << shift;
		if (!(byte & 0x80))
			return value;
	}
	reader->_error = YES;
	return 0;
}

static BOOL SkipBytes(PBDataReader* reader, uint64_t count)
{
	if (reader->_error || count > reader->_end - reader->_position) {
		reader->_error = YES;
		return NO;
	}
	reader->_position += (size_t)count;
	return YES;
}

BOOL PBReaderHasMoreData(PBDataReader* reader)
{
	return !reader->_error && reader->_position < reader->_end;
}

BOOL PBReaderHasError(PBDataReader* reader)
{
	return reader->_error;
}

void PBReaderReadTag32AndType(PBDataReader* reader, uint32_t* tag, uint8_t* type)
{
	uint64_t key = ReadVarint(reader);
	*tag = (uint32_t)(key >> 3);
	*type = (uint8_t)(key & 7);
}

BOOL PBReaderReadBOOL(PBDataReader* reader)
{
	return ReadVarint(reader) != 0;
}

uint32_t PBReaderReadUint32(PBDataReader *reader)
{
	return (uint32_t)ReadVarint(reader);
}

int32_t PBReaderReadInt32(PBDataReader* reader)
{
	return (int32_t)ReadVarint(reader);
}

uint64_t PBReaderReadUint64(PBDataReader* reader)
{
	return ReadVarint(reader);
}

int64_t PBReaderReadInt64(PBDataReader* reader)
{
	return (int64_t)ReadVarint(reader);
}

// A double is a 64-bit little-endian IEEE 754 value (wire type 1).
double PBReaderReadDouble(PBDataReader* reader)
{
	size_t start = reader->_position;
	if (!SkipBytes(reader, 8))
		return 0;
	uint64_t bits = 0;
	for (int i = 7; i >= 0; i--)
		bits = (bits << 8) | reader->_bytes[start + i];
	double value;
	memcpy(&value, &bits, sizeof(value));
	return value;
}

NSData* PBReaderReadData(PBDataReader* reader)
{
	uint64_t length = ReadVarint(reader);
	size_t start = reader->_position;
	if (!SkipBytes(reader, length))
		return nil;
	return [NSData dataWithBytes: reader->_bytes + start length: (size_t)length];
}

NSString* PBReaderReadString(PBDataReader* reader)
{
	NSData* data = PBReaderReadData(reader);
	if (data == nil)
		return nil;
	NSString* string = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
	if (string == nil)
		reader->_error = YES;
	return [string autorelease];
}

BOOL PBReaderPlaceMark(PBDataReader* reader, PBDataReaderMark* mark)
{
	uint64_t length = ReadVarint(reader);
	if (reader->_error || length > reader->_end - reader->_position) {
		reader->_error = YES;
		return NO;
	}
	mark->previousEnd = reader->_end;
	mark->end = reader->_position + (size_t)length;
	reader->_end = mark->end;
	return YES;
}

void PBReaderRecallMark(PBDataReader* reader, PBDataReaderMark* mark)
{
	reader->_position = mark->end;
	reader->_end = mark->previousEnd;
}

// How deeply groups may be nested in a value that is skipped; deeper data is treated as malformed rather than
// recursed into without bound. It is the default nesting limit of the protobuf reference implementation.
#define PBMaxGroupDepth 100

static BOOL SkipValue(PBDataReader* reader, uint32_t tag, uint8_t type, unsigned depth)
{
	switch (type) {
		case 0: // varint
			ReadVarint(reader);
			return !reader->_error;
		case 1: // 64-bit
			return SkipBytes(reader, 8);
		case 2: // length-delimited
			return SkipBytes(reader, ReadVarint(reader));
		case 3: { // a group, up to its end
			if (depth >= PBMaxGroupDepth) {
				reader->_error = YES;
				return NO;
			}
			for (;;) {
				uint32_t innerTag = 0;
				uint8_t innerType = 0;
				PBReaderReadTag32AndType(reader, &innerTag, &innerType);
				if (reader->_error)
					return NO;
				if (innerType == TYPE_END_GROUP) {
					if (innerTag == tag)
						return YES;
					reader->_error = YES;
					return NO;
				}
				if (!SkipValue(reader, innerTag, innerType, depth + 1))
					return NO;
			}
		}
		case 5: // 32-bit
			return SkipBytes(reader, 4);
		default:
			reader->_error = YES;
			return NO;
	}
}

BOOL PBReaderSkipValueWithTag(PBDataReader* reader, uint32_t tag, uint8_t type)
{
	return SkipValue(reader, tag, type, 0);
}
