/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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

#ifndef _SF_SFKEYCHAIN_H_
#define _SF_SFKEYCHAIN_H_

// not sure if this is the header that imports these
// it could also be `SFCredential_Private.h` or `SFCredentialStore_Private.h`
#import <SecurityFoundation/_SFPasswordCredential.h>
#import <SecurityFoundation/_SFAccessPolicy.h>

extern NSString* const SFKeychainErrorDomain;

// not sure how these codes are supposed to be set up
typedef NS_ENUM(NSInteger, SFKeychainError) {
	// doesn't start at `-1` because that's usually for "unknown" errors
	// these are guesses by the way
	SFKeychainErrorInvalidParameter = -2,
	SFKeychainErrorSaveFailed = -3,
	SFKeychainErrorMissingAccessGroup = -4,
	SFKeychainErrorInvalidPersistentIdentifier = -5,
	SFKeychainErrorSecureDecodeFailed = -6,
};

#endif // _SF_SFKEYCHAIN_H_
