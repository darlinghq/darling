/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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

// This file deliberately does not import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>. Apps see the
// UTType constants as `UTType *const`, but they are defined writable here so that the registry, built once
// when the framework loads (before any app code runs), can point them at the built-in type objects.
// The exported symbols stay plain data symbols, which is all an app's `extern UTType *const` import needs.

#import <UniformTypeIdentifiers/UTType.h>
#include <dispatch/dispatch.h>

#define UT_CORE_TYPE(constant, identifier, parents, extensions, mimeTypes, description) UTType *constant = nil;
#define UT_BASE_TYPE(identifier, parents, extensions, mimeTypes, description)
#include "UTTypeTable.h"
#undef UT_CORE_TYPE
#undef UT_BASE_TYPE

typedef enum {
	UTTypeKindDeclared,
	UTTypeKindDynamic,
	UTTypeKindUndeclared,
} UTTypeKind;

static NSString *const UTTypeCodingIdentifierKey = @"UTTypeIdentifier";
static NSString *const UTDynamicPrefix = @"dyn.";
static NSString *const UTDynamicSeparator = @"\x1f";
static const int UTMaxConformanceDepth = 64;

// Lowercased identifier -> UTType, and tag class -> (lowercased tag -> NSArray of UTType in table order).
// Built once, never mutated afterwards and never freed, so lookups need no locking.
static NSDictionary *UTRegistry;
static NSDictionary *UTTagIndex;
static dispatch_once_t UTRegistryOnce;

static void UTBuildRegistry(void *context);

static void UTEnsureRegistry(void)
{
	dispatch_once_f(&UTRegistryOnce, NULL, UTBuildRegistry);
}

__attribute__((constructor))
static void UTTypeFrameworkInit(void)
{
	UTEnsureRegistry();
}

static NSArray *UTSplitList(const char *list)
{
	NSMutableArray *result = [NSMutableArray array];
	for (NSString *item in [[NSString stringWithUTF8String: list] componentsSeparatedByString: @" "]) {
		if ([item length] > 0)
			[result addObject: item];
	}
	return result;
}

// Appends type to the table-ordered list of every tag's (lowercased) index entry.
static void UTIndexAddType(NSMutableDictionary *index, NSArray *tags, id type)
{
	for (NSString *tag in tags) {
		NSString *key = [tag lowercaseString];
		NSArray *existing = [index objectForKey: key];
		[index setObject: existing ? [existing arrayByAddingObject: type] : @[type] forKey: key];
	}
}

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

@interface UTType ()
- (instancetype)_initWithIdentifier:(NSString *)identifier kind:(UTTypeKind)kind parents:(NSArray *)parents
                               tags:(NSDictionary *)tags description:(NSString *)description;
- (void)_setParents:(NSArray *)parents;
@end

@implementation UTType {
	NSString *_identifier;
	NSString *_key;         // lowercased identifier: identifiers compare case-insensitively
	NSArray *_parents;      // UTType objects this type directly conforms to
	NSDictionary *_tags;
	NSString *_description;
	UTTypeKind _kind;
}

static void UTBuildRegistry(void *context)
{
	@autoreleasepool {
		struct UTTypeRow {
			UTType **constant;
			const char *identifier, *parents, *extensions, *mimeTypes, *description;
		};
		static const struct UTTypeRow rows[] = {
#define UT_CORE_TYPE(constant, identifier, parents, extensions, mimeTypes, description) \
			{ &constant, identifier, parents, extensions, mimeTypes, description },
#define UT_BASE_TYPE(identifier, parents, extensions, mimeTypes, description) \
			{ NULL, identifier, parents, extensions, mimeTypes, description },
#include "UTTypeTable.h"
#undef UT_CORE_TYPE
#undef UT_BASE_TYPE
		};
		size_t count = sizeof(rows) / sizeof(rows[0]);
		NSMutableDictionary *registry = [NSMutableDictionary dictionaryWithCapacity: count];
		NSMutableDictionary *extensionIndex = [NSMutableDictionary dictionary];
		NSMutableDictionary *mimeIndex = [NSMutableDictionary dictionary];

		for (size_t i = 0; i < count; i++) {
			NSArray *extensions = UTSplitList(rows[i].extensions), *mimeTypes = UTSplitList(rows[i].mimeTypes);
			NSMutableDictionary *tags = [NSMutableDictionary dictionary];
			if ([extensions count])
				[tags setObject: extensions forKey: UTTagClassFilenameExtension];
			if ([mimeTypes count])
				[tags setObject: mimeTypes forKey: UTTagClassMIMEType];

			NSString *identifier = [NSString stringWithUTF8String: rows[i].identifier];
			UTType *type = [[UTType alloc] _initWithIdentifier: identifier kind: UTTypeKindDeclared parents: nil tags: tags
			                                       description: [NSString stringWithUTF8String: rows[i].description]];
			[registry setObject: type forKey: type->_key];
			[type release];

			UTIndexAddType(extensionIndex, extensions, type);
			UTIndexAddType(mimeIndex, mimeTypes, type);
		}

		// Parents are resolved in a second pass so the table can list types in any order.
		for (size_t i = 0; i < count; i++) {
			UTType *type = [registry objectForKey: [[NSString stringWithUTF8String: rows[i].identifier] lowercaseString]];
			NSMutableArray *parents = [NSMutableArray array];
			for (NSString *parentIdentifier in UTSplitList(rows[i].parents)) {
				UTType *parent = [registry objectForKey: [parentIdentifier lowercaseString]];
				if (parent)
					[parents addObject: parent];
				else
					NSLog(@"UniformTypeIdentifiers: %@ conforms to undeclared type %@", type->_identifier, parentIdentifier);
			}
			[type _setParents: parents];
			if (rows[i].constant)
				*rows[i].constant = type;
		}

		UTRegistry = [registry copy];
		UTTagIndex = [@{
			UTTagClassFilenameExtension: [[extensionIndex copy] autorelease],
			UTTagClassMIMEType: [[mimeIndex copy] autorelease],
		} copy];
	}
}

static BOOL UTTypeConformsToKey(UTType *type, NSString *key, int depth)
{
	if ([type->_key isEqualToString: key])
		return YES;
	if (depth >= UTMaxConformanceDepth)
		return NO;
	for (UTType *parent in type->_parents) {
		if (UTTypeConformsToKey(parent, key, depth + 1))
			return YES;
	}
	return NO;
}

static void UTTypeCollectSupertypes(UTType *type, NSMutableSet *into, int depth)
{
	if (depth >= UTMaxConformanceDepth)
		return;
	for (UTType *parent in type->_parents) {
		[into addObject: parent];
		UTTypeCollectSupertypes(parent, into, depth + 1);
	}
}

static NSString *UTHexEncode(NSString *string)
{
	NSData *data = [string dataUsingEncoding: NSUTF8StringEncoding];
	const unsigned char *bytes = [data bytes];
	NSMutableString *hex = [NSMutableString stringWithCapacity: [data length] * 2];
	for (NSUInteger i = 0; i < [data length]; i++)
		[hex appendFormat: @"%02x", bytes[i]];
	return hex;
}

static NSString *UTHexDecode(NSString *hex)
{
	NSUInteger length = [hex length];
	if (length % 2)
		return nil;
	NSMutableData *data = [NSMutableData dataWithLength: length / 2];
	unsigned char *bytes = [data mutableBytes];
	for (NSUInteger i = 0; i < length; i++) {
		unichar c = [hex characterAtIndex: i];
		int nibble = (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
		if (nibble < 0)
			return nil;
		bytes[i / 2] = (unsigned char) ((bytes[i / 2] << 4) | nibble);
	}
	return [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease];
}

static UTType *UTTypeLookup(NSString *identifier, BOOL allowUndeclared);

// Dynamic identifiers here are "dyn." followed by the hex UTF-8 of "tagClass<US>tag<US>parentIdentifier", so a
// dynamic type round-trips through its identifier. This encoding is Darling's own, not Apple's.
static UTType *UTDynamicType(NSString *tagClass, NSString *tag, UTType *parent)
{
	NSString *payload = [@[tagClass, tag, parent->_identifier] componentsJoinedByString: UTDynamicSeparator];
	NSString *identifier = [UTDynamicPrefix stringByAppendingString: UTHexEncode(payload)];
	return [[[UTType alloc] _initWithIdentifier: identifier kind: UTTypeKindDynamic parents: @[parent]
	                                       tags: @{ tagClass: @[tag] } description: nil] autorelease];
}

static UTType *UTDynamicTypeFromIdentifier(NSString *identifier)
{
	NSString *payload = UTHexDecode([identifier substringFromIndex: [UTDynamicPrefix length]]);
	NSArray *parts = [payload componentsSeparatedByString: UTDynamicSeparator];
	if ([parts count] == 3 && [UTTagIndex objectForKey: parts[0]] && [parts[1] length] > 0
	    && ![[parts[2] lowercaseString] hasPrefix: UTDynamicPrefix]) {
		UTType *parent = UTTypeLookup(parts[2], YES);
		if (parent)
			return UTDynamicType(parts[0], parts[1], parent);
	}
	// A dynamic identifier this implementation did not generate: keep its identity, without tags or conformances.
	return [[[UTType alloc] _initWithIdentifier: identifier kind: UTTypeKindDynamic parents: nil tags: nil description: nil] autorelease];
}

static UTType *UTTypeLookup(NSString *identifier, BOOL allowUndeclared)
{
	UTEnsureRegistry();
	if (!UTIdentifierIsValid(identifier))
		return nil;
	NSString *key = [identifier lowercaseString];
	UTType *type = [UTRegistry objectForKey: key];
	if (type)
		return type;
	if ([key hasPrefix: UTDynamicPrefix] && [key length] > [UTDynamicPrefix length])
		return UTDynamicTypeFromIdentifier(identifier);
	if (!allowUndeclared)
		return nil;
	return [[[UTType alloc] _initWithIdentifier: identifier kind: UTTypeKindUndeclared parents: nil tags: nil description: nil] autorelease];
}

static UTType *UTRuntimeType(NSString *identifier, UTType *parent)
{
	UTEnsureRegistry();
	if (!UTIdentifierIsValid(identifier))
		return nil;
	UTType *type = UTTypeLookup(identifier, NO);
	if (type)
		return type;
	// Without an Info.plist declaration the type is undeclared, but it keeps the conformance the caller asked for.
	NSArray *parents = [parent isKindOfClass: [UTType class]] ? @[parent] : nil;
	return [[[UTType alloc] _initWithIdentifier: identifier kind: UTTypeKindUndeclared parents: parents tags: nil description: nil] autorelease];
}

- (instancetype)_initWithIdentifier:(NSString *)identifier kind:(UTTypeKind)kind parents:(NSArray *)parents
                               tags:(NSDictionary *)tags description:(NSString *)description
{
	self = [super init];
	if (self) {
		_identifier = [identifier copy];
		_key = [[identifier lowercaseString] copy];
		_kind = kind;
		_parents = [parents copy] ?: [@[] retain];
		_tags = [tags copy] ?: [@{} retain];
		_description = [description copy];
	}
	return self;
}

- (void)_setParents:(NSArray *)parents
{
	NSArray *old = _parents;
	_parents = [parents copy];
	[old release];
}

- (void)dealloc
{
	[_identifier release];
	[_key release];
	[_parents release];
	[_tags release];
	[_description release];
	[super dealloc];
}

+ (UTType *)typeWithIdentifier:(NSString *)identifier
{
	return UTTypeLookup(identifier, NO);
}

+ (NSArray<UTType *> *)typesWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(UTType *)supertype
{
	UTEnsureRegistry();
	if (![tag isKindOfClass: [NSString class]] || [tag length] == 0 || ![tagClass isKindOfClass: [NSString class]])
		return @[];
	if (supertype && ![supertype isKindOfClass: [UTType class]])
		return @[];
	NSDictionary *index = [UTTagIndex objectForKey: tagClass];
	if (!index)
		return @[];

	NSMutableArray *result = [NSMutableArray array];
	for (UTType *type in [index objectForKey: [tag lowercaseString]]) {
		if (!supertype || [type conformsToType: supertype])
			[result addObject: type];
	}
	if ([result count] == 0 && [tag rangeOfString: UTDynamicSeparator].location == NSNotFound)
		[result addObject: UTDynamicType(tagClass, tag, supertype ?: UTTypeData)];
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

+ (UTType *)exportedTypeWithIdentifier:(NSString *)identifier
{
	UTEnsureRegistry();
	return UTRuntimeType(identifier, UTTypeData);
}

+ (UTType *)exportedTypeWithIdentifier:(NSString *)identifier conformingToType:(UTType *)parentType
{
	return UTRuntimeType(identifier, parentType);
}

+ (UTType *)importedTypeWithIdentifier:(NSString *)identifier
{
	UTEnsureRegistry();
	return UTRuntimeType(identifier, UTTypeData);
}

+ (UTType *)importedTypeWithIdentifier:(NSString *)identifier conformingToType:(UTType *)parentType
{
	return UTRuntimeType(identifier, parentType);
}

- (NSString *)identifier
{
	return _identifier;
}

- (NSDictionary<NSString *, NSArray<NSString *> *> *)tags
{
	return _tags;
}

- (NSString *)preferredFilenameExtension
{
	NSArray *extensions = [_tags objectForKey: UTTagClassFilenameExtension];
	return [extensions count] ? [extensions objectAtIndex: 0] : nil;
}

- (NSString *)preferredMIMEType
{
	NSArray *mimeTypes = [_tags objectForKey: UTTagClassMIMEType];
	return [mimeTypes count] ? [mimeTypes objectAtIndex: 0] : nil;
}

- (NSString *)localizedDescription
{
	if (_kind == UTTypeKindDynamic)
		return nil;
	if (_description)
		return _description;
	for (UTType *parent in _parents) {
		NSString *description = [parent localizedDescription];
		if (description)
			return description;
	}
	return nil;
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
	return [_key hasPrefix: @"public."];
}

- (BOOL)conformsToType:(UTType *)type
{
	if (![type isKindOfClass: [UTType class]])
		return NO;
	return UTTypeConformsToKey(self, type->_key, 0);
}

- (BOOL)isSubtypeOfType:(UTType *)type
{
	return [type isKindOfClass: [UTType class]] && ![_key isEqualToString: type->_key] && [self conformsToType: type];
}

- (BOOL)isSupertypeOfType:(UTType *)type
{
	return [type isKindOfClass: [UTType class]] && [type isSubtypeOfType: self];
}

- (NSSet<UTType *> *)supertypes
{
	NSMutableSet *supertypes = [NSMutableSet set];
	UTTypeCollectSupertypes(self, supertypes, 0);
	return [[supertypes copy] autorelease];
}

- (BOOL)isEqual:(id)object
{
	if (object == self)
		return YES;
	return [object isKindOfClass: [UTType class]] && [_key isEqualToString: ((UTType *) object)->_key];
}

- (NSUInteger)hash
{
	return [_key hash];
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
	UTType *type = [UTTypeLookup(identifier, YES) retain];
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
