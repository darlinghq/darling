#import <CryptoTokenKit/TKTLVRecord.h>
#import <Foundation/Foundation.h>
#include "ctkcodec.h"

@interface TKTLVRecord ()
- (instancetype)_initWithTag:(TKTLVTag)tag value:(NSData *)value data:(NSData *)data;
// Decodes the record at the start of the buffer. Each encoding overrides this; the base class has none.
+ (nullable instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed;
@end

static BOOL decodeRecord(const uint8_t *bytes, size_t length, bool (*decode)(const uint8_t *, size_t, ctk_tlv *),
                         size_t *consumed, TKTLVTag *tag, NSData **value, NSData **data)
{
	ctk_tlv item;
	// TKTLVTag is only as wide as NSUInteger, so a tag that does not fit (more than 4 octets on 32-bit) is rejected.
	if (!decode(bytes, length, &item) || (uint64_t)(TKTLVTag)item.tag != item.tag)
		return NO;
	*consumed = item.total_length;
	*tag = (TKTLVTag)item.tag;
	*value = [NSData dataWithBytes:bytes + item.value_offset length:item.value_length];
	*data = [NSData dataWithBytes:bytes length:item.total_length];
	return YES;
}

@implementation TKTLVRecord

+ (instancetype)recordFromData:(NSData *)data
{
	size_t consumed = 0;
	TKTLVRecord *record = [self _recordFromBytes:data.bytes length:data.length consumed:&consumed];
	// Trailing octets after the record mean the data is not exactly one record.
	return consumed == data.length ? record : nil;
}

+ (NSArray<TKTLVRecord *> *)sequenceOfRecordsFromData:(NSData *)data
{
	const uint8_t *bytes = data.bytes;
	size_t length = data.length;
	NSMutableArray<TKTLVRecord *> *records = [NSMutableArray array];
	while (length > 0) {
		size_t consumed = 0;
		TKTLVRecord *record = [self _recordFromBytes:bytes length:length consumed:&consumed];
		if (record == nil)
			return nil;
		[records addObject:record];
		bytes += consumed;
		length -= consumed;
	}
	return records;
}

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed
{
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

// GUESS: the documentation lists Equatable and Hashable for the records without saying what equal means;
// two records of one class are equal when their encodings are.
- (BOOL)isEqual:(id)other
{
	return other == self || ([other isKindOfClass:[TKTLVRecord class]] && [other class] == [self class] &&
	                         [_data isEqualToData:((TKTLVRecord *)other)->_data]);
}

- (NSUInteger)hash
{
	return _data.hash;
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

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed
{
	TKTLVTag tag;
	NSData *value, *data;
	if (!decodeRecord(bytes, length, ctk_ber_decode, consumed, &tag, &value, &data))
		return nil;
	return [[[self alloc] _initWithTag:tag value:value data:data] autorelease];
}

// GUESS: the tag is taken as the integer whose big-endian octets are the identifier octets, so a tag
// written 7F21 in ISO/IEC 7816 is 0x7f21. The documentation only says "BER-TLV tag encoding rules".
// A tag that is not well-formed identifier octets (X.690 8.1.2), such as 0x1f or 0x0101, would encode to bytes
// that do not read back as that tag, so it raises NSInvalidArgumentException (GUESS: the documentation does not say).
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

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed
{
	TKTLVTag tag;
	NSData *value, *data;
	if (!decodeRecord(bytes, length, ctk_simple_decode, consumed, &tag, &value, &data))
		return nil;
	return [[[self alloc] _initWithTag:tag value:value data:data] autorelease];
}

// GUESS: a tag or value the Simple-TLV encoding cannot represent (tag 00 or FF, more than 65535 value octets)
// raises NSInvalidArgumentException; the documentation does not say what happens.
- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value
{
	if (!ctk_simple_tag_valid(tag) || value.length > CTK_SIMPLE_MAX_LENGTH) {
		[self release];
		[NSException raise:NSInvalidArgumentException format:@"TKSimpleTLVRecord cannot encode tag %#x with %lu value bytes",
		                   tag, (unsigned long)value.length];
	}
	NSMutableData *data = [NSMutableData dataWithBytes:&tag length:1];
	uint8_t length[3];
	[data appendBytes:length length:ctk_simple_encode_length(value.length, length)];
	[data appendData:value];
	return [super _initWithTag:tag value:value data:data];
}

@end

@implementation TKCompactTLVRecord

+ (instancetype)_recordFromBytes:(const uint8_t *)bytes length:(size_t)length consumed:(size_t *)consumed
{
	TKTLVTag tag;
	NSData *value, *data;
	if (!decodeRecord(bytes, length, ctk_compact_decode, consumed, &tag, &value, &data))
		return nil;
	return [[[self alloc] _initWithTag:tag value:value data:data] autorelease];
}

// GUESS: the tag is the 4-bit tag number (0..15) of ISO/IEC 7816-4, not the whole first octet, and a tag or
// value that does not fit 4 bits raises NSInvalidArgumentException.
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
