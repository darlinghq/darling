/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#import <Contacts/CNPostalAddress.h>
#import <Contacts/CNMutablePostalAddress.h>
#import <darling-stub-forwarding.h>
#import "CNPostalAddressInternal.h"

// The implementation is 64-bit only: the instance variables are declared in a private class extension
// (CNPostalAddressInternal.h), which the legacy runtime of the 32-bit slice cannot do (same as
// ASWebAuthenticationSessionRequest). The 32-bit slice keeps the forwarding stub, and the public headers
// declare no instance variables.
#if __LP64__

// The coder keys are those of an archive made by macOS 26.7.3 and 10.14.4: the names of the instance variables.

@implementation CNPostalAddress

@synthesize street = _street;
@synthesize subLocality = _subLocality;
@synthesize city = _city;
@synthesize subAdministrativeArea = _subAdministrativeArea;
@synthesize state = _state;
@synthesize postalCode = _postalCode;
@synthesize country = _country;
@synthesize ISOCountryCode = _ISOCountryCode;

static void copyParts(CNPostalAddress *destination, CNPostalAddress *source)
{
	CNPostalAddressSetPart(&destination->_street, source->_street);
	CNPostalAddressSetPart(&destination->_subLocality, source->_subLocality);
	CNPostalAddressSetPart(&destination->_city, source->_city);
	CNPostalAddressSetPart(&destination->_subAdministrativeArea, source->_subAdministrativeArea);
	CNPostalAddressSetPart(&destination->_state, source->_state);
	CNPostalAddressSetPart(&destination->_postalCode, source->_postalCode);
	CNPostalAddressSetPart(&destination->_country, source->_country);
	CNPostalAddressSetPart(&destination->_ISOCountryCode, source->_ISOCountryCode);
}

- (instancetype)init
{
	if ((self = [super init]))
	{
		// A new address has empty strings (observed on macOS 26.7.3 and 10.14.4): every part is declared non-nil and the pages
		// say an empty string means a part is not set.
		CNPostalAddressSetPart(&_street, nil);
		CNPostalAddressSetPart(&_subLocality, nil);
		CNPostalAddressSetPart(&_city, nil);
		CNPostalAddressSetPart(&_subAdministrativeArea, nil);
		CNPostalAddressSetPart(&_state, nil);
		CNPostalAddressSetPart(&_postalCode, nil);
		CNPostalAddressSetPart(&_country, nil);
		CNPostalAddressSetPart(&_ISOCountryCode, nil);
	}
	return self;
}

- (void)dealloc
{
	[_street release];
	[_subLocality release];
	[_city release];
	[_subAdministrativeArea release];
	[_state release];
	[_postalCode release];
	[_country release];
	[_ISOCountryCode release];
	[super dealloc];
}

+ (NSString *)localizedStringForKey:(NSString *)key
{
	// The English names that macOS 26.7.3 gives (the locale is not consulted here: Darling has no catalogue of translated
	// names), and nil for a key that is not one of the eight parts. Test: tests/src/contacts_postal_address.m.
	static NSDictionary *names;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		names = [@{
			@"street": @"Street", @"subLocality": @"District", @"city": @"City", @"subAdministrativeArea": @"County",
			@"state": @"State", @"postalCode": @"ZIP", @"country": @"Country", @"ISOCountryCode": @"Country code",
		} retain];
	});
	return key ? names[key] : nil;
}

// A copy of a mutable address is an immutable one, as for the other Foundation classes that come in both
// kinds; an immutable address is its own copy.
- (id)copyWithZone:(NSZone *)zone
{
	if ([self class] == [CNPostalAddress class])
		return [self retain];
	CNPostalAddress *result = [[CNPostalAddress allocWithZone:zone] init];
	copyParts(result, self);
	return result;
}

- (id)mutableCopyWithZone:(NSZone *)zone
{
	CNMutablePostalAddress *result = [[CNMutablePostalAddress allocWithZone:zone] init];
	copyParts(result, self);
	return result;
}

// Two addresses are equal when all eight parts are equal, whether or not either is mutable. macOS 26.7.3 does the same;
// 10.14.4 finds a mutable address unequal to an immutable one with the same parts, but not the other way round, which is not followed.
- (BOOL)isEqual:(id)object
{
	if (object == self)
		return YES;
	if (![object isKindOfClass:[CNPostalAddress class]])
		return NO;
	CNPostalAddress *other = object;
	return [_street isEqualToString:other->_street] &&
		[_subLocality isEqualToString:other->_subLocality] &&
		[_city isEqualToString:other->_city] &&
		[_subAdministrativeArea isEqualToString:other->_subAdministrativeArea] &&
		[_state isEqualToString:other->_state] &&
		[_postalCode isEqualToString:other->_postalCode] &&
		[_country isEqualToString:other->_country] &&
		[_ISOCountryCode isEqualToString:other->_ISOCountryCode];
}

// The text of macOS 10.14.4 and 26.7.3 (the ISO country code is printed as "countryCode").
- (NSString *)description
{
	return [NSString stringWithFormat:@"<%@: %p: street=%@, subLocality=%@, city=%@, subAdministrativeArea=%@, state=%@, postalCode=%@, country=%@, countryCode=%@>",
		[self class], self, _street, _subLocality, _city, _subAdministrativeArea, _state, _postalCode, _country, _ISOCountryCode];
}

- (NSUInteger)hash
{
	return ([_street hash] * 1) ^
		([_subLocality hash] * 3) ^
		([_city hash] * 5) ^
		([_subAdministrativeArea hash] * 7) ^
		([_state hash] * 9) ^
		([_postalCode hash] * 11) ^
		([_country hash] * 13) ^
		([_ISOCountryCode hash] * 15);
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeObject:_street forKey:@"_street"];
	[coder encodeObject:_subLocality forKey:@"_subLocality"];
	[coder encodeObject:_city forKey:@"_city"];
	[coder encodeObject:_subAdministrativeArea forKey:@"_subAdministrativeArea"];
	[coder encodeObject:_state forKey:@"_state"];
	[coder encodeObject:_postalCode forKey:@"_postalCode"];
	[coder encodeObject:_country forKey:@"_country"];
	[coder encodeObject:_ISOCountryCode forKey:@"_ISOCountryCode"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	if ((self = [super init]))
	{
		// A part missing from the archive is empty.
		CNPostalAddressSetPart(&_street, [coder decodeObjectOfClass:[NSString class] forKey:@"_street"]);
		CNPostalAddressSetPart(&_subLocality, [coder decodeObjectOfClass:[NSString class] forKey:@"_subLocality"]);
		CNPostalAddressSetPart(&_city, [coder decodeObjectOfClass:[NSString class] forKey:@"_city"]);
		CNPostalAddressSetPart(&_subAdministrativeArea, [coder decodeObjectOfClass:[NSString class] forKey:@"_subAdministrativeArea"]);
		CNPostalAddressSetPart(&_state, [coder decodeObjectOfClass:[NSString class] forKey:@"_state"]);
		CNPostalAddressSetPart(&_postalCode, [coder decodeObjectOfClass:[NSString class] forKey:@"_postalCode"]);
		CNPostalAddressSetPart(&_country, [coder decodeObjectOfClass:[NSString class] forKey:@"_country"]);
		CNPostalAddressSetPart(&_ISOCountryCode, [coder decodeObjectOfClass:[NSString class] forKey:@"_ISOCountryCode"]);
	}
	return self;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#else

// The properties and methods of the public header have no implementation in the 32-bit stub.
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wincomplete-implementation"
#pragma clang diagnostic ignored "-Wprotocol"

@implementation CNPostalAddress

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation {
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#endif
