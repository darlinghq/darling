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

// The symmetric key and AES-GCM classes of SecurityFoundation share these initializers; they are
// internal to the framework.
#import <SecurityFoundation/_SFAESKey.h>
#import <SecurityFoundation/_SFAuthenticatedCiphertext.h>

@interface _SFKey ()
- (instancetype)initWithKeyData:(NSData *)data;
@end

@interface _SFAESKeySpecifier ()
@property (readonly, nonatomic) size_t keyLength;
@end

@interface _SFAuthenticatedCiphertext () <NSSecureCoding>
- (instancetype)initWithCiphertext:(NSData *)ciphertext authenticationCode:(NSData *)authenticationCode initializationVector:(NSData *)initializationVector;
@property (readonly, nonatomic) NSData *ciphertext;
@property (readonly, nonatomic) NSData *authenticationCode;
@property (readonly, nonatomic) NSData *initializationVector;
@end
