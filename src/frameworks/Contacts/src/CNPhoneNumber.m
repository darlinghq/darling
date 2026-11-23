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

#import <Contacts/CNPhoneNumber.h>
#import <darling-stub-forwarding.h>

// The implementation is 64-bit only: the instance variable is declared in the @implementation, which the
// legacy runtime of the 32-bit slice cannot do (same as ASWebAuthenticationSessionRequest). The 32-bit slice
// keeps the forwarding stub, and the public header declares no instance variables.
#if __LP64__

// The coder key is that of an archive made by macOS 26.7.3 and 10.14.4 (which also writes a nil countryCode).
static NSString * const kStringValueKey = @"stringValue";

@implementation CNPhoneNumber {
	NSString *_stringValue;
}

@synthesize stringValue = _stringValue;

+ (instancetype)phoneNumberWithStringValue:(NSString *)stringValue
{
	return [[[self alloc] initWithStringValue:stringValue] autorelease];
}

- (instancetype)initWithStringValue:(NSString *)string
{
	if (string == nil)
	{
		[self release];
		return nil;
	}
	if ((self = [super init]))
	{
		// The string is kept as it was given: the pages do not mention any normalisation.
		_stringValue = [string copy];
	}
	return self;
}

// The pages mark -init and +new deprecated and do not say what they give. macOS 26.7.3 and 10.14.4 give nil for both (observed),
// as initWithStringValue: does for no string.
- (instancetype)init
{
	[self release];
	return nil;
}

+ (instancetype)new
{
	return [[self alloc] init];
}

- (void)dealloc
{
	[_stringValue release];
	[super dealloc];
}

// Immutable, so a copy is the object itself.
- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

- (BOOL)isEqual:(id)object
{
	if (object == self)
		return YES;
	if (![object isKindOfClass:[CNPhoneNumber class]])
		return NO;
	return [_stringValue isEqualToString:((CNPhoneNumber *) object)->_stringValue];
}

- (NSUInteger)hash
{
	return [_stringValue hash];
}

// The text of macOS 10.14.4 and 26.7.3; the initial country code is not kept, so it always reads (nil).
- (NSString *)description
{
	return [NSString stringWithFormat:@"<%@: %p: stringValue=%@, initialCountryCode=(nil)>", [self class], self, _stringValue];
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeObject:_stringValue forKey:kStringValueKey];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	return [self initWithStringValue:[coder decodeObjectOfClass:[NSString class] forKey:kStringValueKey]];
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

@implementation CNPhoneNumber

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
