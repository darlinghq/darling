/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <SecurityFoundation/SFEncryptionOperation.h>
#import <SecurityFoundation/_SFAESKeySpecifier.h>
#import <SecurityFoundation/_SFAuthenticatedCiphertext.h>
#import <SecurityFoundation/_SFAESKey.h>

#define SFAuthenticatedEncryptionOperation _SFAuthenticatedEncryptionOperation

// i'm 90% sure this is AES-GCM

@interface SFAuthenticatedEncryptionOperation : NSObject <SFEncryptionOperation>

- (instancetype)initWithKeySpecifier:(SFAESKeySpecifier*)keySpecifier;

- (SFAuthenticatedCiphertext*)encrypt:(NSData*)data withKey:(SFAESKey*)key error:(NSError**)error;
- (SFAuthenticatedCiphertext*)encrypt:(NSData*)data withKey:(SFAESKey*)key additionalAuthenticatedData:(NSData*)aad error:(NSError**)error;

// common sense tells me that the input parameter should be `SFAuthenticatedCiphertext*`,
// but `SecDbKeychainItemV7.m` in Security says otherwise
- (NSData*)decrypt:(NSData*)data withKey:(SFAESKey*)key error:(NSError**)error;

@end
