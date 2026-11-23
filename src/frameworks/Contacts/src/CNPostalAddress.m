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

// The coder keys are Darling's own: the pages do not give the keys of an archive made by macOS. They are
// the names of the properties.

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
		// GUESS: a new address has empty strings: every part is declared non-nil and the pages say an empty
		// string means a part is not set.
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
	// GUESS: Darling has no catalogue of the localized property names yet, so the key comes back as it is.
	return key;
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

// GUESS: two addresses are equal when all eight parts are equal, whether or not either is mutable (the pages
// list Equatable and Hashable and say no more).
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
	[coder encodeObject:_street forKey:@"street"];
	[coder encodeObject:_subLocality forKey:@"subLocality"];
	[coder encodeObject:_city forKey:@"city"];
	[coder encodeObject:_subAdministrativeArea forKey:@"subAdministrativeArea"];
	[coder encodeObject:_state forKey:@"state"];
	[coder encodeObject:_postalCode forKey:@"postalCode"];
	[coder encodeObject:_country forKey:@"country"];
	[coder encodeObject:_ISOCountryCode forKey:@"ISOCountryCode"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	if ((self = [super init]))
	{
		// A part missing from the archive is empty.
		CNPostalAddressSetPart(&_street, [coder decodeObjectOfClass:[NSString class] forKey:@"street"]);
		CNPostalAddressSetPart(&_subLocality, [coder decodeObjectOfClass:[NSString class] forKey:@"subLocality"]);
		CNPostalAddressSetPart(&_city, [coder decodeObjectOfClass:[NSString class] forKey:@"city"]);
		CNPostalAddressSetPart(&_subAdministrativeArea, [coder decodeObjectOfClass:[NSString class] forKey:@"subAdministrativeArea"]);
		CNPostalAddressSetPart(&_state, [coder decodeObjectOfClass:[NSString class] forKey:@"state"]);
		CNPostalAddressSetPart(&_postalCode, [coder decodeObjectOfClass:[NSString class] forKey:@"postalCode"]);
		CNPostalAddressSetPart(&_country, [coder decodeObjectOfClass:[NSString class] forKey:@"country"]);
		CNPostalAddressSetPart(&_ISOCountryCode, [coder decodeObjectOfClass:[NSString class] forKey:@"ISOCountryCode"]);
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
