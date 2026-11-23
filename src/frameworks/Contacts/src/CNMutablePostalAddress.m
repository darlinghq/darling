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

#import <Contacts/CNMutablePostalAddress.h>
#import <darling-stub-forwarding.h>
#import "CNPostalAddressInternal.h"

// The implementation is 64-bit only: the instance variables are declared in a private class extension
// (CNPostalAddressInternal.h), which the legacy runtime of the 32-bit slice cannot do (same as
// ASWebAuthenticationSessionRequest). The 32-bit slice keeps the forwarding stub, and the public headers
// declare no instance variables.
#if __LP64__

@implementation CNMutablePostalAddress

// The getters are those of CNPostalAddress; the parts are its instance variables.
@dynamic street;
@dynamic subLocality;
@dynamic city;
@dynamic subAdministrativeArea;
@dynamic state;
@dynamic postalCode;
@dynamic country;
@dynamic ISOCountryCode;

- (void)setStreet:(NSString *)street
{
	CNPostalAddressSetPart(&_street, street);
}

- (void)setSubLocality:(NSString *)subLocality
{
	CNPostalAddressSetPart(&_subLocality, subLocality);
}

- (void)setCity:(NSString *)city
{
	CNPostalAddressSetPart(&_city, city);
}

- (void)setSubAdministrativeArea:(NSString *)subAdministrativeArea
{
	CNPostalAddressSetPart(&_subAdministrativeArea, subAdministrativeArea);
}

- (void)setState:(NSString *)state
{
	CNPostalAddressSetPart(&_state, state);
}

- (void)setPostalCode:(NSString *)postalCode
{
	CNPostalAddressSetPart(&_postalCode, postalCode);
}

- (void)setCountry:(NSString *)country
{
	CNPostalAddressSetPart(&_country, country);
}

- (void)setISOCountryCode:(NSString *)ISOCountryCode
{
	CNPostalAddressSetPart(&_ISOCountryCode, ISOCountryCode);
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

@implementation CNMutablePostalAddress

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
