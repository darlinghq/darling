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

#import <CryptoTokenKit/TKSmartCardATR.h>
#import <Foundation/Foundation.h>
#include "ctkatr.h"

@interface TKSmartCardATRInterfaceGroup ()
- (instancetype)_initWithGroup:(const ctk_atr_group *)group protocol:(NSNumber *)protocol;
@end

@implementation TKSmartCardATRInterfaceGroup

- (instancetype)_initWithGroup:(const ctk_atr_group *)group protocol:(NSNumber *)protocol
{
	if ((self = [super init]) != nil) {
		if (group->has_ta)
			_TA = [[NSNumber alloc] initWithUnsignedChar:group->ta];
		if (group->has_tb)
			_TB = [[NSNumber alloc] initWithUnsignedChar:group->tb];
		if (group->has_tc)
			_TC = [[NSNumber alloc] initWithUnsignedChar:group->tc];
		_protocol = [protocol retain];
	}
	return self;
}

- (void)dealloc
{
	[_TA release];
	[_TB release];
	[_TC release];
	[_protocol release];
	[super dealloc];
}

- (NSNumber *)TA
{
	return _TA;
}

- (NSNumber *)TB
{
	return _TB;
}

- (NSNumber *)TC
{
	return _TC;
}

- (NSNumber *)protocol
{
	return _protocol;
}

@end

static int nextOctetFromBlock(void *context)
{
	return ((int (^)(void))context)();
}

// Octets of a data object handed out one at a time; -1 once they are used up.
struct octetFeed {
	const uint8_t *bytes;
	NSUInteger length;
	NSUInteger position;
};

static int nextOctetFromFeed(void *context)
{
	struct octetFeed *feed = context;
	return feed->position < feed->length ? feed->bytes[feed->position++] : -1;
}

// The TKSmartCardProtocol value for a protocol type T of ISO/IEC 7816-3: the option bit 1 << T, which is how
// TKSmartCardProtocol's T0, T1 and T15 are numbered. The other types (2 to 14) get their bit too, as black-box
// observation of TKSmartCardATR on macOS 26.7.3 shows (tier 2; test: cryptotokenkit_atr.m "protocol bits").
static NSNumber *protocolForT(uint8_t t)
{
	return [NSNumber numberWithUnsignedInteger:(NSUInteger)1 << t];
}

@implementation TKSmartCardATR

// Fills the object from a parsed ATR, or releases it and returns nil. `bytes` is what the ATR property returns.
- (instancetype)_initWithParsed:(const ctk_atr *)atr bytes:(NSData *)bytes ok:(BOOL)ok
{
	if (!ok) {
		[self release];
		return nil;
	}
	if ((self = [super init]) != nil) {
		_bytes = [bytes copy];
		_historicalBytes = [[NSData alloc] initWithBytes:atr->bytes + atr->historical_offset length:atr->historical_length];

		// ISO/IEC 7816-3: TDi announces the protocol T of the groups after it, so the first group (TA1..TD1, the
		// global one) has no protocol and group i + 1 carries the T of TDi. There is a group after every TDi.
		NSMutableArray *groups = [NSMutableArray array];
		for (size_t i = 0; i < atr->group_count; i++) {
			NSNumber *groupProtocol = i == 0 ? nil : protocolForT(atr->groups[i - 1].protocol);
			TKSmartCardATRInterfaceGroup *group = [[TKSmartCardATRInterfaceGroup alloc] _initWithGroup:&atr->groups[i] protocol:groupProtocol];
			[groups addObject:group];
			[group release];
		}
		NSMutableArray *protocols = [NSMutableArray array];
		uint8_t types[16];
		size_t typeCount = ctk_atr_protocols(atr, types);
		for (size_t i = 0; i < typeCount; i++)
			[protocols addObject:protocolForT(types[i])];
		_groups = [groups copy];
		_protocols = [protocols copy];
		_historicalRecords = [[self _recordsFromHistorical:_historicalBytes] retain];
	}
	return self;
}

// ISO/IEC 7816-4, 8.2 (2005 edition): the first historical octet is the category indicator. With 80 the
// compact-TLV objects run to the end. With 00 they sit between it and a status indicator of three octets at the
// end, and the status indicator is returned as one more record, with tag 8 (the tag ISO/IEC 7816-4 gives it)
// and the three octets as value, after the others. Other categories (10 is a DIR data reference, the rest
// proprietary or reserved) hold no compact-TLV objects.
// A historical part that is not made of whole compact-TLV objects, or category 00 without room for the status
// indicator, yields no records at all. The status indicator record and these rules come from black-box
// observation of TKSmartCardATR on macOS 26.7.3 (tier 2); test: cryptotokenkit_atr.m, "historical records".
- (NSArray<TKCompactTLVRecord *> *)_recordsFromHistorical:(NSData *)historical
{
	const NSUInteger statusLength = 3;
	if (historical.length == 0)
		return [NSArray array];
	uint8_t category = ((const uint8_t *)historical.bytes)[0];
	NSRange range = NSMakeRange(1, historical.length - 1);
	NSData *status = nil;
	if (category == 0x00) {
		if (range.length < statusLength)
			return [NSArray array];
		range.length -= statusLength;
		status = [historical subdataWithRange:NSMakeRange(historical.length - statusLength, statusLength)];
	} else if (category != 0x80) {
		return [NSArray array];
	}
	NSArray *records = [TKCompactTLVRecord sequenceOfRecordsFromData:[historical subdataWithRange:range]];
	if (records == nil)
		return [NSArray array];
	if (status == nil)
		return records;
	TKCompactTLVRecord *statusRecord = [[[TKCompactTLVRecord alloc] initWithTag:8 value:status] autorelease];
	return [records arrayByAddingObject:statusRecord];
}

- (instancetype)initWithSource:(int (^)(void))source
{
	ctk_atr atr;
	BOOL ok = ctk_atr_parse(nextOctetFromBlock, source, &atr);
	return [self _initWithParsed:&atr bytes:[NSData dataWithBytes:atr.bytes length:atr.length] ok:ok];
}

- (instancetype)initWithBytes:(NSData *)bytes
{
	struct octetFeed feed = { bytes.bytes, bytes.length, 0 };
	ctk_atr atr;
	// Octets after the ATR are allowed and kept in `bytes`, as black-box observation of TKSmartCardATR on macOS 26.7.3
	// shows (tier 2; test: cryptotokenkit_atr.m, "octets after the ATR").
	BOOL ok = ctk_atr_parse(nextOctetFromFeed, &feed, &atr);
	return [self _initWithParsed:&atr bytes:bytes ok:ok];
}

- (void)dealloc
{
	[_bytes release];
	[_historicalBytes release];
	[_historicalRecords release];
	[_protocols release];
	[_groups release];
	[super dealloc];
}

- (NSData *)bytes
{
	return _bytes;
}

- (NSData *)historicalBytes
{
	return _historicalBytes;
}

- (NSArray<TKCompactTLVRecord *> *)historicalRecords
{
	return _historicalRecords;
}

- (NSArray<NSNumber *> *)protocols
{
	return _protocols;
}

// The index counts groups the way ISO/IEC 7816-3 numbers TA1/TB1/TC1/TD1, from 1. An index above the number of
// groups, or negative, finds nothing.
// QUIRK: index 0 raises NSRangeException instead of finding nothing. Not documented. Evidence: black-box
// observation of TKSmartCardATR on macOS 26.7.3 (tier 2), where the index minus one is passed to an array.
// Test: cryptotokenkit_atr.m, "interface group index 0".
- (TKSmartCardATRInterfaceGroup *)interfaceGroupAtIndex:(NSInteger)index
{
	if (index == 0)
		[NSException raise:NSRangeException format:@"interface group index 0 is out of range (indexes start at 1)"];
	return index >= 1 && (NSUInteger)index <= _groups.count ? _groups[index - 1] : nil;
}

// The first group whose protocol is the given one (a single TKSmartCardProtocol value, as in observed results).
// QUIRK: the second group, the one after TD1, is found by its protocol only when that is T=0; it is skipped for
// every other protocol although its `protocol` property has it. Not documented. Evidence: black-box observation
// of TKSmartCardATR on macOS 26.7.3 (tier 2), over every pair and triple of TDi with T in {0, 1, 2, 15}.
// Test: cryptotokenkit_atr.m, "interfaceGroupForProtocol skips group 2".
- (TKSmartCardATRInterfaceGroup *)interfaceGroupForProtocol:(TKSmartCardProtocol)protocol
{
	NSNumber *wanted = [NSNumber numberWithUnsignedInteger:protocol];
	for (NSUInteger i = 1; i < _groups.count; i++) {
		TKSmartCardATRInterfaceGroup *group = _groups[i];
		if (i == 1 && protocol != TKSmartCardProtocolT0)
			continue;
		if ([group.protocol isEqual:wanted])
			return group;
	}
	return nil;
}

@end
