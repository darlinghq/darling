// CryptoTokenKit TKSmartCardATR. The ATRs are built from the layout of ISO/IEC 7816-3 clause 8 (TS, T0, TAi..TDi,
// historical octets, TCK = XOR of T0..last octet before TCK) and ISO/IEC 7816-4 8.2 / 8.3 (category indicator and
// Compact-TLV historical records, 2005 edition numbering). Where the standards and the documentation do not
// settle a result, it is the black-box observation of macOS 26.7.3 (x86_64): every check passes on that system
// unchanged. The checks that depend on such an observation are marked "observed".
// Prints nothing on success; the exit code is the number of failed checks.
#import <Foundation/Foundation.h>
#import <CryptoTokenKit/CryptoTokenKit.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)
#define DATA(...) ({ const unsigned char b[] = { __VA_ARGS__ }; [NSData dataWithBytes:b length:sizeof b]; })

static TKSmartCardATR *atr(NSData *data) { return [[[TKSmartCardATR alloc] initWithBytes:data] autorelease]; }

static BOOL raisesRange(void (^block)(void))
{
	@try { block(); } @catch (NSException *e) { return [e.name isEqualToString:NSRangeException]; }
	return NO;
}

static NSNumber *num(NSUInteger v) { return [NSNumber numberWithUnsignedInteger:v]; }

int main(void)
{
	@autoreleasepool {
		NSNumber *T0 = num(TKSmartCardProtocolT0);
		NSNumber *T1 = num(TKSmartCardProtocolT1);

		// TS and T0 only. ISO/IEC 7816-3: with no TD1, T=0 is the only and the default protocol (the documentation
		// says the default protocol is at index 0 of `protocols`). There is always a first interface group.
		TKSmartCardATR *minimal = atr(DATA(0x3b, 0x00));
		CHECK(minimal != nil && [minimal.bytes isEqualToData:DATA(0x3b, 0x00)]);
		CHECK(minimal.historicalBytes.length == 0 && minimal.historicalRecords.count == 0);
		CHECK([minimal.protocols isEqualToArray:[NSArray arrayWithObject:T0]]);
		TKSmartCardATRInterfaceGroup *first = [minimal interfaceGroupAtIndex:1];
		CHECK(first != nil && first.TA == nil && first.TB == nil && first.TC == nil && first.protocol == nil);
		CHECK([minimal interfaceGroupAtIndex:2] == nil);
		// Observed: the value of TS is not checked.
		CHECK(atr(DATA(0x3f, 0x00)) != nil && atr(DATA(0x3a, 0x00)) != nil);

		// T0 = 91: TA1 = 11, TD1 = 01 (T=1), one historical octet 42, TCK = 91 ^ 11 ^ 01 ^ 42 = C3.
		TKSmartCardATR *t1 = atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3));
		CHECK(t1 != nil && [t1.historicalBytes isEqualToData:DATA(0x42)]);
		// The protocols are the values of TKSmartCardProtocol, not the raw T.
		CHECK([t1.protocols isEqualToArray:[NSArray arrayWithObject:T1]]);
		// The first interface group is the global one: it has TA1 but no protocol (documented on InterfaceGroup.protocol).
		TKSmartCardATRInterfaceGroup *group = [t1 interfaceGroupAtIndex:1];
		CHECK(group != nil && [group.TA intValue] == 0x11 && group.TB == nil && group.TC == nil && group.protocol == nil);
		// Observed: a group follows every TDi, carrying the T of that TDi, even when it holds no interface bytes.
		group = [t1 interfaceGroupAtIndex:2];
		CHECK(group != nil && group.TA == nil && [group.protocol isEqual:T1]);
		CHECK([t1 interfaceGroupAtIndex:3] == nil && [t1 interfaceGroupAtIndex:-1] == nil);
		// QUIRK test, "interfaceGroupForProtocol skips group 2": the group after TD1 matches only T=0 (observed), so
		// the T=1 group 2 here is not found by its protocol, while group 3 (after TD2) would be.
		CHECK([t1 interfaceGroupForProtocol:TKSmartCardProtocolT1] == nil);
		CHECK([t1 interfaceGroupForProtocol:TKSmartCardProtocolT0] == nil);
		CHECK([t1 interfaceGroupForProtocol:TKSmartCardProtocolAny] == nil);
		// QUIRK test, "interface group index 0": index 0 raises NSRangeException (observed).
		CHECK(raisesRange(^{ [t1 interfaceGroupAtIndex:0]; }));
		// ATRs are compared by identity (observed).
		TKSmartCardATR *same = atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3));
		CHECK(![t1 isEqual:same] && [t1 isEqual:t1]);

		// TCK is required when a TDi announces a protocol other than T=0 and its value is not checked (observed).
		CHECK(atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42)) == nil);
		CHECK(atr(DATA(0x3b, 0x91, 0x11, 0x01, 0x42, 0xc4)) != nil);
		CHECK(atr(DATA(0x3b, 0x80, 0x00)) != nil);
		CHECK(atr(DATA(0x3b, 0x80, 0x0f)) == nil && atr(DATA(0x3b, 0x80, 0x0f, 0x00)) != nil);
		CHECK(atr(DATA(0x3b, 0x80, 0x02)) == nil);
		// Empty data, and data cut off in the interface bytes or the historical bytes.
		CHECK(atr([NSData data]) == nil && atr(DATA(0x3b)) == nil);
		CHECK(atr(DATA(0x3b, 0x10)) == nil && atr(DATA(0x3b, 0x05, 0x01, 0x02)) == nil);
		// Observed: octets after the ATR are allowed, and the bytes property is all the data given.
		CHECK([atr(DATA(0x3b, 0x00, 0x00)).bytes isEqualToData:DATA(0x3b, 0x00, 0x00)]);
		CHECK([atr(DATA(0x3b, 0x00, 0x00)).historicalBytes length] == 0);

		// T0 = 80, TD1 = 80 (T=0, TD2 follows), TD2 = 01 (T=1), TCK = 01. Three groups: group 2 carries the T=0 of
		// TD1, group 3 the T=1 of TD2. Observed: T=1 comes first in `protocols`, then the others in order.
		TKSmartCardATR *chain = atr(DATA(0x3b, 0x80, 0x80, 0x01, 0x01));
		CHECK(([chain.protocols isEqualToArray:[NSArray arrayWithObjects:T1, T0, nil]]));
		CHECK([chain interfaceGroupAtIndex:1].protocol == nil && [[chain interfaceGroupAtIndex:2].protocol isEqual:T0] && [[chain interfaceGroupAtIndex:3].protocol isEqual:T1]);
		CHECK([chain interfaceGroupForProtocol:TKSmartCardProtocolT0] == [chain interfaceGroupAtIndex:2]);
		CHECK([chain interfaceGroupForProtocol:TKSmartCardProtocolT1] == [chain interfaceGroupAtIndex:3]);
		TKSmartCardATR *twice = atr(DATA(0x3b, 0x80, 0x81, 0x01, 0x00));
		CHECK([[twice interfaceGroupAtIndex:2].protocol isEqual:T1] && [[twice interfaceGroupAtIndex:3].protocol isEqual:T1]);
		CHECK([twice interfaceGroupForProtocol:TKSmartCardProtocolT1] == [twice interfaceGroupAtIndex:3]);
		TKSmartCardATR *reversed = atr(DATA(0x3b, 0x80, 0x81, 0x00, 0x01));
		CHECK(([reversed.protocols isEqualToArray:[NSArray arrayWithObjects:T1, T0, nil]]));

		// T=15 is not offered in `protocols`; types 2 to 14 are, as the option bit 1 << T ("protocol bits", observed).
		TKSmartCardATR *t15 = atr(DATA(0x3b, 0x80, 0x8f, 0x01, 0x8f));
		CHECK([t15.protocols isEqualToArray:[NSArray arrayWithObject:T1]]);
		CHECK([[t15 interfaceGroupAtIndex:2].protocol isEqual:num(TKSmartCardProtocolT15)] && TKSmartCardProtocolT15 == 0x8000);
		CHECK([atr(DATA(0x3b, 0x80, 0x0f, 0x00)).protocols isEqualToArray:[NSArray arrayWithObject:T0]]);
		TKSmartCardATR *t2 = atr(DATA(0x3b, 0x80, 0x02, 0x82));
		CHECK([t2.protocols isEqualToArray:[NSArray arrayWithObject:num(4)]] && [[t2 interfaceGroupAtIndex:2].protocol isEqual:num(4)]);
		CHECK(([atr(DATA(0x3b, 0x80, 0x82, 0x01, 0x80)).protocols isEqualToArray:[NSArray arrayWithObjects:T1, num(4), nil]]));

		// TA2 present (specific mode): its low nibble is the only protocol (observed). T0 = 80, TD1 = 91 (T=1, TA2 and
		// TD2 follow), TA2 = 20, TD2 = 00, TCK = 31: T=0 although TD1 says T=1.
		TKSmartCardATR *specific = atr(DATA(0x3b, 0x80, 0x91, 0x20, 0x00, 0x31));
		CHECK([specific.protocols isEqualToArray:[NSArray arrayWithObject:T0]]);
		group = [specific interfaceGroupAtIndex:2];
		CHECK([group.TA intValue] == 0x20 && [group.protocol isEqual:T1] && [[specific interfaceGroupAtIndex:3].protocol isEqual:T0]);
		CHECK([atr(DATA(0x3b, 0x80, 0x91, 0x01, 0x00, 0x30)).protocols isEqualToArray:[NSArray arrayWithObject:T1]]);
		CHECK([atr(DATA(0x3b, 0x80, 0x91, 0x81, 0x00, 0xb1)).protocols isEqualToArray:[NSArray arrayWithObject:T1]]);
		CHECK([atr(DATA(0x3b, 0x80, 0x91, 0x0f, 0x00, 0x3e)).protocols isEqualToArray:[NSArray arrayWithObject:num(TKSmartCardProtocolT15)]]);

		// T0 = 60: TB1 and TC1 without TD1: group 1 only, and T=0 is the default protocol.
		TKSmartCardATR *tbtc = atr(DATA(0x3b, 0x60, 0x25, 0x03));
		group = [tbtc interfaceGroupAtIndex:1];
		CHECK([group.TB intValue] == 0x25 && [group.TC intValue] == 3 && group.TA == nil && group.protocol == nil);
		CHECK([tbtc.protocols isEqualToArray:[NSArray arrayWithObject:T0]] && [tbtc interfaceGroupAtIndex:2] == nil);

		// An ATR longer than the 33 octets of ISO/IEC 7816-3 is read as far as it is well formed (observed): TS, T0 = F0,
		// then twelve groups of TA, TB, TC, TD.
		unsigned char longBytes[60];
		size_t n = 0;
		longBytes[n++] = 0x3b;
		longBytes[n++] = 0xf0;
		for (int g = 0; g < 12; g++) {
			longBytes[n++] = 1;
			longBytes[n++] = 2;
			longBytes[n++] = 3;
			longBytes[n++] = (g < 11 ? 0xf0 : 0x00) | 1;
		}
		longBytes[n++] = 0;
		TKSmartCardATR *longer = atr([NSData dataWithBytes:longBytes length:n]);
		CHECK(n == 51 && longer != nil && [longer interfaceGroupAtIndex:13] != nil && [longer interfaceGroupAtIndex:14] == nil);

		// Historical records (7816-4 8.2): category 80, records to the end.
		TKSmartCardATR *cat80 = atr(DATA(0x3b, 0x03, 0x80, 0x31, 0xc0));
		CHECK(cat80.historicalRecords.count == 1 && [cat80.historicalRecords[0] tag] == 3);
		CHECK([[cat80.historicalRecords[0] value] isEqualToData:DATA(0xc0)]);
		CHECK([atr(DATA(0x3b, 0x02, 0x80, 0x00)).historicalRecords count] == 1);
		// Category 00 ("historical records"): the records, then the 3-octet status indicator returned as a last record
		// with tag 8 (observed).
		TKSmartCardATR *cat00 = atr(DATA(0x3b, 0x06, 0x00, 0x31, 0xc0, 0x00, 0x90, 0x00));
		CHECK(cat00.historicalRecords.count == 2 && [cat00.historicalRecords[0] tag] == 3);
		CHECK([cat00.historicalRecords[1] tag] == 8 && [[cat00.historicalRecords[1] value] isEqualToData:DATA(0x00, 0x90, 0x00)]);
		CHECK(atr(DATA(0x3b, 0x04, 0x00, 0x00, 0x90, 0x00)).historicalRecords.count == 1);
		TKSmartCardATR *two = atr(DATA(0x3b, 0x08, 0x00, 0x11, 0xaa, 0x21, 0xbb, 0xcc, 0xdd, 0xee));
		CHECK(two.historicalRecords.count == 3 && [[two.historicalRecords[2] value] isEqualToData:DATA(0xcc, 0xdd, 0xee)]);
		// No records for category 00 without room for the status indicator, for other categories, and when the
		// historical octets are not whole Compact-TLV objects (observed).
		CHECK(atr(DATA(0x3b, 0x03, 0x00, 0x90, 0x00)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x03, 0x00, 0x31, 0xc0)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x01, 0x00)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x06, 0x00, 0x35, 0xc0, 0x00, 0x90, 0x00)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x03, 0x10, 0x31, 0xc0)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x03, 0x80, 0x35, 0xc0)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x04, 0x80, 0x31, 0xc0, 0x35)).historicalRecords.count == 0);
		CHECK(atr(DATA(0x3b, 0x03, 0x40, 0x31, 0xc0)).historicalRecords.count == 0);

		// initWithSource: pulls exactly one ATR, then stops; a block that returns -1 (documented as the error signal) gives nil.
		static const unsigned char stream[] = { 0x3b, 0x91, 0x11, 0x01, 0x42, 0xc3, 0xde, 0xad };
		__block size_t pos = 0;
		TKSmartCardATR *streamed = [[[TKSmartCardATR alloc] initWithSource:^int { return pos < sizeof stream ? stream[pos++] : -1; }] autorelease];
		CHECK(streamed != nil && [streamed.bytes isEqualToData:t1.bytes] && pos == 6);
		pos = 0;
		TKSmartCardATR *dry = [[[TKSmartCardATR alloc] initWithSource:^int { return pos < 4 ? stream[pos++] : -1; }] autorelease];
		CHECK(dry == nil);
		TKSmartCardATR *failing = [[[TKSmartCardATR alloc] initWithSource:^int { return -5; }] autorelease];
		CHECK(failing == nil);
	}
	return failures;
}
