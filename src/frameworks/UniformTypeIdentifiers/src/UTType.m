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

#import <UniformTypeIdentifiers/UTType.h>
#import <CoreServices/CoreServices.h>

// Types are resolved through the CoreServices UTI database. A tag with no
// declared type resolves to a dynamic type whose identifier is derived from
// the tag, as the real framework does.

static NSString *const UTTagClassFilenameExtension = @"public.filename-extension";
static NSString *const UTTagClassMIMEType = @"public.mime-type";
static NSString *const UTDynamicPrefix = @"dyn.";

@implementation UTType {
    NSString *_identifier;
}

- (instancetype)initWithIdentifier:(NSString *)identifier
{
    if ((self = [super init])) {
        _identifier = [identifier copy];
    }
    return self;
}

- (void)dealloc
{
    [_identifier release];
    [super dealloc];
}

+ (instancetype)typeWithIdentifier:(NSString *)identifier
{
    if (identifier == nil || [identifier length] == 0) {
        return nil;
    }
    return [[[self alloc] initWithIdentifier:identifier] autorelease];
}

+ (instancetype)_typeWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(UTType *)supertype
{
    if (tag == nil || [tag length] == 0) {
        return nil;
    }

    CFStringRef found = UTTypeCreatePreferredIdentifierForTag((CFStringRef)tagClass, (CFStringRef)tag, supertype ? (CFStringRef)supertype->_identifier : NULL);
    if (found != NULL) {
        UTType *type = [self typeWithIdentifier:(NSString *)found];
        CFRelease(found);
        return type;
    }

    if (supertype != nil) {
        return nil;
    }
    return [self typeWithIdentifier:[UTDynamicPrefix stringByAppendingString:[NSString stringWithFormat:@"%@.%@", tagClass, tag]]];
}

+ (instancetype)typeWithFilenameExtension:(NSString *)filenameExtension
{
    return [self _typeWithTag:filenameExtension tagClass:UTTagClassFilenameExtension conformingToType:nil];
}

+ (instancetype)typeWithFilenameExtension:(NSString *)filenameExtension conformingToType:(UTType *)supertype
{
    return [self _typeWithTag:filenameExtension tagClass:UTTagClassFilenameExtension conformingToType:supertype];
}

+ (instancetype)typeWithMIMEType:(NSString *)mimeType
{
    return [self _typeWithTag:mimeType tagClass:UTTagClassMIMEType conformingToType:nil];
}

+ (instancetype)typeWithMIMEType:(NSString *)mimeType conformingToType:(UTType *)supertype
{
    return [self _typeWithTag:mimeType tagClass:UTTagClassMIMEType conformingToType:supertype];
}

+ (NSArray<UTType *> *)typesWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(UTType *)supertype
{
    NSMutableArray *types = [NSMutableArray array];
    CFArrayRef identifiers = UTTypeCreateAllIdentifiersForTag((CFStringRef)tagClass, (CFStringRef)tag, supertype ? (CFStringRef)supertype->_identifier : NULL);

    if (identifiers != NULL) {
        for (NSString *identifier in (NSArray *)identifiers) {
            [types addObject:[self typeWithIdentifier:identifier]];
        }
        CFRelease(identifiers);
    }
    return types;
}

- (NSString *)identifier
{
    return _identifier;
}

- (NSString *)_preferredTagOfClass:(NSString *)tagClass
{
    CFStringRef tag = UTTypeCopyPreferredTagWithClass((CFStringRef)_identifier, (CFStringRef)tagClass);
    return tag ? [(NSString *)tag autorelease] : nil;
}

- (NSString *)preferredFilenameExtension
{
    return [self _preferredTagOfClass:UTTagClassFilenameExtension];
}

- (NSString *)preferredMIMEType
{
    return [self _preferredTagOfClass:UTTagClassMIMEType];
}

- (NSDictionary<NSString *, NSArray<NSString *> *> *)tags
{
    NSMutableDictionary *tags = [NSMutableDictionary dictionary];
    NSString *extension = [self preferredFilenameExtension];
    NSString *mimeType = [self preferredMIMEType];

    if (extension) {
        tags[UTTagClassFilenameExtension] = @[extension];
    }
    if (mimeType) {
        tags[UTTagClassMIMEType] = @[mimeType];
    }
    return tags;
}

- (NSString *)localizedDescription
{
    CFStringRef description = UTTypeCopyDescription((CFStringRef)_identifier);
    return description ? [(NSString *)description autorelease] : nil;
}

- (BOOL)isDynamic
{
    return [_identifier hasPrefix:UTDynamicPrefix];
}

- (BOOL)isDeclared
{
    return ![self isDynamic];
}

- (BOOL)isPublicType
{
    return [_identifier hasPrefix:@"public."];
}

- (BOOL)conformsToType:(UTType *)type
{
    return type != nil && UTTypeConformsTo((CFStringRef)_identifier, (CFStringRef)type->_identifier);
}

- (BOOL)isSubtypeOfType:(UTType *)type
{
    return ![self isEqual:type] && [self conformsToType:type];
}

- (BOOL)isSupertypeOfType:(UTType *)type
{
    return [type isSubtypeOfType:self];
}

// Only the types this type directly names as ancestors are known from the database.
- (NSSet<UTType *> *)supertypes
{
    return [NSSet set];
}

- (BOOL)isEqual:(id)other
{
    return [other isKindOfClass:[UTType class]] && [_identifier isEqual:((UTType *)other)->_identifier];
}

- (NSUInteger)hash
{
    return [_identifier hash];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<UTType 0x%lx> %@", (unsigned long)self, _identifier];
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
    [coder encodeObject:_identifier forKey:@"identifier"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    return [self initWithIdentifier:[coder decodeObjectOfClass:[NSString class] forKey:@"identifier"]];
}

@end
