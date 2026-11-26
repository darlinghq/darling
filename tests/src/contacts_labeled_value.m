// CFLAGS: -framework Foundation -framework Contacts
// Copyright (C) 2026 Darling developers
//
// CNLabeledValue, as described on the Contacts pages for the class and its members
// (https://developer.apple.com/documentation/contacts/cnlabeledvalue):
//  - an immutable object holding a value and an optional label; labeled values are thread-safe;
//  - initWithLabel:value: makes a new labeled value with a new identifier; the identifier can be persisted;
//  - labeledValueBySettingLabel:, ...Value: and ...Label:value: return a labeled value that keeps the identifier
//    (and the part that is not set);
//  - localizedStringForLabel: returns a custom label as it is;
//  - the class conforms to NSCopying, NSSecureCoding, Equatable and Hashable.
// Expectations that go beyond the pages are observations of macOS 26.7.3 and 10.14.4 (see CNLabeledValue.m).
// Prints PASS, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <Contacts/Contacts.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static id roundTrip(id object, NSArray<Class> *classes)
{
	NSError *error = nil;
	NSData *data = [NSKeyedArchiver archivedDataWithRootObject:object requiringSecureCoding:YES error:&error];
	if (data == nil)
		return nil;
	return [NSKeyedUnarchiver unarchivedObjectOfClasses:[NSSet setWithArray:classes] fromData:data error:&error];
}

static void testCreate(void)
{
	NSMutableString *source = [NSMutableString stringWithString:@"a@example.com"];
	CNLabeledValue<NSString *> *lv = [CNLabeledValue labeledValueWithLabel:@"work" value:source];
	CHECK(lv != nil);
	CHECK([lv.label isEqualToString:@"work"]);
	CHECK([lv.value isEqualToString:@"a@example.com"]);
	// The value property is declared copy: a later change of the source does not reach the labeled value.
	[source appendString:@".uk"];
	CHECK([lv.value isEqualToString:@"a@example.com"]);

	CNLabeledValue<NSString *> *noLabel = [[[CNLabeledValue alloc] initWithLabel:nil value:@"b"] autorelease];
	CHECK(noLabel != nil && noLabel.label == nil && [noLabel.value isEqualToString:@"b"]);

	// The identifier is a UUID string.
	CHECK(lv.identifier != nil && [[[NSUUID alloc] initWithUUIDString:lv.identifier] autorelease] != nil);
	// Every new labeled value has its own identifier.
	CNLabeledValue<NSString *> *again = [CNLabeledValue labeledValueWithLabel:@"work" value:@"a@example.com"];
	CHECK(![lv.identifier isEqualToString:again.identifier]);

	// A nil value raises CNPropertyInvalidTypeException (macOS 10.14.4; 26.7.3 accepts it, which is not followed), and so does
	// setting one; -init and +new raise NSInternalInconsistencyException (both macOS versions).
	id none = nil;
	@try
	{
		[CNLabeledValue labeledValueWithLabel:@"x" value:none];
		CHECK(NO);
	}
	@catch (NSException *exception)
	{
		CHECK([exception.name isEqualToString:@"CNPropertyInvalidTypeException"]);
		CHECK([exception.reason isEqualToString:@"Labeled values require a nonnull value, but a null value was presented."]);
	}
	@try
	{
		[lv labeledValueBySettingValue:none];
		CHECK(NO);
	}
	@catch (NSException *exception)
	{
		CHECK([exception.name isEqualToString:@"CNPropertyInvalidTypeException"]);
	}
	@try
	{
		[[CNLabeledValue alloc] performSelector:@selector(init)];
		CHECK(NO);
	}
	@catch (NSException *exception)
	{
		CHECK([exception.name isEqualToString:NSInternalInconsistencyException]);
	}
}

static void testSetting(void)
{
	CNLabeledValue<NSString *> *lv = [CNLabeledValue labeledValueWithLabel:@"home" value:@"one"];

	CNLabeledValue<NSString *> *newLabel = [lv labeledValueBySettingLabel:@"work"];
	CHECK(newLabel != lv);
	CHECK([newLabel.label isEqualToString:@"work"] && [newLabel.value isEqualToString:@"one"]);
	CHECK([newLabel.identifier isEqualToString:lv.identifier]);

	CNLabeledValue<NSString *> *newValue = [lv labeledValueBySettingValue:@"two"];
	CHECK([newValue.label isEqualToString:@"home"] && [newValue.value isEqualToString:@"two"]);
	CHECK([newValue.identifier isEqualToString:lv.identifier]);

	CNLabeledValue<NSString *> *both = [lv labeledValueBySettingLabel:nil value:@"three"];
	CHECK(both.label == nil && [both.value isEqualToString:@"three"]);
	CHECK([both.identifier isEqualToString:lv.identifier]);

	// The receiver does not change.
	CHECK([lv.label isEqualToString:@"home"] && [lv.value isEqualToString:@"one"]);
	// A label can be cleared.
	CHECK([lv labeledValueBySettingLabel:nil].label == nil);
}

static void testLocalizedLabel(void)
{
	CHECK([[CNLabeledValue localizedStringForLabel:@"my own label"] isEqualToString:@"my own label"]);
}

static void testEquality(void)
{
	CNLabeledValue<NSString *> *lv = [CNLabeledValue labeledValueWithLabel:@"home" value:@"one"];
	CNLabeledValue<NSString *> *copy = [[lv copy] autorelease];
	CHECK(copy != nil && [copy isEqual:lv] && copy.hash == lv.hash);
	CHECK([lv isEqual:lv]);
	CHECK(![lv isEqual:nil] && ![lv isEqual:@"one"]);

	// Setting a part to what it already is gives an equal object (same identifier).
	CNLabeledValue<NSString *> *same = [lv labeledValueBySettingLabel:@"home"];
	CHECK([same isEqual:lv] && same.hash == lv.hash);
	CHECK(![[lv labeledValueBySettingLabel:@"work"] isEqual:lv]);
	CHECK(![[lv labeledValueBySettingValue:@"two"] isEqual:lv]);
	CHECK(![[lv labeledValueBySettingLabel:nil] isEqual:lv]);

	// The identifier takes part in equality (macOS 26.7.3 and 10.14.4).
	CNLabeledValue<NSString *> *other = [CNLabeledValue labeledValueWithLabel:@"home" value:@"one"];
	CHECK(![other isEqual:lv]);
}

static void testSecureCoding(void)
{
	CHECK([CNLabeledValue supportsSecureCoding]);

	// The keys of the archive are those macOS writes (observed on 26.7.3 and 10.14.4).
	CNLabeledValue<NSString *> *coded = [CNLabeledValue labeledValueWithLabel:@"work" value:@"a@example.com"];
	NSData *codedData = [NSKeyedArchiver archivedDataWithRootObject:coded requiringSecureCoding:YES error:NULL];
	Class plistClass = NSClassFromString(@"NSPropertyListSerialization");
	NSDictionary *plist = [plistClass propertyListWithData:codedData options:0 format:NULL error:NULL];
	BOOL foundKeys = NO;
	for (id object in plist[@"$objects"])
		if ([object isKindOfClass:[NSDictionary class]] && object[@"_identifier"] != nil && object[@"_label"] != nil && object[@"_value"] != nil)
			foundKeys = YES;
	CHECK(foundKeys);

	CNLabeledValue<NSString *> *lv = [CNLabeledValue labeledValueWithLabel:@"work" value:@"a@example.com"];
	CNLabeledValue<NSString *> *back = roundTrip(lv, @[[CNLabeledValue class], [NSString class]]);
	CHECK(back != nil && [back isKindOfClass:[CNLabeledValue class]]);
	CHECK([back.identifier isEqualToString:lv.identifier]);
	CHECK([back.label isEqualToString:@"work"] && [back.value isEqualToString:@"a@example.com"]);
	CHECK([back isEqual:lv]);

	CNLabeledValue<NSString *> *noLabel = [CNLabeledValue labeledValueWithLabel:nil value:@"x"];
	CNLabeledValue<NSString *> *backNoLabel = roundTrip(noLabel, @[[CNLabeledValue class], [NSString class]]);
	CHECK(backNoLabel != nil && backNoLabel.label == nil && [backNoLabel isEqual:noLabel]);

	// A value of another documented type (CNContact.dates holds date components).
	NSDateComponents *anniversary = [[[NSDateComponents alloc] init] autorelease];
	anniversary.year = 2001;
	anniversary.month = 2;
	anniversary.day = 3;
	CNLabeledValue<NSDateComponents *> *date = [CNLabeledValue labeledValueWithLabel:@"anniversary" value:anniversary];
	CNLabeledValue<NSDateComponents *> *backDate = roundTrip(date, @[[CNLabeledValue class], [NSDateComponents class]]);
	CHECK(backDate != nil && backDate.value.year == 2001 && backDate.value.month == 2 && backDate.value.day == 3);
}

int main(void)
{
	@autoreleasepool
	{
		testCreate();
		testSetting();
		testLocalizedLabel();
		testEquality();
		testSecureCoding();
	}
	if (g_failures == 0)
		printf("PASS\n");
	return g_failures;
}
