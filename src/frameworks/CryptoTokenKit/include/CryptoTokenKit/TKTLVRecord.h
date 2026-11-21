/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NSUInteger TKTLVTag;

// One tag-length-value record. The subclasses differ in how the tag and length are encoded.
API_AVAILABLE(macos(10.10))
@interface TKTLVRecord : NSObject
{
	// Declared here, not in the implementation: the i386 slice uses the
	// legacy Objective-C ABI, which needs every ivar in the interface.
@private
	TKTLVTag _tag;
	NSData *_value;
	NSData *_data;
}

// The tag, the value without tag and length, and the record as encoded.
@property (nonatomic, readonly) TKTLVTag tag;
@property (nonatomic, readonly) NSData *value;
@property (nonatomic, readonly) NSData *data;

// Parses one record; nil if the data is not a well-formed record of this encoding.
+ (nullable instancetype)recordFromData:(NSData *)data;
// Parses a concatenation of records; nil if any of them is malformed.
+ (nullable NSArray<TKTLVRecord *> *)sequenceOfRecordsFromData:(NSData *)data;

@end

// BER-TLV (ISO 7816-4 / ASN.1 BER): multi-byte tags, definite lengths.
API_AVAILABLE(macos(10.10))
@interface TKBERTLVRecord : TKTLVRecord

+ (NSData *)dataForTag:(TKTLVTag)tag;

- (instancetype)initWithTag:(TKTLVTag)tag value:(NSData *)value;
- (instancetype)initWithTag:(TKTLVTag)tag records:(NSArray<TKTLVRecord *> *)records;

@end

// Simple-TLV (ISO 7816-4): one-byte tag.
API_AVAILABLE(macos(10.10))
@interface TKSimpleTLVRecord : TKTLVRecord

- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value;
@end

// Compact-TLV (ISO 7816-4): the tag and the length share one byte. Used for the historical bytes of an ATR.
API_AVAILABLE(macos(10.10))
@interface TKCompactTLVRecord : TKTLVRecord

- (instancetype)initWithTag:(UInt8)tag value:(NSData *)value;
@end

NS_ASSUME_NONNULL_END
