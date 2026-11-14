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
#import <CloudKit/CKDefines.h>
#import <CloudKit/CKOperation.h>
#import <CloudKit/CKUserIdentity.h>

NS_ASSUME_NONNULL_BEGIN

// Finds the people in the user's contacts who use the app.
@interface CKDiscoverAllUserIdentitiesOperation : CKOperation {
@private
    void (^_userIdentityDiscoveredBlock)(CKUserIdentity *identity);
    void (^_discoverAllUserIdentitiesCompletionBlock)(NSError * _Nullable operationError);
}

@property (nonatomic, copy, nullable) void (^userIdentityDiscoveredBlock)(CKUserIdentity *identity);
@property (nonatomic, copy, nullable) void (^discoverAllUserIdentitiesCompletionBlock)(NSError * _Nullable operationError);
@end

NS_ASSUME_NONNULL_END
