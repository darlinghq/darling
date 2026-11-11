/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Developers

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

// UTType is a view over the LaunchServices type database: every question about a type is asked of the
// UTType functions in LaunchServices, so that this class and UTTypeConformsTo() and friends agree.
//
// This file deliberately does not import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>. Apps see the
// UTType constants as `UTType *const`, but they are defined writable here so that they can be pointed at
// type objects when the framework loads, before any app code runs. The exported symbols stay plain data
// symbols, which is all an app's `extern UTType *const` import needs.

#import <UniformTypeIdentifiers/UTType.h>
#include <CoreServices/CoreServices.h>

// Not in the public LaunchServices headers; CoreServices exports it (also on macOS).
extern CFArrayRef UTTypeCopyParentIdentifiers(CFStringRef uti);

#define UT_CONSTANT(constant, identifier) UTType *constant = nil;
#include "UTTypeConstants.h"
#undef UT_CONSTANT

enum {
	UTTypeKindDeclared,
	UTTypeKindDynamic,
	UTTypeKindUndeclared,
};

static NSString *const UTTypeCodingIdentifierKey = @"UTTypeIdentifier";
static NSString *const UTDynamicPrefix = @"dyn.";
// Bounds the walk over the supertypes of a type, in case declarations make the graph cyclic.
static const NSUInteger UTMaxSupertypes = 1024;

static BOOL UTIdentifierIsValid(NSString *identifier)
{
	if (![identifier isKindOfClass: [NSString class]])
		return NO;
	NSUInteger length = [identifier length];
	if (length == 0 || [identifier characterAtIndex: 0] == '.' || [identifier characterAtIndex: length - 1] == '.')
		return NO;
	for (NSUInteger i = 0; i < length; i++) {
		unichar c = [identifier characterAtIndex: i];
		BOOL ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c >= 0x80;
		if (!ok)
			return NO;
	}
	return YES;
}

static NSArray *UTCopyArray(CFArrayRef array)
{
	return array ? [(NSArray *) array autorelease] : @[];
}

@interface UTType ()
- (instancetype)_initWithIdentifier:(NSString *)identifier kind:(int)kind parent:(nullable UTType *)parent;
@end

@implementation UTType

- (instancetype)_initWithIdentifier:(NSString *)identifier kind:(int)kind parent:(UTType *)parent
{
	self = [super init];
	if (self) {
		_identifier = [identifier copy];
		_kind = kind;
		_parent = [parent retain];
	}
	return self;
}

- (void)dealloc
{
	[_identifier release];
	[_parent release];
	[super dealloc];
}

__attribute__((constructor))
static void UTTypeFrameworkInit(void)
{
	@autoreleasepool {
#define UT_CONSTANT(constant, identifier) \
		constant = [[UTType alloc] _initWithIdentifier: @identifier kind: UTTypeKindDeclared parent: nil];
#include "UTTypeConstants.h"
#undef UT_CONSTANT
	}
}

// Returns the type for identifier: a declared type, a dynamic one, or (when allowUndeclared) an undeclared one.
static UTType *UTTypeLookup(NSString *identifier, BOOL allowUndeclared, UTType *parent)
{
	if (!UTIdentifierIsValid(identifier))
		return nil;
	int kind;
	if ([identifier length] > [UTDynamicPrefix length]
	    && [identifier compare: UTDynamicPrefix options: NSCaseInsensitiveSearch range: NSMakeRange(0, [UTDynamicPrefix length])] == NSOrderedSame)
		kind = UTTypeKindDynamic;
	else if (UTTypeIsDeclared((CFStringRef) identifier))
		kind = UTTypeKindDeclared;
	else if (allowUndeclared)
		kind = UTTypeKindUndeclared;
	else
		return nil;
	return [[[UTType alloc] _initWithIdentifier: identifier kind: kind parent: kind == UTTypeKindUndeclared ? parent : nil] autorelease];
}

+ (UTType *)typeWithIdentifier:(NSString *)identifier
{
	return UTTypeLookup(identifier, NO, nil);
}

+ (NSArray<UTType *> *)typesWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(UTType *)supertype
{
	if (![tag isKindOfClass: [NSString class]] || [tag length] == 0 || ![tagClass isKindOfClass: [NSString class]])
		return @[];
	if (supertype && ![supertype isKindOfClass: [UTType class]])
		return @[];

	CFArrayRef identifiers = UTTypeCreateAllIdentifiersForTag((CFStringRef) tagClass, (CFStringRef) tag,
	                                                          supertype ? (CFStringRef) supertype->_identifier : NULL);
	NSMutableArray *result = [NSMutableArray array];
	for (NSString *identifier in UTCopyArray(identifiers)) {
		UTType *type = UTTypeLookup(identifier, YES, nil);
		if (type)
			[result addObject: type];
	}
	return result;
}

+ (UTType *)typeWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(UTType *)supertype
{
	NSArray *types = [self typesWithTag: tag tagClass: tagClass conformingToType: supertype];
	return [types count] ? [types objectAtIndex: 0] : nil;
}

+ (UTType *)typeWithFilenameExtension:(NSString *)filenameExtension
{
	return [self typeWithTag: filenameExtension tagClass: UTTagClassFilenameExtension conformingToType: nil];
}

+ (UTType *)typeWithFilenameExtension:(NSString *)filenameExtension conformingToType:(UTType *)supertype
{
	return [self typeWithTag: filenameExtension tagClass: UTTagClassFilenameExtension conformingToType: supertype];
}

+ (UTType *)typeWithMIMEType:(NSString *)mimeType
{
	return [self typeWithTag: mimeType tagClass: UTTagClassMIMEType conformingToType: nil];
}

+ (UTType *)typeWithMIMEType:(NSString *)mimeType conformingToType:(UTType *)supertype
{
	return [self typeWithTag: mimeType tagClass: UTTagClassMIMEType conformingToType: supertype];
}

// Without a declaration the type is undeclared, but it keeps the conformance the caller asked for.
+ (UTType *)exportedTypeWithIdentifier:(NSString *)identifier
{
	return UTTypeLookup(identifier, YES, UTTypeData);
}

+ (UTType *)exportedTypeWithIdentifier:(NSString *)identifier conformingToType:(UTType *)parentType
{
	return UTTypeLookup(identifier, YES, [parentType isKindOfClass: [UTType class]] ? parentType : nil);
}

+ (UTType *)importedTypeWithIdentifier:(NSString *)identifier
{
	return UTTypeLookup(identifier, YES, UTTypeData);
}

+ (UTType *)importedTypeWithIdentifier:(NSString *)identifier conformingToType:(UTType *)parentType
{
	return UTTypeLookup(identifier, YES, [parentType isKindOfClass: [UTType class]] ? parentType : nil);
}

- (NSString *)identifier
{
	return _identifier;
}

- (NSDictionary<NSString *, NSArray<NSString *> *> *)tags
{
	NSDictionary *declaration = (NSDictionary *) UTTypeCopyDeclaration((CFStringRef) _identifier);
	NSDictionary *tags = [[[declaration objectForKey: (NSString *) kUTTypeTagSpecificationKey] retain] autorelease];
	[declaration release];
	if (tags)
		return tags;

	// Dynamic types have no declaration but carry the tag they were made from.
	NSMutableDictionary *result = [NSMutableDictionary dictionary];
	for (NSString *tagClass in @[UTTagClassFilenameExtension, UTTagClassMIMEType]) {
		NSArray *values = UTCopyArray(UTTypeCopyAllTagsWithClass((CFStringRef) _identifier, (CFStringRef) tagClass));
		if ([values count])
			[result setObject: values forKey: tagClass];
	}
	return result;
}

- (NSString *)preferredTagWithClass:(NSString *)tagClass
{
	CFStringRef tag = UTTypeCopyPreferredTagWithClass((CFStringRef) _identifier, (CFStringRef) tagClass);
	return tag ? [(NSString *) tag autorelease] : nil;
}

- (NSString *)preferredFilenameExtension
{
	return [self preferredTagWithClass: UTTagClassFilenameExtension];
}

- (NSString *)preferredMIMEType
{
	return [self preferredTagWithClass: UTTagClassMIMEType];
}

- (NSString *)localizedDescription
{
	if (_kind == UTTypeKindDynamic)
		return nil;
	CFStringRef description = UTTypeCopyDescription((CFStringRef) _identifier);
	if (description)
		return [(NSString *) description autorelease];
	for (NSString *parentIdentifier in UTCopyArray(UTTypeCopyParentIdentifiers((CFStringRef) _identifier))) {
		UTType *parent = UTTypeLookup(parentIdentifier, NO, nil);
		NSString *inherited = [parent localizedDescription];
		if (inherited)
			return inherited;
	}
	return [_parent localizedDescription];
}

- (NSNumber *)version
{
	return nil;
}

- (NSURL *)referenceURL
{
	return nil;
}

- (BOOL)isDynamic
{
	return _kind == UTTypeKindDynamic;
}

- (BOOL)isDeclared
{
	return _kind == UTTypeKindDeclared;
}

- (BOOL)isPublicType
{
	return [_identifier length] > 7 && [_identifier compare: @"public." options: NSCaseInsensitiveSearch range: NSMakeRange(0, 7)] == NSOrderedSame;
}

- (BOOL)conformsToType:(UTType *)type
{
	if (![type isKindOfClass: [UTType class]])
		return NO;
	if (UTTypeConformsTo((CFStringRef) _identifier, (CFStringRef) type->_identifier))
		return YES;
	return _parent && [_parent conformsToType: type];
}

- (BOOL)isSubtypeOfType:(UTType *)type
{
	return [type isKindOfClass: [UTType class]] && ![self isEqual: type] && [self conformsToType: type];
}

- (BOOL)isSupertypeOfType:(UTType *)type
{
	return [type isKindOfClass: [UTType class]] && [type isSubtypeOfType: self];
}

- (NSSet<UTType *> *)supertypes
{
	NSMutableSet *supertypes = [NSMutableSet set];
	NSMutableArray *pending = [NSMutableArray arrayWithObject: self];
	NSMutableSet *seen = [NSMutableSet setWithObject: [_identifier lowercaseString]];

	while ([pending count] && [seen count] < UTMaxSupertypes) {
		UTType *current = [pending lastObject];
		[pending removeLastObject];

		NSMutableArray *parents = [NSMutableArray array];
		for (NSString *parentIdentifier in UTCopyArray(UTTypeCopyParentIdentifiers((CFStringRef) current->_identifier))) {
			UTType *parent = UTTypeLookup(parentIdentifier, YES, nil);
			if (parent)
				[parents addObject: parent];
		}
		if (current->_parent)
			[parents addObject: current->_parent];

		for (UTType *parent in parents) {
			if ([seen member: [parent->_identifier lowercaseString]])
				continue;
			[seen addObject: [parent->_identifier lowercaseString]];
			[supertypes addObject: parent];
			[pending addObject: parent];
		}
	}
	return supertypes;
}

- (BOOL)isEqual:(id)object
{
	if (object == self)
		return YES;
	return [object isKindOfClass: [UTType class]] && UTTypeEqual((CFStringRef) _identifier, (CFStringRef) ((UTType *) object)->_identifier);
}

- (NSUInteger)hash
{
	return [[_identifier lowercaseString] hash];
}

- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	if ([coder allowsKeyedCoding])
		[coder encodeObject: _identifier forKey: UTTypeCodingIdentifierKey];
	else
		[coder encodeObject: _identifier];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	NSString *identifier = [coder allowsKeyedCoding]
		? [coder decodeObjectOfClass: [NSString class] forKey: UTTypeCodingIdentifierKey]
		: [coder decodeObject];
	UTType *type = [UTTypeLookup(identifier, YES, nil) retain];
	[self release];
	return type;
}

- (NSString *)description
{
	return _identifier;
}

- (NSString *)debugDescription
{
	static const char *const kinds[] = { "declared", "dynamic", "undeclared" };
	return [NSString stringWithFormat: @"<%@ %p> %@ (%s)", [self class], self, _identifier, kinds[_kind]];
}

@end
