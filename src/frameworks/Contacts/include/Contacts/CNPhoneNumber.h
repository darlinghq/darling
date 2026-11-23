/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#ifndef CONTACTS_CNPHONENUMBER_H
#define CONTACTS_CNPHONENUMBER_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// An immutable phone number.
API_AVAILABLE(macos(10.11))
@interface CNPhoneNumber : NSObject <NSCopying, NSSecureCoding>

// Both give nil for a nil string.
+ (nullable instancetype)phoneNumberWithStringValue:(NSString *)stringValue;
- (nullable instancetype)initWithStringValue:(NSString *)string;

- (null_unspecified instancetype)init API_DEPRECATED("Use initWithStringValue:", macos(10.11, 10.13));
+ (null_unspecified instancetype)new API_DEPRECATED("Use phoneNumberWithStringValue:", macos(10.11, 10.13));

@property (readonly, copy, NS_NONATOMIC_IOSONLY) NSString *stringValue;

@end

NS_ASSUME_NONNULL_END

#endif
