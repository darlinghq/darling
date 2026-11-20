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

// The TKSmartCardProtocol member for a protocol type T of ISO/IEC 7816-3, or nil for a T the enumeration has no member for.
static NSNumber *protocolForT(uint8_t t)
{
	switch (t) {
	case 0:
		return [NSNumber numberWithUnsignedInteger:TKSmartCardProtocolT0];
	case 1:
		return [NSNumber numberWithUnsignedInteger:TKSmartCardProtocolT1];
	case 15:
		return [NSNumber numberWithUnsignedInteger:TKSmartCardProtocolT15];
	default:
		return nil;
	}
}

@implementation TKSmartCardATR

// Fills the object from a parsed ATR, or releases it and returns nil.
- (instancetype)_initWithParsed:(const ctk_atr *)atr ok:(BOOL)ok
{
	if (!ok) {
		[self release];
		return nil;
	}
	if ((self = [super init]) != nil) {
		_bytes = [[NSData alloc] initWithBytes:atr->bytes length:atr->length];
		_historicalBytes = [[NSData alloc] initWithBytes:atr->bytes + atr->historical_offset length:atr->historical_length];

		// ISO/IEC 7816-3: TDi announces the protocol T of the groups after it, so the first group (TA1..TD1, the
		// global one) has no protocol and group i + 1 carries the T of TDi. The protocols are the T of each TDi in
		// order, each once, the first being the default; with no TD1 the default is T=0.
		// GUESS: a T that TKSmartCardProtocol has no member for (2 to 14) is left out of both.
		NSMutableArray *groups = [NSMutableArray array];
		NSMutableArray *protocols = [NSMutableArray array];
		for (size_t i = 0; i < atr->group_count; i++) {
			NSNumber *groupProtocol = i == 0 ? nil : protocolForT(atr->groups[i - 1].protocol);
			TKSmartCardATRInterfaceGroup *group = [[TKSmartCardATRInterfaceGroup alloc] _initWithGroup:&atr->groups[i] protocol:groupProtocol];
			[groups addObject:group];
			[group release];
			if (atr->groups[i].has_td) {
				NSNumber *announced = protocolForT(atr->groups[i].protocol);
				if (announced != nil && ![protocols containsObject:announced])
					[protocols addObject:announced];
			}
		}
		if (atr->group_count == 0 || !atr->groups[0].has_td)
			[protocols insertObject:protocolForT(0) atIndex:0];
		_groups = [groups copy];
		_protocols = [protocols copy];
		_historicalRecords = [[self _recordsFromHistorical:_historicalBytes] retain];
	}
	return self;
}

// ISO/IEC 7816-4, 8.2 (2005 edition): the first historical octet is the category indicator. With 00 the
// compact-TLV objects sit between it and a status indicator of three octets at the end, with 80 they run to
// the end. Other categories (10 is a DIR data reference, the rest proprietary or reserved) hold no compact-TLV objects.
// GUESS: a historical part that is not made of whole compact-TLV objects yields no records at all.
- (NSArray<TKCompactTLVRecord *> *)_recordsFromHistorical:(NSData *)historical
{
	const NSUInteger statusLength = 3;
	if (historical.length == 0)
		return [NSArray array];
	uint8_t category = ((const uint8_t *)historical.bytes)[0];
	NSRange range = NSMakeRange(1, historical.length - 1);
	if (category == 0x00) {
		if (range.length < statusLength)
			return [NSArray array];
		range.length -= statusLength;
	} else if (category != 0x80) {
		return [NSArray array];
	}
	return [TKCompactTLVRecord sequenceOfRecordsFromData:[historical subdataWithRange:range]] ?: [NSArray array];
}

// The block hands over the ATR octet by octet. A -1 from it is the documented way to report a failure and, like a
// stream that is not a valid ATR, makes the init return nil.
- (instancetype)initWithSource:(int (^)(void))source
{
	ctk_atr atr;
	BOOL ok = ctk_atr_parse(nextOctetFromBlock, source, &atr);
	return [self _initWithParsed:&atr ok:ok];
}

- (instancetype)initWithBytes:(NSData *)bytes
{
	struct octetFeed feed = { bytes.bytes, bytes.length, 0 };
	ctk_atr atr;
	// GUESS: the documentation only says that data without a valid ATR gives nil; octets after the ATR are
	// treated as making the data invalid.
	BOOL ok = ctk_atr_parse(nextOctetFromFeed, &feed, &atr) && feed.position == feed.length;
	return [self _initWithParsed:&atr ok:ok];
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

// The index counts groups the way ISO/IEC 7816-3 numbers TA1/TB1/TC1/TD1, from 1, so 0 and negative values find nothing
// (the documentation of interfaceGroup(at:) says the same).
- (TKSmartCardATRInterfaceGroup *)interfaceGroupAtIndex:(NSInteger)index
{
	return index >= 1 && (NSUInteger)index <= _groups.count ? _groups[index - 1] : nil;
}

// GUESS: the documentation does not say how the protocol is matched; the first group whose protocol is the given one is returned.
- (TKSmartCardATRInterfaceGroup *)interfaceGroupForProtocol:(TKSmartCardProtocol)protocol
{
	NSNumber *wanted = [NSNumber numberWithUnsignedInteger:protocol];
	for (TKSmartCardATRInterfaceGroup *group in _groups) {
		if ([group.protocol isEqual:wanted])
			return group;
	}
	return nil;
}

// GUESS: the documentation lists Equatable and Hashable without saying more; ATRs are equal when their octets are.
- (BOOL)isEqual:(id)other
{
	return other == self || ([other isKindOfClass:[TKSmartCardATR class]] && [_bytes isEqualToData:((TKSmartCardATR *)other)->_bytes]);
}

- (NSUInteger)hash
{
	return _bytes.hash;
}

@end
