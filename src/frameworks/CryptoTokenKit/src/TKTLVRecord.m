/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#import <CryptoTokenKit/TKTLVRecord.h>
#import <Foundation/Foundation.h>
#include "ctkcodec.h"

@interface TKTLVRecord ()
- (instancetype)_initWithTag:(TKTLVTag)tag value:(NSData *)value data:(NSData *)data;
// Decodes the record at the start of the buffer. Each encoding overrides this; the base class has none.
+ (nullable instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed indefinite:(BOOL *)indefinite;
@end

static BOOL decodeRecord(const uint8_t *bytes, size_t length, bool (*decode)(const uint8_t *, size_t, ctk_tlv *),
                         size_t *consumed, BOOL *indefinite, TKTLVTag *tag, NSData **value, NSData **data)
{
	ctk_tlv item;
	// TKTLVTag is only as wide as NSUInteger, so a tag that does not fit (more than 4 octets on 32-bit) is rejected.
	if (!decode(bytes, length, &item) || (uint64_t)(TKTLVTag)item.tag != item.tag)
		return NO;
	*indefinite = item.indefinite;
	*consumed = item.total_length;
	*tag = (TKTLVTag)item.tag;
	*value = [NSData dataWithBytes:bytes + item.value_offset length:item.value_length];
	*data = [NSData dataWithBytes:bytes length:item.total_length];
	return YES;
}

@implementation TKTLVRecord

// The first record of the data; octets after it are ignored (black-box observation of TKBERTLVRecord,
// TKSimpleTLVRecord and TKCompactTLVRecord on macOS 26.7.3, tier 2; test: cryptotokenkit_tlv.m "octets after
// the record").
+ (instancetype)recordFromData:(NSData *)data
{
	size_t consumed = 0;
	BOOL indefinite = NO;
	TKTLVRecord *record = [self _recordFromBytes:data.bytes length:data.length consumed:&consumed indefinite:&indefinite];
	// QUIRK: a record of the indefinite length form has to be the last thing in the data, while octets after a
	// definite-length record are left alone. Not documented. Evidence: black-box observation of
	// TKBERTLVRecord on macOS 26.7.3 (tier 2), for +recordFromData: only, +sequenceOfRecordsFromData: reads a run of
	// them. Test: cryptotokenkit_tlv.m, "indefinite length".
	return indefinite && consumed != data.length ? nil : record;
}

+ (NSArray<TKTLVRecord *> *)sequenceOfRecordsFromData:(NSData *)data
{
	const uint8_t *bytes = data.bytes;
	size_t length = data.length;
	NSMutableArray<TKTLVRecord *> *records = [NSMutableArray array];
	while (length > 0) {
		size_t consumed = 0;
		BOOL indefinite = NO;
		TKTLVRecord *record = [self _recordFromBytes:bytes length:length consumed:&consumed indefinite:&indefinite];
		if (record == nil)
			return nil;
		[records addObject:record];
		bytes += consumed;
		length -= consumed;
	}
	return records;
}

// QUIRK: the abstract class raises NSInternalInconsistencyException when asked to decode, instead of returning
// nil; only an empty sequence is accepted by +sequenceOfRecordsFromData: since it decodes nothing. Not
// documented. Evidence: black-box observation of TKTLVRecord on macOS 26.7.3 (tier 2).
// Test: cryptotokenkit_tlv.m, "abstract class".
+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed indefinite:(BOOL *)indefinite
{
	[NSException raise:NSInternalInconsistencyException format:@"%@ cannot decode records, use a subclass", NSStringFromClass(self)];
	return nil;
}

- (instancetype)_initWithTag:(TKTLVTag)tag value:(NSData *)value data:(NSData *)data
{
	if ((self = [super init]) != nil) {
		_tag = tag;
		_value = [value copy];
		_data = [data copy];
	}
	return self;
}

- (void)dealloc
{
	[_value release];
	[_data release];
	[super dealloc];
}

- (TKTLVTag)tag
{
	return _tag;
}

- (NSData *)value
{
	return _value;
}

- (NSData *)data
{
	return _data;
}

@end

static NSData *concatenatedData(NSArray<TKTLVRecord *> *records)
{
	NSMutableData *value = [NSMutableData data];
	for (TKTLVRecord *record in records)
		[value appendData:record.data];
	return value;
}

@implementation TKBERTLVRecord

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed indefinite:(BOOL *)indefinite
{
	TKTLVTag tag;
	NSData *value, *data;
	if (!decodeRecord(bytes, length, ctk_ber_decode, consumed, indefinite, &tag, &value, &data))
		return nil;
	return [[[self alloc] _initWithTag:tag value:value data:data] autorelease];
}

// The tag is the integer whose big-endian octets are the identifier octets, so a tag written 7F21 in ISO/IEC 7816
// is 0x7f21. Tag 0 raises NSInvalidArgumentException; every other integer encodes as it stands (see
// ctk_ber_tag_valid).
+ (NSData *)dataForTag:(TKTLVTag)tag
{
	if (!ctk_ber_tag_valid(tag))
		[NSException raise:NSInvalidArgumentException format:@"%#lx is not a BER-TLV tag", (unsigned long)tag];
	uint8_t octets[CTK_BER_MAX_TAG_OCTETS];
	size_t count = ctk_ber_encode_tag(tag, octets);
	return [NSData dataWithBytes:octets length:count];
}

// The documentation says BER is parsed and DER is produced, so the length is the shortest definite one.
+ (NSData *)_encodedWithTag:(TKTLVTag)tag value:(NSData *)value
{
	NSMutableData *data = [NSMutableData dataWithData:[self dataForTag:tag]];
	uint8_t length[CTK_BER_MAX_LENGTH_OCTETS];
	[data appendBytes:length length:ctk_ber_encode_length(value.length, length)];
	[data appendData:value];
	return data;
}

- (instancetype)initWithTag:(TKTLVTag)tag value:(NSData *)value
{
	if (!ctk_ber_tag_valid(tag)) {
		[self release];
		[NSException raise:NSInvalidArgumentException format:@"%#lx is not a BER-TLV tag", (unsigned long)tag];
	}
	return [super _initWithTag:tag value:value data:[TKBERTLVRecord _encodedWithTag:tag value:value]];
}

- (instancetype)initWithTag:(TKTLVTag)tag records:(NSArray<TKTLVRecord *> *)records
{
	return [self initWithTag:tag value:concatenatedData(records)];
}

@end

@implementation TKSimpleTLVRecord

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed indefinite:(BOOL *)indefinite
{
	TKTLVTag tag;
	NSData *value, *data;
	if (!decodeRecord(bytes, length, ctk_simple_decode, consumed, indefinite, &tag, &value, &data))
		return nil;
	return [[[self alloc] _initWithTag:tag value:value data:data] autorelease];
}

// More than 65535 value octets raises NSInvalidArgumentException (the length does not fit the encoding). Every tag
// value is encoded as given, including 00 and FF, which the decoder does not accept (black-box observation of
// TKSimpleTLVRecord on macOS 26.7.3, tier 2; test: cryptotokenkit_tlv.m "Simple tags 00 and FF").
- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value
{
	if (value.length > CTK_SIMPLE_MAX_LENGTH) {
		[self release];
		[NSException raise:NSInvalidArgumentException format:@"TKSimpleTLVRecord cannot encode %lu value bytes",
		                   (unsigned long)value.length];
	}
	NSMutableData *data = [NSMutableData dataWithBytes:&tag length:1];
	uint8_t length[3];
	[data appendBytes:length length:ctk_simple_encode_length(value.length, length)];
	[data appendData:value];
	return [super _initWithTag:tag value:value data:data];
}

@end

@implementation TKCompactTLVRecord

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed indefinite:(BOOL *)indefinite
{
	TKTLVTag tag;
	NSData *value, *data;
	if (!decodeRecord(bytes, length, ctk_compact_decode, consumed, indefinite, &tag, &value, &data))
		return nil;
	return [[[self alloc] _initWithTag:tag value:value data:data] autorelease];
}

// The tag is the 4-bit tag number (0..15) of ISO/IEC 7816-4, not the whole first octet, and a tag or value that
// does not fit 4 bits raises NSInvalidArgumentException.
- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value
{
	if (tag > CTK_COMPACT_MAX_TAG || value.length > CTK_COMPACT_MAX_LENGTH) {
		[self release];
		[NSException raise:NSInvalidArgumentException format:@"TKCompactTLVRecord cannot encode tag %u with %lu value bytes",
		                   tag, (unsigned long)value.length];
	}
	uint8_t header = (uint8_t)((tag << 4) | value.length);
	NSMutableData *data = [NSMutableData dataWithBytes:&header length:1];
	[data appendData:value];
	return [super _initWithTag:tag value:value data:data];
}

@end
