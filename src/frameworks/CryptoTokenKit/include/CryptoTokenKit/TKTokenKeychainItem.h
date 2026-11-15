/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#import <Foundation/Foundation.h>
#import <CryptoTokenKit/TKToken.h>

NS_ASSUME_NONNULL_BEGIN

// An item (certificate or key) a token exposes to the keychain.
API_AVAILABLE(macos(10.10))
@interface TKTokenKeychainItem : NSObject

- (instancetype)initWithObjectID:(TKTokenObjectID)objectID;

@property (nonatomic, readonly) TKTokenObjectID objectID;
@property (nonatomic, copy, nullable) NSString *label;
// Per operation, the constraint that must be satisfied before the operation is allowed.
@property (nonatomic, copy, nullable) NSDictionary<NSNumber *, TKTokenOperationConstraint> *constraints;

@end

NS_ASSUME_NONNULL_END
