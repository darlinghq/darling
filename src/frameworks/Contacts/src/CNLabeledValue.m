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

// The coder keys are those of an archive made by macOS 26.7.3 and 10.14.4 (read back through NSKeyedArchiver; macOS also writes
// keys for properties this class does not have yet, which are ignored when decoding).
static NSString * const kIdentifierKey = @"_identifier";
static NSString * const kLabelKey = @"_label";
static NSString * const kValueKey = @"_value";

// The exception macOS 10.14.4 raises for a labeled value without a value (observed; macOS 26.7.3 accepts a nil value, which is
// not followed here because a value is declared non-optional and 10.14.4 is the strictest).
static void requireValue(id value)
{
	if (value == nil)
		[NSException raise:@"CNPropertyInvalidTypeException" format:@"Labeled values require a nonnull value, but a null value was presented."];
}

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

// The designated initializer, with the parts as they are (a value of nil is refused; the parts are copied by the callers that
// take them from outside).
- (instancetype)initWithIdentifier:(NSString *)identifier label:(NSString *)label value:(id)value
{
	requireValue(value);
	if ((self = [super init]))
	{
		_identifier = [identifier retain];
		_label = [label retain];
		_value = [value retain];
	}
	return self;
}

// A labeled value of the receiver's class with the given parts; the parts are retained as they are.
static CNLabeledValue *makeLabeledValue(Class cls, NSString *identifier, NSString *label, id value)
{
	return [[[cls alloc] initWithIdentifier:identifier label:label value:value] autorelease];
}

+ (instancetype)labeledValueWithLabel:(NSString *)label value:(id)value
{
	return [[[self alloc] initWithLabel:label value:value] autorelease];
}

// macOS raises NSInternalInconsistencyException for -init and +new (26.7.3 and 10.14.4, observed).
- (instancetype)init
{
	[self release];
	[NSException raise:NSInternalInconsistencyException format:@"-[CNLabeledValue init]: initializer unavailable; use -[CNLabeledValue initWithLabel:value:] instead"];
	return nil;
}

- (instancetype)initWithLabel:(NSString *)label value:(id)value
{
	// The identifier is a UUID string (macOS 26.7.3 and 10.14.4, observed).
	return [self initWithIdentifier:[[NSUUID UUID] UUIDString] label:[[label copy] autorelease] value:[[value copyWithZone:nil] autorelease]];
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label
{
	return makeLabeledValue([self class], _identifier, [[label copy] autorelease], _value);
}

- (instancetype)labeledValueBySettingValue:(id)value
{
	requireValue(value);
	return makeLabeledValue([self class], _identifier, _label, [[value copyWithZone:nil] autorelease]);
}

- (instancetype)labeledValueBySettingLabel:(NSString *)label value:(id)value
{
	requireValue(value);
	return makeLabeledValue([self class], _identifier, [[label copy] autorelease], [[value copyWithZone:nil] autorelease]);
}

+ (NSString *)localizedStringForLabel:(NSString *)label
{
	// A custom label comes back as it is. The label constants of the framework (CNLabelHome and the others) are not defined
	// yet, so none can be recognised; macOS turns them into a localized word ("_$!<Home>!$_" is "home"), here every label
	// comes back as it is.
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

// Two labeled values are equal when identifier, label and value are all equal (observed on macOS 26.7.3 and 10.14.4: two
// labeled values made with the same label and value are not equal, and a label or value that differs makes them unequal).
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

// The text of macOS 10.14.4: the class, the address and the three parts (nil prints as "(nil)").
- (NSString *)description
{
	return [NSString stringWithFormat:@"<%@: %p: identifier=%@, label=%@, value=%@>", [self class], self, _identifier, _label ?: @"(nil)", _value ?: @"(nil)"];
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
		// An archive without an identifier or without a value is not one this class wrote. Darling's NSCoder has no
		// failWithError:.
		if (_identifier == nil || _value == nil)
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
