// CFLAGS: -framework Foundation -framework Contacts
// Copyright (C) 2026 Darling developers
//
// CNPostalAddress and CNMutablePostalAddress, as described on the Contacts pages for the classes and their
// members (https://developer.apple.com/documentation/contacts/cnpostaladdress and .../cnmutablepostaladdress):
//  - an immutable postal address with the parts street, subLocality, city, subAdministrativeArea, state,
//    postalCode, country and ISOCountryCode; a street of several lines is delimited with "\n";
//  - CNMutablePostalAddress is a subclass whose parts can be set; "to remove properties when saving a mutable
//    postal address, set string properties to empty values";
//  - the classes conform to NSCopying, NSMutableCopying, NSSecureCoding, Equatable and Hashable.
// Expectations that go beyond the pages are observations of macOS 26.7.3 and 10.14.4 (see CNPostalAddress.m).
// Prints PASS, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <Contacts/Contacts.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static CNMutablePostalAddress *sampleAddress(void)
{
	CNMutablePostalAddress *address = [[[CNMutablePostalAddress alloc] init] autorelease];
	address.street = @"1 Infinite Loop\nBuilding 4";
	address.subLocality = @"Downtown";
	address.city = @"Cupertino";
	address.subAdministrativeArea = @"Santa Clara County";
	address.state = @"CA";
	address.postalCode = @"95014";
	address.country = @"United States";
	address.ISOCountryCode = @"US";
	return address;
}

static BOOL samePartsAs(CNPostalAddress *a, CNPostalAddress *b)
{
	return [a.street isEqualToString:b.street] && [a.subLocality isEqualToString:b.subLocality] &&
		[a.city isEqualToString:b.city] && [a.subAdministrativeArea isEqualToString:b.subAdministrativeArea] &&
		[a.state isEqualToString:b.state] && [a.postalCode isEqualToString:b.postalCode] &&
		[a.country isEqualToString:b.country] && [a.ISOCountryCode isEqualToString:b.ISOCountryCode];
}

static void testParts(void)
{
	// A new address has an empty string for every part.
	CNMutablePostalAddress *empty = [[[CNMutablePostalAddress alloc] init] autorelease];
	CHECK([empty.street length] == 0 && [empty.subLocality length] == 0 && [empty.city length] == 0);
	CHECK([empty.subAdministrativeArea length] == 0 && [empty.state length] == 0 && [empty.postalCode length] == 0);
	CHECK([empty.country length] == 0 && [empty.ISOCountryCode length] == 0);
	CHECK([[[[CNPostalAddress alloc] init] autorelease].city length] == 0);

	CNMutablePostalAddress *address = sampleAddress();
	CHECK([address.street isEqualToString:@"1 Infinite Loop\nBuilding 4"]);
	CHECK([address.subLocality isEqualToString:@"Downtown"] && [address.city isEqualToString:@"Cupertino"]);
	CHECK([address.subAdministrativeArea isEqualToString:@"Santa Clara County"] && [address.state isEqualToString:@"CA"]);
	CHECK([address.postalCode isEqualToString:@"95014"] && [address.country isEqualToString:@"United States"]);
	CHECK([address.ISOCountryCode isEqualToString:@"US"]);
	CHECK([address isKindOfClass:[CNPostalAddress class]]);

	// The properties are declared copy.
	NSMutableString *source = [NSMutableString stringWithString:@"Berlin"];
	address.city = source;
	[source appendString:@"-Mitte"];
	CHECK([address.city isEqualToString:@"Berlin"]);

	// Setting a part to an empty string removes it; nil is stored as an empty string (observed).
	address.state = @"";
	CHECK([address.state length] == 0);
	id none = nil;
	address.postalCode = none;
	CHECK(address.postalCode != nil && [address.postalCode length] == 0);
}

static void testCopying(void)
{
	CNMutablePostalAddress *mutableAddress = sampleAddress();

	// A copy of a mutable address is an immutable one with the same parts, and does not follow later changes.
	CNPostalAddress *immutable = [[mutableAddress copy] autorelease];
	CHECK(immutable != nil && ![immutable isKindOfClass:[CNMutablePostalAddress class]]);
	CHECK([immutable isKindOfClass:[CNPostalAddress class]]);
	CHECK(samePartsAs(immutable, mutableAddress));
	mutableAddress.city = @"Elsewhere";
	CHECK([immutable.city isEqualToString:@"Cupertino"]);

	// An immutable address is its own copy.
	CHECK([[immutable copy] autorelease] == immutable);

	// A mutable copy is independent of its source, for both kinds.
	CNMutablePostalAddress *editable = [[immutable mutableCopy] autorelease];
	CHECK([editable isKindOfClass:[CNMutablePostalAddress class]] && samePartsAs(editable, immutable));
	editable.street = @"2 Elsewhere Way";
	CHECK([immutable.street isEqualToString:@"1 Infinite Loop\nBuilding 4"]);
	CNMutablePostalAddress *fromMutable = [[mutableAddress mutableCopy] autorelease];
	CHECK([fromMutable isKindOfClass:[CNMutablePostalAddress class]] && samePartsAs(fromMutable, mutableAddress));
	fromMutable.country = @"Nowhere";
	CHECK([mutableAddress.country isEqualToString:@"United States"]);
}

static void testEquality(void)
{
	CNPostalAddress *a = [[sampleAddress() copy] autorelease];
	CNPostalAddress *b = [[sampleAddress() copy] autorelease];
	CHECK([a isEqual:b] && a.hash == b.hash);
	CHECK([a isEqual:a]);
	CHECK(![a isEqual:nil] && ![a isEqual:@"1 Infinite Loop"]);
	// A mutable address with the same parts equals an immutable one (macOS 26.7.3; 10.14.4 finds the mutable one unequal to the
	// immutable one but not the other way round, which is not followed).
	CNMutablePostalAddress *m = sampleAddress();
	CHECK([m isEqual:a] && [a isEqual:m] && m.hash == a.hash);

	// Each part takes part in the comparison.
	NSArray<NSString *> *keys = @[@"street", @"subLocality", @"city", @"subAdministrativeArea", @"state", @"postalCode", @"country", @"ISOCountryCode"];
	for (NSString *key in keys)
	{
		CNMutablePostalAddress *changed = sampleAddress();
		[changed setValue:@"changed" forKey:key];
		CHECK(![changed isEqual:a]);
	}
}

static void testSecureCoding(void)
{
	CHECK([CNPostalAddress supportsSecureCoding] && [CNMutablePostalAddress supportsSecureCoding]);

	CNPostalAddress *immutable = [[sampleAddress() copy] autorelease];
	NSError *error = nil;
	NSData *data = [NSKeyedArchiver archivedDataWithRootObject:immutable requiringSecureCoding:YES error:&error];
	CHECK(data != nil);
	CNPostalAddress *back = [NSKeyedUnarchiver unarchivedObjectOfClass:[CNPostalAddress class] fromData:data error:&error];
	CHECK(back != nil && ![back isKindOfClass:[CNMutablePostalAddress class]] && samePartsAs(back, immutable));
	CHECK([back isEqual:immutable]);

	CNMutablePostalAddress *mutableAddress = sampleAddress();
	data = [NSKeyedArchiver archivedDataWithRootObject:mutableAddress requiringSecureCoding:YES error:&error];
	CHECK(data != nil);
	CNMutablePostalAddress *backMutable = [NSKeyedUnarchiver unarchivedObjectOfClass:[CNMutablePostalAddress class] fromData:data error:&error];
	CHECK(backMutable != nil && samePartsAs(backMutable, mutableAddress));
	backMutable.city = @"Changed";
	CHECK([backMutable.city isEqualToString:@"Changed"]);
}

static void testLocalizedKey(void)
{
	// The English names macOS 26.7.3 gives (the locale is not consulted), and nil for another key.
	CHECK([[CNPostalAddress localizedStringForKey:@"street"] isEqualToString:@"Street"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"subLocality"] isEqualToString:@"District"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"city"] isEqualToString:@"City"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"subAdministrativeArea"] isEqualToString:@"County"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"state"] isEqualToString:@"State"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"postalCode"] isEqualToString:@"ZIP"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"country"] isEqualToString:@"Country"]);
	CHECK([[CNPostalAddress localizedStringForKey:@"ISOCountryCode"] isEqualToString:@"Country code"]);
	CHECK([CNPostalAddress localizedStringForKey:@"bogus"] == nil);
}

int main(void)
{
	@autoreleasepool
	{
		testParts();
		testCopying();
		testEquality();
		testSecureCoding();
		testLocalizedKey();
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures;
}
