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

NS_ASSUME_NONNULL_BEGIN

@class TKToken, TKTokenDriver, TKTokenSession, TKTokenKeychainContents;
@protocol TKTokenDelegate;

// Identifier of an object (certificate, key) on a token, and a condition attached to an operation on it.
typedef id TKTokenObjectID API_AVAILABLE(macos(10.10));
typedef id<NSObject, NSSecureCoding> TKTokenOperationConstraint API_AVAILABLE(macos(10.10));

typedef NS_ENUM(NSInteger, TKTokenOperation) {
    TKTokenOperationNone = 0,
    TKTokenOperationReadData = 1,
    TKTokenOperationSignData = 2,
    TKTokenOperationDecryptData = 3,
    TKTokenOperationPerformKeyExchange = 4,
} API_AVAILABLE(macos(10.10));

// A cryptographic token (a smart card, for example) made available to the system by a token driver.
API_AVAILABLE(macos(10.10))
@interface TKToken : NSObject

@property (nonatomic, readonly) TKTokenDriver *tokenDriver;
@property (nonatomic, weak, nullable) id<TKTokenDelegate> delegate;
@property (nonatomic, readonly) NSString *tokenID;
@property (nonatomic, nullable, strong) TKTokenKeychainContents *keychainContents;

- (instancetype)initWithTokenDriver:(TKTokenDriver *)tokenDriver instanceID:(NSString *)instanceID;

@end

API_AVAILABLE(macos(10.10))
@protocol TKTokenDelegate <NSObject>

// Creates the session in which a client uses the token.
- (nullable TKTokenSession *)token:(TKToken *)token createSessionWithError:(NSError **)error;

@optional
- (void)token:(TKToken *)token terminateSession:(TKTokenSession *)session;

@end

NS_ASSUME_NONNULL_END
