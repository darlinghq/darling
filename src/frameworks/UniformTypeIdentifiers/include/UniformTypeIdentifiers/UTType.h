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

#include <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface UTType : NSObject <NSCopying, NSSecureCoding>

+ (nullable instancetype)typeWithIdentifier:(NSString *)identifier;
+ (nullable instancetype)typeWithFilenameExtension:(NSString *)filenameExtension;
+ (nullable instancetype)typeWithFilenameExtension:(NSString *)filenameExtension conformingToType:(UTType *)supertype;
+ (nullable instancetype)typeWithMIMEType:(NSString *)mimeType;
+ (nullable instancetype)typeWithMIMEType:(NSString *)mimeType conformingToType:(UTType *)supertype;
+ (NSArray<UTType *> *)typesWithTag:(NSString *)tag tagClass:(NSString *)tagClass conformingToType:(nullable UTType *)supertype;

@property (nonatomic, readonly, copy) NSString *identifier;
@property (nonatomic, readonly, nullable) NSString *preferredFilenameExtension;
@property (nonatomic, readonly, nullable) NSString *preferredMIMEType;
@property (nonatomic, readonly) NSDictionary<NSString *, NSArray<NSString *> *> *tags;
@property (nonatomic, readonly, copy) NSString *localizedDescription;
@property (nonatomic, readonly, getter=isDeclared) BOOL declared;
@property (nonatomic, readonly, getter=isDynamic) BOOL dynamic;
@property (nonatomic, readonly, getter=isPublicType) BOOL publicType;
@property (nonatomic, readonly) NSSet<UTType *> *supertypes;

- (BOOL)conformsToType:(UTType *)type;
- (BOOL)isSupertypeOfType:(UTType *)type;
- (BOOL)isSubtypeOfType:(UTType *)type;

@end

NS_ASSUME_NONNULL_END
