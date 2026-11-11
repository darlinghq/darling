// CFLAGS: -framework Foundation -framework AddressBook
// Copyright (C) 2026 Lubos Dolezel

#import <Foundation/Foundation.h>
#import <AddressBook/ABAddressBook.h>
#import <AddressBook/ABPerson.h>
#import <AddressBook/ABGlobals.h>
#include <stdio.h>

static int failures = 0;
#define check(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

int main(void)
{
	@autoreleasepool {
		ABAddressBook *book = [ABAddressBook sharedAddressBook];
		check(book != nil);
		check([ABAddressBook sharedAddressBook] == book);

		// The "me" record is stable and has a stable unique id.
		ABPerson *me = [book me];
		check(me != nil);
		check([book me] == me);
		NSString *uid = [me uniqueId];
		check([uid length] > 0);
		check([[me uniqueId] isEqualToString:uid]);

		// Other records have their own ids and do not inherit the login name.
		ABPerson *other = [[[ABPerson alloc] init] autorelease];
		check(![[other uniqueId] isEqualToString:uid]);
		check([other valueForProperty:kABFirstNameProperty] == nil);

		// Values set on a record are returned, and can be removed again.
		check([other setValue:@"Ada" forProperty:kABFirstNameProperty]);
		check([[other valueForProperty:kABFirstNameProperty] isEqual:@"Ada"]);
		check([me valueForProperty:kABFirstNameProperty] != nil);
		check([other removeValueForProperty:kABFirstNameProperty]);
		check([other valueForProperty:kABFirstNameProperty] == nil);
		check(![other setValue:nil forProperty:kABFirstNameProperty]);

		// setMe: replaces the "me" record.
		[book setMe:other];
		check([book me] == other);

		check([[book people] count] == 0);
		check([book recordForUniqueId:@"missing"] == nil);
		check(![book hasUnsavedChanges]);
		check([book save]);
	}
	return failures ? 1 : 0;
}
