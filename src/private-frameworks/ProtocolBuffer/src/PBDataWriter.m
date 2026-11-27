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


#import <ProtocolBuffer/PBDataWriter.h>
#import <ProtocolBuffer/PBCodable.h>
#import <darling-stub-forwarding.h>
#include <string.h>

@implementation PBDataWriter

- (instancetype)init
{
	self = [super init];
	if (self != nil)
		_data = [[NSMutableData alloc] init];
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

static void WriteVarint(PBDataWriter* writer, uint64_t value)
{
	uint8_t bytes[10];
	size_t count = 0;
	while (value >= 0x80) {
		bytes[count++] = (uint8_t)(value | 0x80);
		value >>= 7;
	}
	bytes[count++] = (uint8_t)value;
	[writer->_data appendBytes: bytes length: count];
}

static void WriteKey(PBDataWriter* writer, uint32_t tag, unsigned type)
{
	WriteVarint(writer, ((uint64_t)tag << 3) | type);
}

static void WriteFixed(PBDataWriter* writer, uint64_t value, unsigned size)
{
	uint8_t bytes[8];
	for (unsigned i = 0; i < size; i++)
		bytes[i] = (uint8_t)(value >> (8 * i)); // little endian
	[writer->_data appendBytes: bytes length: size];
}

void PBDataWriterWriteBOOLField(PBDataWriter* writer, BOOL boolean, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, boolean ? 1 : 0);
}

void PBDataWriterWriteDataField(PBDataWriter* writer, NSData* value, uint32_t tag)
{
	WriteKey(writer, tag, 2);
	WriteVarint(writer, [value length]);
	[writer->_data appendData: value];
}

void PBDataWriterWriteStringField(PBDataWriter* writer, NSString* string, uint32_t tag)
{
	PBDataWriterWriteDataField(writer, [string dataUsingEncoding: NSUTF8StringEncoding], tag);
}

void PBDataWriterWriteSubmessage(PBDataWriter* writer, PBCodable* codable, uint32_t tag)
{
	PBDataWriter* inner = [[PBDataWriter alloc] init];
	[codable writeTo: inner];
	PBDataWriterWriteDataField(writer, inner->_data, tag);
	[inner release];
}

void PBDataWriterWriteDoubleField(PBDataWriter* writer, double value, uint32_t tag)
{
	uint64_t bits;
	memcpy(&bits, &value, sizeof(bits));
	WriteKey(writer, tag, 1);
	WriteFixed(writer, bits, 8);
}

void PBDataWriterWriteFloatField(PBDataWriter* writer, float value, uint32_t tag)
{
	uint32_t bits;
	memcpy(&bits, &value, sizeof(bits));
	WriteKey(writer, tag, 5);
	WriteFixed(writer, bits, 4);
}

void PBDataWriterWriteFixed32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
	WriteKey(writer, tag, 5);
	WriteFixed(writer, (uint32_t)value, 4);
}

void PBDataWriterWriteFixed64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
	WriteKey(writer, tag, 1);
	WriteFixed(writer, (uint64_t)value, 8);
}

void PBDataWriterWriteSfixed32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
	PBDataWriterWriteFixed32Field(writer, value, tag);
}

void PBDataWriterWriteSfixed64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
	PBDataWriterWriteFixed64Field(writer, value, tag);
}

void PBDataWriterWriteInt32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, (uint64_t)(int64_t)value); // a negative value takes ten bytes
}

void PBDataWriterWriteInt64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, (uint64_t)value);
}

void PBDataWriterWriteUint32Field(PBDataWriter* writer, uint32_t value, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, value);
}

void PBDataWriterWriteUint64Field(PBDataWriter* writer, uint64_t value, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, value);
}

void PBDataWriterWriteSint32Field(PBDataWriter* writer, int32_t value, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, ((uint32_t)value << 1) ^ (uint32_t)(value >> 31)); // zigzag
}

void PBDataWriterWriteSint64Field(PBDataWriter* writer, int64_t value, uint32_t tag)
{
	WriteKey(writer, tag, 0);
	WriteVarint(writer, ((uint64_t)value << 1) ^ (uint64_t)(value >> 63));
}
