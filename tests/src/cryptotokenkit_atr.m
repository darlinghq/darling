// CryptoTokenKit TKSmartCardATR. The ATRs are built from the layout of ISO/IEC 7816-3 clause 8 (TS, T0, TAi..TDi,
// historical octets, TCK = XOR of T0..last octet before TCK) and ISO/IEC 7816-4 8.2 / 8.3 (category indicator and
// Compact-TLV historical records, 2005 edition numbering). Behaviours that neither the documentation nor the standards settle are marked GUESS.
// Prints nothing on success; the exit code is the number of failed checks.
#import <Foundation/Foundation.h>
#import <CryptoTokenKit/CryptoTokenKit.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)
#define DATA(...) ({ const unsigned char b[] = { __VA_ARGS__ }; [NSData dataWithBytes:b length:sizeof b]; })

static TKSmartCardATR *atr(NSData *data) { return [[[TKSmartCardATR alloc] initWithBytes:data] autorelease]; }

int main(void)
{
	@autoreleasepool {
		NSNumber *T0 = [NSNumber numberWithUnsignedInteger:TKSmartCardProtocolT0];
		NSNumber *T1 = [NSNumber numberWithUnsignedInteger:TKSmartCardProtocolT1];

		// TS and T0 only. ISO/IEC 7816-3: with no TD1, T=0 is the only and the default protocol (the documentation
		// says the default protocol is at index 0 of `protocols`).
		TKSmartCardATR *minimal = atr(DATA(0x3b, 0x00));
		CHECK(minimal != nil && [minimal.bytes isEqualToData:DATA(0x3b, 0x00)]);
		CHECK(minimal.historicalBytes.length == 0 && minimal.historicalRecords.count == 0);
		CHECK([minimal.protocols isEqualToArray:[NSArray arrayWithObject:T0]]);
		CHECK([minimal interfaceGroupAtIndex:1] == nil);
		CHECK(atr(DATA(0x3f, 0x00)) != nil);

		// T0 = 91: TA1 = 11, TD1 = 01 (T=1), one historical octet 42, TCK = 91 ^ 11 ^ 01 ^ 42 = C3.
		TKSmartCardATR *t1 = atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3));
		CHECK(t1 != nil && [t1.historicalBytes isEqualToData:DATA(0x42)]);
		// The protocols are the values of TKSmartCardProtocol, not the raw T.
		CHECK([t1.protocols isEqualToArray:[NSArray arrayWithObject:T1]]);
		TKSmartCardATRInterfaceGroup *group = [t1 interfaceGroupAtIndex:1];
		// The first interface group is the global one: it has TA1 but no protocol (documented on InterfaceGroup.protocol).
		CHECK(group != nil && [group.TA intValue] == 0x11 && group.TB == nil && group.TC == nil && group.protocol == nil);
		// Interface group indexes start at 1 (documented on interfaceGroup(at:)).
		CHECK([t1 interfaceGroupAtIndex:0] == nil && [t1 interfaceGroupAtIndex:2] == nil && [t1 interfaceGroupAtIndex:-1] == nil);
		// TD1 announces T=1 for the groups after it; there are none, so no group has a protocol.
		CHECK([t1 interfaceGroupForProtocol:TKSmartCardProtocolT1] == nil);
		CHECK([t1 interfaceGroupForProtocol:TKSmartCardProtocolT0] == nil);
		CHECK([t1 interfaceGroupForProtocol:TKSmartCardProtocolAny] == nil);
		CHECK([t1 isEqual:atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3))] && t1.hash == atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3)).hash);
		CHECK(![t1 isEqual:minimal]);

		// Wrong TCK, missing TCK, bad TS, trailing octets, empty data.
		CHECK(atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc4)) == nil);
		CHECK(atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42)) == nil);
		CHECK(atr(DATA(0x3a, 0x00)) == nil);
		// GUESS: octets after the ATR make the data invalid (the documentation only says invalid data gives nil).
		CHECK(atr(DATA(0x3b, 0x00, 0x00)) == nil);
		CHECK(atr([NSData data]) == nil);

		// T0 = 80, TD1 = 80 (T=0, TD2 follows), TD2 = 01 (T=1), TCK = 01. Two groups; group 2 carries the T=0 of TD1.
		TKSmartCardATR *chain = atr(DATA(0x3b, 0x80, 0x80, 0x01, 0x01));
		CHECK(([chain.protocols isEqualToArray:[NSArray arrayWithObjects:T0, T1, nil]]));
		CHECK([chain interfaceGroupAtIndex:1].protocol == nil && [[chain interfaceGroupAtIndex:2].protocol isEqual:T0]);
		CHECK([chain interfaceGroupForProtocol:TKSmartCardProtocolT0] == [chain interfaceGroupAtIndex:2]);
		CHECK([chain interfaceGroupForProtocol:TKSmartCardProtocolT1] == nil);

		// T0 = 80, TD1 = 91 (T=1, TA2 and TD2 follow), TA2 = 20, TD2 = 00 (T=0), TCK = 80 ^ 91 ^ 20 ^ 00 = 31.
		// The first announced protocol is the default, so it is at index 0.
		TKSmartCardATR *t1first = atr(DATA(0x3b, 0x80, 0x91, 0x20, 0x00, 0x31));
		CHECK(([t1first.protocols isEqualToArray:[NSArray arrayWithObjects:T1, T0, nil]]));
		group = [t1first interfaceGroupAtIndex:2];
		CHECK([group.TA intValue] == 0x20 && [group.protocol isEqual:T1] && [t1first interfaceGroupForProtocol:TKSmartCardProtocolT1] == group);

		// T0 = 60: TB1 and TC1 without TD1: group 1 only, and T=0 is the default protocol.
		TKSmartCardATR *tbtc = atr(DATA(0x3b, 0x60, 0x25, 0x03));
		group = [tbtc interfaceGroupAtIndex:1];
		CHECK([group.TB intValue] == 0x25 && [group.TC intValue] == 3 && group.TA == nil && group.protocol == nil);
		CHECK([tbtc.protocols isEqualToArray:[NSArray arrayWithObject:T0]]);

		// Historical records (7816-4 8.2): category 80, records to the end.
		TKSmartCardATR *cat80 = atr(DATA(0x3b, 0x03, 0x80, 0x31, 0xc0));
		CHECK(cat80.historicalRecords.count == 1 && [cat80.historicalRecords[0] tag] == 3);
		CHECK([[cat80.historicalRecords[0] value] isEqualToData:DATA(0xc0)]);
		// Category 00: records, then a 3-octet status indicator at the end.
		TKSmartCardATR *cat00 = atr(DATA(0x3b, 0x06, 0x00, 0x31, 0xc0, 0x00, 0x90, 0x00));
		CHECK(cat00.historicalRecords.count == 1 && [cat00.historicalRecords[0] tag] == 3);
		// Category 10 (DIR data reference) holds no compact-TLV records.
		CHECK(atr(DATA(0x3b, 0x03, 0x10, 0x31, 0xc0)).historicalRecords.count == 0);
		// GUESS: historical octets that are not whole Compact-TLV objects give no records.
		CHECK(atr(DATA(0x3b, 0x03, 0x80, 0x35, 0xc0)).historicalRecords.count == 0);

		// initWithSource: pulls exactly one ATR, then stops; a block that returns -1 (documented as the error signal) gives nil.
		static const unsigned char stream[] = { 0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3, 0xde, 0xad };
		__block size_t pos = 0;
		TKSmartCardATR *streamed = [[[TKSmartCardATR alloc] initWithSource:^int { return pos < sizeof stream ? stream[pos++] : -1; }] autorelease];
		CHECK(streamed != nil && [streamed isEqual:t1] && pos == 6);
		pos = 0;
		TKSmartCardATR *dry = [[[TKSmartCardATR alloc] initWithSource:^int { return pos < 4 ? stream[pos++] : -1; }] autorelease];
		CHECK(dry == nil);
	}
	return failures;
}
