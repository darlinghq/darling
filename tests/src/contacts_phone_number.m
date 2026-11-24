// CFLAGS: -framework Foundation -framework Contacts
// Copyright (C) 2026 Darling developers
//
// CNPhoneNumber, as described on the Contacts pages for the class and its members
// (https://developer.apple.com/documentation/contacts/cnphonenumber):
//  - an immutable object holding the string value of a phone number; objects are thread-safe;
//  - initWithStringValue: returns a new phone number from a string, and nil when the string is nil;
//  - stringValue gives the string back;
//  - the class conforms to NSCopying, NSSecureCoding, Equatable and Hashable;
//  - init and new are deprecated.
// Expectations that go beyond the pages are observations of macOS 26.7.3 and 10.14.4 (see CNPhoneNumber.m).
// Prints PASS, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <Contacts/Contacts.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void testCreate(void)
{
	CNPhoneNumber *number = [[[CNPhoneNumber alloc] initWithStringValue:@"(555) 010-4477"] autorelease];
	CHECK(number != nil);
	// The string comes back as it was given: the pages mention no normalisation.
	CHECK([number.stringValue isEqualToString:@"(555) 010-4477"]);

	CNPhoneNumber *viaClass = [CNPhoneNumber phoneNumberWithStringValue:@"+1 555 010 4477"];
	CHECK(viaClass != nil && [viaClass.stringValue isEqualToString:@"+1 555 010 4477"]);

	// The string is copied.
	NSMutableString *source = [NSMutableString stringWithString:@"12345"];
	CNPhoneNumber *copied = [CNPhoneNumber phoneNumberWithStringValue:source];
	[source appendString:@"6"];
	CHECK([copied.stringValue isEqualToString:@"12345"]);

	// The pages: "returns nil when the value of string is nil".
	id none = nil;
	CHECK([CNPhoneNumber phoneNumberWithStringValue:none] == nil);
	CHECK([[CNPhoneNumber alloc] initWithStringValue:none] == nil);
}

static void testDeprecated(void)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	// The deprecated init and new give nil (observed).
	CHECK([[CNPhoneNumber alloc] init] == nil);
	CHECK([CNPhoneNumber new] == nil);
#pragma clang diagnostic pop
}

static void testEquality(void)
{
	CNPhoneNumber *a = [CNPhoneNumber phoneNumberWithStringValue:@"555-0100"];
	CNPhoneNumber *b = [CNPhoneNumber phoneNumberWithStringValue:@"555-0100"];
	CNPhoneNumber *c = [CNPhoneNumber phoneNumberWithStringValue:@"555-0101"];
	CHECK([a isEqual:b] && a.hash == b.hash);
	CHECK(![a isEqual:c]);
	CHECK([a isEqual:a]);
	CHECK(![a isEqual:nil] && ![a isEqual:@"555-0100"]);
	// The string is compared as it is: a different notation is a different number.
	CHECK(![a isEqual:[CNPhoneNumber phoneNumberWithStringValue:@"5550100"]]);

	CNPhoneNumber *copy = [[a copy] autorelease];
	CHECK(copy != nil && [copy isEqual:a] && [copy.stringValue isEqualToString:@"555-0100"]);
}

static void testSecureCoding(void)
{
	CHECK([CNPhoneNumber supportsSecureCoding]);
	CNPhoneNumber *number = [CNPhoneNumber phoneNumberWithStringValue:@"+44 20 7946 0000"];
	NSError *error = nil;
	NSData *data = [NSKeyedArchiver archivedDataWithRootObject:number requiringSecureCoding:YES error:&error];
	CHECK(data != nil);
	CNPhoneNumber *back = [NSKeyedUnarchiver unarchivedObjectOfClass:[CNPhoneNumber class] fromData:data error:&error];
	CHECK(back != nil && [back isEqual:number] && [back.stringValue isEqualToString:@"+44 20 7946 0000"]);
}

int main(void)
{
	@autoreleasepool
	{
		testCreate();
		testDeprecated();
		testEquality();
		testSecureCoding();
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures;
}
