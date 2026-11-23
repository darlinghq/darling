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

#import <Contacts/CNLabeledValue.h>
#import <Contacts/CNContactRelation.h>
#import <Contacts/CNInstantMessageAddress.h>
#import <Contacts/CNPhoneNumber.h>
#import <Contacts/CNPostalAddress.h>
#import <Contacts/CNSocialProfile.h>
#import <darling-stub-forwarding.h>

// The implementation is 64-bit only: the instance variables are declared in the @implementation, which the
// legacy runtime of the 32-bit slice cannot do (same as ASWebAuthenticationSessionRequest). The 32-bit slice
// keeps the forwarding stub, and the public header declares no instance variables.
#if __LP64__

// The coder keys are Darling's own: the pages do not give the keys of an archive made by macOS.
static NSString * const kIdentifierKey = @"identifier";
static NSString * const kLabelKey = @"label";
static NSString * const kValueKey = @"value";

static BOOL sameObject(id a, id b)
{
	return a == b || [a isEqual:b];
}

@implementation CNLabeledValue {
	NSString *_identifier;
	NSString *_label;
	id _value;
}

@synthesize identifier = _identifier;
@synthesize label = _label;
@synthesize value = _value;

// A labeled value of the receiver's class with the given parts; the parts are retained as they are.
static CNLabeledValue *makeLabeledValue(Class cls, NSString *identifier, NSString *label, id value)
{
	CNLabeledValue *result = [[cls alloc] init];
	if (result != nil)
	{
		result->_identifier = [identifier retain];
		result->_label = [label retain];
		result->_value = [value retain];
	}
	return [result autorelease];
}

+ (instancetype)labeledValueWithLabel:(NSString *)label value:(id)value
{
	return [[[self alloc] initWithLabel:label value:value] autorelease];
}

- (instancetype)initWithLabel:(NSString *)label value:(id)value
{
	if ((self = [super init]))
	{
		// GUESS: the pages call the identifier unique and persistable but give no format; a UUID string.
		_identifier = [[[NSUUID UUID] UUIDString] retain];
		_label = [label copy];
		// GUESS: a nil value is not rejected (the pages declare a non-optional value but do not say what
		// happens otherwise); the value reads nil.
		_value = [value copyWithZone:nil];
	}
	return self;
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label
{
	return makeLabeledValue([self class], _identifier, [[label copy] autorelease], _value);
}

- (instancetype)labeledValueBySettingValue:(id)value
{
	return makeLabeledValue([self class], _identifier, _label, [[value copyWithZone:nil] autorelease]);
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label value:(id)value
{
	return makeLabeledValue([self class], _identifier, [[label copy] autorelease], [[value copyWithZone:nil] autorelease]);
}

+ (NSString *)localizedStringForLabel:(NSString *)label
{
	// A custom label comes back as it is. GUESS: the label constants of the framework are not defined
	// yet, so none can be recognised and localized; every label comes back as it is.
	return label;
}

- (void)dealloc
{
	[_identifier release];
	[_label release];
	[_value release];
	[super dealloc];
}

// Immutable, so a copy is the object itself.
- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

// GUESS: two labeled values are equal when identifier, label and value are all equal (the pages list
// Equatable and Hashable and say no more). Rejected alternative: ignoring the identifier, which would make two
// separate entries of a contact with the same label and value equal.
- (BOOL)isEqual:(id)object
{
	if (object == self)
		return YES;
	if (![object isKindOfClass:[CNLabeledValue class]])
		return NO;
	CNLabeledValue *other = object;
	return [_identifier isEqual:other->_identifier] && sameObject(_label, other->_label) && sameObject(_value, other->_value);
}

- (NSUInteger)hash
{
	return [_identifier hash] ^ ([_label hash] * 31) ^ ([_value hash] * 961);
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeObject:_identifier forKey:kIdentifierKey];
	[coder encodeObject:_label forKey:kLabelKey];
	[coder encodeObject:_value forKey:kValueKey];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	if ((self = [super init]))
	{
		// The value classes are the ones CNContact's labeled value arrays hold.
		NSSet *valueClasses = [NSSet setWithObjects:[NSString class], [NSDateComponents class], [CNPhoneNumber class],
			[CNPostalAddress class], [CNContactRelation class], [CNSocialProfile class], [CNInstantMessageAddress class], nil];
		_identifier = [[coder decodeObjectOfClass:[NSString class] forKey:kIdentifierKey] copy];
		_label = [[coder decodeObjectOfClass:[NSString class] forKey:kLabelKey] copy];
		_value = [[coder decodeObjectOfClasses:valueClasses forKey:kValueKey] retain];
		// An archive without an identifier is not one this class wrote. A missing value is read as nil, as
		// initWithLabel:value: accepts one (GUESS 2 there). Darling's NSCoder has no failWithError:.
		if (_identifier == nil)
		{
			[self release];
			return nil;
		}
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

@implementation CNLabeledValue

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
