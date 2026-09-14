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

#ifndef _UNIFORMTYPEIDENTIFIERS_UTTYPE_H_
#define _UNIFORMTYPEIDENTIFIERS_UTTYPE_H_

#include <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString *const UTTagClassFilenameExtension;
extern NSString *const UTTagClassMIMEType;

@interface UTType : NSObject <NSCopying, NSSecureCoding>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

// Returns nil when the identifier is neither a declared type nor in the dynamic ("dyn.") namespace.
+ (nullable UTType *)typeWithIdentifier:(NSString *)identifier;
+ (nullable UTType *)typeWithFilenameExtension:(NSString *)filenameExtension;
+ (nullable UTType *)typeWithFilenameExtension:(NSString *)filenameExtension conformingToType:(UTType *)supertype;
+ (nullable UTType *)typeWithMIMEType:(NSString *)mimeType;
+ (nullable UTType *)typeWithMIMEType:(NSString *)mimeType conformingToType:(UTType *)supertype;

@property (readonly, copy) NSString *identifier;
@property (readonly, nullable, copy) NSString *preferredFilenameExtension;
@property (readonly, nullable, copy) NSString *preferredMIMEType;
@property (readonly, nullable, copy) NSString *localizedDescription;
@property (readonly, nullable, copy) NSNumber *version;
@property (readonly, nullable, copy) NSURL *referenceURL;
@property (readonly, getter=isDynamic) BOOL dynamic;
@property (readonly, getter=isDeclared) BOOL declared;
@property (readonly, getter=isPublicType) BOOL publicType;

// Tag specification
+ (nullable UTType *)typeWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(nullable UTType *)supertype;
+ (NSArray<UTType *> *)typesWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(nullable UTType *)supertype;
@property (readonly) NSDictionary<NSString *, NSArray<NSString *> *> *tags;

// Conformance
- (BOOL)conformsToType:(UTType *)type;
- (BOOL)isSupertypeOfType:(UTType *)type;
- (BOOL)isSubtypeOfType:(UTType *)type;
@property (readonly, copy) NSSet<UTType *> *supertypes;

// Runtime declarations
+ (UTType *)exportedTypeWithIdentifier:(NSString *)identifier;
+ (UTType *)exportedTypeWithIdentifier:(NSString *)identifier conformingToType:(UTType *)parentType;
+ (UTType *)importedTypeWithIdentifier:(NSString *)identifier;
+ (UTType *)importedTypeWithIdentifier:(NSString *)identifier conformingToType:(UTType *)parentType;

@end

NS_ASSUME_NONNULL_END

#endif
